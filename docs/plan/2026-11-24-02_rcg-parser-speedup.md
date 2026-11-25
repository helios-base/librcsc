# RCG パーサの高速化 計画

| 項目 | 内容 |
|---|---|
| 作成日 | 2026-11-24 |
| 最終更新日 | 2026-11-25 |
| 対象 | `develop` (`212f5e7`)。同梱の simdjson 3.8.0 |
| ブランチ | `feature/rcg-parser-speedup`。`develop` の `212f5e7` から、ユーザが GitHub で作った |
| 状態 | 完了。PR #123 (`Speed up the rcg parser and update the bundled simdjson to 5.1.1`) で develop に取り込む (Squash Merge)。詳しくは「進み具合」を参照 |
| 関連 | `docs/plan/2026-11-21_boost-cmake-maintenance.md` (simdjson の更新と `parser_json` の扱いをこの計画に回した)。参照実装: rcssmonitor の PR #65 (`docs/plan/2026-11-11_3d-view.md` の Phase 2)。利用側: soccerwindow2 (`src/model/main_data.cpp`) |

## 進み具合

| 段階 | 内容 | 状態 | ユーザの確認 | CI | コミット | プッシュ |
|---|---|---|---|---|---|---|
| 1 | 計測と回帰テストの土台 (§4.1、S11) | 完了 | 済み | 通過 | `b4d1b5c` | 済み |
| 2 | simdjson 5.1.1 への更新、`parser_json` の移動 (§4.2、D2、D3、D6、S8、S9、S14) | 完了 | 済み | 通過 | `e778865` | 済み |
| 3 | メモリからの解析、ファイルの一括読み込み (§4.3、D7、S2、S3、S12、S15、S16、S17) | 完了 | 済み | 通過 | `20f8b9d` | 済み |
| — | GCC 15 の警告 (`-Wdangling-reference`、段階 3 のテスト) の修正、`docs/todo.md` への記録 (§4.3.2、§8) | 完了 | 済み | 通過 | `a0adabe` | 済み |
| 4 | 行の並列解析 (§4.4、D5、S1、S10、S13、S18、S19) | 完了 | 済み | 通過 | `dd51505` | 済み |
| 5 | libdeflate による展開 (§4.5、D4、D8、S20) | 完了 | 済み | 通過 | `dfcbed2` | 済み |
| 6 | 読み込みの堅牢化 (§4.6、D9、S5、S21、S22) | 完了 | 済み | 通過 | `9e29dc9` | 済み |
| 7 | 仕上げ: 最終の計測、全体の比較、soccerwindow2 での確認、文書 (§4.7) | 完了 | 済み | 通過 | `3e87052` | 済み |
| — | develop への取り込み (PR #123、Squash Merge) | 完了 | 済み | 通過 | — | — |

## 1. 目的と方針

- rcg の読み込みを速くする。対象は librcsc のツール (`rcg2csv` など) と、librcsc を使う soccerwindow2。
- rcssmonitor の PR #65 の方式に倣う。rcssmonitor の RCG パーサはもともと librcsc のものを移植したもので、構成が同じため、素直に持ち込める。
  1. ファイル全体をメモリに読む (gzip は一度に展開する)。
  2. メモリの上で行に分けて解析する (`std::getline` を使わない)。
  3. JSON は 1 レコード 1 行として、バッファの上でそのまま解析する。
  4. 行を複数のスレッドで並列に解析し、ハンドラへは呼び出したスレッドで元の順に渡す。
- 同梱の simdjson を 5.1.1 に更新し、GCC 15 の警告 9 件を解消する。
- 古い JSON パーサ (`parser_json.*`) と、それだけが使う nlohmann/json を `obsolete/rcg/` に移す。
- 今の API は残す。利用側 (soccerwindow2 など) は直さずにビルドでき、今の呼び出し方でも解析は速くなるようにする。
- 移植のついでに、rcssmonitor で直した読み込みの不具合 (rcssmonitor の block B) も直す。

## 2. 現状 (`212f5e7`、2026-11-24 に調べた)

### 2.1 librcsc の RCG パーサ

- 形式の判定: `Parser::create( std::istream & )` が先頭の 4 バイトを読む。`[` なら JSON、`ULG` と文字なら版、それ以外は v1 (`rcsc/rcg/parser.cpp`)。
- 入口は `parse( std::istream &, Handler & )` と `parse( const std::string & filepath, Handler & )`。基底の `parse(filepath)` は `std::ifstream` で開くため、gzip を読めない。
- テキスト (v4〜v6、`parser_v4.cpp`): `std::getline` で 1 行ずつ読み、`sscanf`・`strtol`・`strtof` で解析する。空行や解釈できない行があると、そこで読み込みをやめて false を返す。
- JSON (`parser_simdjson.cpp`、simdjson の On-Demand):
  - `parse(istream)` は、ストリームを `std::string` に写し、さらに `padded_string` に写してから、ファイル全体を 1 つの文書として解析する。
  - `parse(filepath)` は `padded_string::load` で読むため、gzip を読めない。
  - 知らないキーがあると、読み込み全体が失敗する。
- ハンドラ (`handler.h`): `handleServerVersion()` は仮想だが、`handleTimestamp()` は仮想でない。どのパーサも、この 2 つを呼んでいない。
- ハンドラの戻り値 (段階 1 で確かめた):
  - テキスト: `handleShow()` などが false を返しても、最後まで読む (`parse()` は true)。`handleLogVersion()` が false なら、そこで読み込みをやめる (`rcgverconv` は、同じ版への変換をこれで断っている)。
  - JSON: `handleShow()` などが false を返すと、そこで読み込みをやめる (`parse()` は false)。`handleLogVersion()` の戻り値は無視している。
- `ParserV4` などは `parse( std::istream &, ... )` だけを上書きしているため、基底の `parse( const std::string & filepath, ... )` が隠れ、派生クラスの型からは呼べない (段階 1 で確かめた。`Parser &` を通せば呼べる)。
- 実際のログで `Parser` を動かすテストはない (rcg のテストは `run_test_rcg_types` だけ)。

### 2.2 同梱の simdjson

- 3.8.0 (`rcsc/rcg/simdjson/`)。GCC 15 のエラーを直すため、`base_formatter` の 3 行に本家と同じ修正を当ててある (`897bc90`、#119)。
- GCC 15 (CI の Ubuntu 26.04) で、`simdjson.cpp` に `-Wtemplate-id-cdtor` の警告が 9 件出る。
- ヘッダはインストールしないが、simdjson のシンボルは `librcsc.so` から公開されている。
- On-Demand の解析部 (stage 2) は、コンパイル時に選ばれる実装で動く。librcsc は CPU を指定せずにコンパイルするため、SIMD を使わない `fallback` になっている (手元の CMake と Autotools のビルドで、`simdjson::fallback::ondemand` のシンボルを確かめた)。構造の索引付け (stage 1) は、実行時に CPU を見て実装を選ぶため、SIMD を使う。
- 本家の最新は 5.3.0 (2026-11-22)。rcssmonitor は 5.1.1 を同梱している。

### 2.3 古い JSON パーサ

- `rcsc/rcg/parser_json.{h,cpp}` は nlohmann/json を使うパーサで、simdjson のパーサ (#66、`05b8e43`) より前のもの。
- どちらのビルドにも入ったことがなく、インストールもされない。`parser.cpp` に、コメントアウトされた参照があるだけ。
- 同梱の `rcsc/rcg/nlohmann/json.hpp` を使うのは `parser_json.cpp` だけ。

### 2.4 利用側

- ツール (`src/`): `rcg2csv`、`rcg2txt`、`rcgrenameteam`、`rcgreverse`、`rcgvalidator`、`rcgverconv` は、`rcsc::gzifstream` で開き、`Parser::create( fin )` と `parse( fin, handler )` を呼ぶ。
- `rcgresultprinter`: `parse(filepath)` が gzip を読めないため、`.gz` を一時ファイル (`temp_directory_path()/"temp.rcg"`) に展開してから読む。名前が決まっているため、2 つ同時に動かすとぶつかる。
- soccerwindow2 (`src/model/main_data.cpp`):
  - ファイル: `gzifstream`、`Parser::create( fin )`、`parse( fin, ... )`。
  - ライブ受信: 1 パケットごとに `ParserSimdJSON::parseData()`、`ParserV4::parseLine( 0, ... )`。
  - これらの API は残す。

### 2.5 rcssmonitor の高速化 (PR #65、参照実装)

- 測ったログ: JSON (rcssserver 18.1.3、展開後 53.6 MB、6,209 フレーム)、テキスト ULG5 (28.7 MB、8,406 フレーム)。
- 手順ごとの時間 (`bench_load`、5 回の最良、JSON / テキスト):

| 手順 | 変えたこと | JSON | テキスト |
|---|---|---|---|
| A1 | ベンチマークを追加 | 235 ms | 130 ms |
| A2 | simdjson 5.1.1 | 222 ms | 130 ms |
| A3 | `gzread` で一度に読み、メモリから解析 | 179 ms | 121 ms |
| A4 | JSON を 1 レコード 1 行として、その場で解析 | 151 ms | — |
| A5 | テキストをメモリから 1 行ずつ解析 | — | 122 ms |
| A6 | 行の並列解析 (最大 8 スレッド、1 スレッドあたり 1,024 行以上) | 84 ms | 60 ms |
| — | libdeflate で展開 | 53〜74 ms | 33〜42 ms |

- 残る時間の多くは展開で、並列にできない。展開だけなら zlib 69 / 48 ms、libdeflate 34 / 22 ms。
- simdjson 5.1.1 の `simdjson.h` は 10 MB (3.8.0 は 3.8 MB)。`simdjson.cpp` のコンパイルは 3.5 秒から 9.6 秒になった。GCC 15 の警告 9 件は消えた。
- 3.8.0 から 5.1.1 で直す必要があった所は、`get_string( std::string & )` の結果を使っていない 1 か所だけだった。

### 2.6 見つかっている不具合

B1〜B11 は、rcssmonitor では直してある。B12〜B14 は librcsc のツールの不具合。B13・B14 は段階 3 で、B1〜B12 は段階 6 で直した (§4.6.2)。

| # | 場所 | 内容 |
|---|---|---|
| B1 | `parser_v4.cpp:121-123` | 記録の名前を `%s` で `char name[32]` に読むため、長い名前でスタックをはみ出す |
| B2 | `parser_v4.cpp:499`、`:512` | `unum == 0` を受け付け、`player_[-1]` に書く |
| B3 | `parser_v4.cpp:390` | `strtol` の失敗を `LONG_MIN`・`LONG_MAX` で調べていて、失敗を検出できない |
| B4 | `parser_v4.cpp:79` | ヘッダの版を `std::stoi` で読むため、壊れたヘッダで例外が出る |
| B5 | `parser_v4.cpp` の `parseLine()` | 各記録の解析の結果を無視している。空行や知らない記録で、読み込み全体をやめる |
| B6 | `parser_v4.cpp:696-697` | チームのグラフィックの文字列を 15 文字までしか読めない |
| B7 | `parser_v4.cpp` の `parseTeam()` | チーム名 `null` を空にしていない (show の中では空にしている) |
| B8 | `parser_simdjson.cpp:169-183` | `version`・`timestamp` を読み捨てる。そのため `rcgverconv` などで書き直した JSON の rcg は `"version":"unknown"` になり、タイムスタンプが消える |
| B9 | `parser_simdjson.cpp:478` | `vq` を `'h'`・`'l'` ではなく真偽値 (1・0) で `char view_quality_` に入れている |
| B10 | `parser_simdjson.cpp` の `parseShow()` | プレイヤーを出てきた順に並べている (side と unum の位置に置いていない)。省かれうるフィールドがないと失敗する。失敗しても途中までのデータで `handleShow()` を呼ぶ |
| B11 | `parser_simdjson.cpp:151-157` | 知らないキーがあると、読み込み全体が失敗する |
| B12 | `src/rcgversion.cpp` の `get_version()` | v6 を判別できず、知らない版を `-1` として `json` と表示する (ULG6 のログが `version=json` になる)。段階 1 で見つけた |
| B13 | `src/rcg2csv.cpp`・`src/rcgvalidator.cpp` の `handleLogVersion()` | 版が 4 より小さいと false を返すため、JSON (-1) も断っている (`rcgvalidator` は `Unsupported RCG version -1` と出す)。JSON のパーサがこの戻り値を無視していたため、読めていただけだった。段階 3 で見つけ、直した |
| B14 | `src/resultprinter.cpp` (`rcgresultprinter`) | `strptime()` の前に `tm` を初期化していないため、`tm_isdst` が不定になり、出す日時が 1 時間ずれることがある。段階 3 で見つけ、直した |

### 2.7 計測に使うログ

作業ツリーの直下にある、Git で追跡していないファイルを使う (リポジトリには入れない)。JSON と ULG5 は、rcssmonitor で測ったものと同じファイル (MD5 で確かめた)。

| ログ | 形式 | 展開後の大きさ | show の数 | ほかの記録の数 |
|---|---|---|---|---|
| `1-json.rcg.gz` | JSON、rcssserver 18.1.3 | 53,644,753 バイト | 6,209 | 319 |
| `202207160432-HELIOS2022_1-vs-CYRUS_0.rcg.gz` | ULG5 | 28,729,173 バイト | 8,406 | 600 |
| `202607050404-HELIOS2026_0-vs-YuShan2026_1.rcg.gz` | ULG6 | 22,305,464 バイト | 6,469 | 489 |

- 手元の環境: Ubuntu 24.04、GCC 13.3、32 コア

## 3. 決定事項

### 3.1 ユーザの決定

| # | 決定 | 決めた日 | いつ |
|---|---|---|---|
| D1 | 実装は rcssmonitor の方式 (PR #65) に倣う | 2026-11-24 | 計画の作成時 (依頼) |
| D2 | 同梱の simdjson を 5.1.1 にする。rcssmonitor と同じ版にする (最新は 5.3.0) | 2026-11-24 | 計画の作成時 |
| D3 | `parser_json.{h,cpp}` と `nlohmann/json.hpp` を `obsolete/rcg/` に移す。ビルドには入れない。(2026-11-24、段階 2 の初めに、`nlohmann/json.hpp` は移さずに削除することにした。D6) | 2026-11-24 | 計画の作成時 |
| D4 | gzip の展開に libdeflate を使う。任意の依存とし、見つかれば使い、なければ zlib で展開する | 2026-11-24 | 計画の作成時 |
| D5 | 並列に解析するスレッドの数の上限を、API で設定できるようにする (例: `Parser::setMaxThreads( n )`。0 は既定、1 は並列にしない) | 2026-11-24 | 計画の作成時 |
| D6 | `nlohmann/json.hpp` は `obsolete/` にも移さず、削除する (D3 を変えた)。`parser_json.{h,cpp}` は D3 のとおり `obsolete/rcg/` に移す | 2026-11-24 | 段階 2 の初め |
| D7 | ファイルを (gzip なら展開して) メモリに読む関数 (S3) は `rcsc/gz/` に置く (§3.3) | 2026-11-24 | 段階 3 の初め |
| D8 | CI では、CMake の 2 つのジョブにだけ `libdeflate-dev` を入れ、Autotools の 2 つのジョブには入れない (rcssmonitor と同じ。§3.3) | 2026-11-25 | 段階 5 の初め |
| D9 | ボールの高さ (`ball.z`、`ball.vz`。rcssmonitor の B-5) は、今回は `BallT` に加えない。サーバの形式が決まってから加える。今は知らないキーとして飛ばす。`docs/todo.md` に記録する (§3.3) | 2026-11-25 | 段階 6 の初め |

### 3.2 設計上の判断 (Claude)

| # | 判断 | 理由 |
|---|---|---|
| S1 | 並列化は rcssmonitor と同じ方式にする。`std::thread` で行を連続したブロックに分け、各スレッドは記録用のハンドラ (`RecordingHandler`) に解析する。すべて終わった後、呼び出したスレッドで、ブロックの順に本当のハンドラへ再生する | 利用側のハンドラはスレッド安全でなくてよく、今のハンドラをそのまま使える。rcssmonitor で順序と結果を確かめてある |
| S2 | 今の API (`create(istream)`、`parse(istream)`、`parse(filepath)`、`parseLine()`、`parseData()`) は残す。`parse(istream)` は内部でメモリに写して、速い経路を通る | soccerwindow2 などを直さずに使え、直さなくても解析は速くなる |
| S3 | ファイルを (gzip なら展開して) メモリに読む関数を公開する。基底の `parse(filepath)` と `ParserSimdJSON::parse(filepath)` をこの関数で読むようにし、gzip を読めるようにする | ツールと利用側が同じ経路を使える。`rcgresultprinter` の一時ファイルをなくせる |
| S4 | SOVERSION は 20 のままにする。`Parser` と `Handler` の仮想関数が増えて ABI は変わるため、librcsc を使うプログラムの再ビルドが要ることをリリースノートに書く | SOVERSION はプロトコルの版に合わせて変えてきた (`8440779`) |
| S5 | 空行は無視する。解釈できない行は飛ばして続け、最後に飛ばした行の番号をまとめて出す。飛ばした行があっても `parse()` は true を返す。(2026-11-25、段階 4 で、飛ばして続けることと報告を先に入れた。S18) | rcssmonitor の B-8 と同じ。サーバが途中で止まったログなども読める |
| S6 | テストは CppUnit で、`rcsc/rcg/` に置く。テストのデータは `rcsc/rcg/test_data/` に置き、配布物 (`EXTRA_DIST`) に入れる | 既存の `run_test_rcg_types` と同じ。`make distcheck` でも動く |
| S7 | ベンチマークは `src/` の開発用のプログラム (インストールしない) として、両方のビルドで作る | `object_table_printer` と同じ扱い |
| S8 | simdjson は本家のタグ v5.1.1 の amalgamated のファイル (`singleheader/`) から取り、rcssmonitor のもの (`ab78451`) と同じことを確かめる | 出所をはっきりさせる |
| S9 | On-Demand の、コンパイル時に選ばれる実装の既定は変えない (CPU を指定するフラグを既定では加えない) | 配布されたバイナリが動く CPU を狭めない。効果は段階 2 で測って記録する |
| S10 | スレッドの数の既定と閾値は rcssmonitor と同じにする (`hardware_concurrency()` と 8 の小さい方。1 スレッドあたり 1,024 行以上) | rcssmonitor で測った値がある |
| S11 | 全体のログでの比較は、2 つのビルドの `rcgverconv` の出力を比べるスクリプト `test/compare_rcg_output.sh` で行う。CTest では動かさない (段階 1 で決めた) | `rcgverconv` は、ハンドラに渡ったものをすべて書き出す。実際のログが要るため、テストには入れられない |
| S12 | JSON のパーサも、`handleLogVersion()` が false なら読み込みをやめる (段階 3。段階 1 で決めた) | テキストのパーサと同じにする。`rcgverconv` は同じ版への変換をこれで断っているが、JSON では今は効いていない |
| S13 | ハンドラが false を返しても、読み込みは止めない (段階 4 から。JSON も)。JSON の動作が変わることをリリースノートに書く (段階 1 で決めた) | 並列の経路では、ハンドラは解析が終わった後に呼ばれるため、戻り値で止められない。テキストは今も止まらない |
| S14 | On-Demand の対象の CPU を選ぶ手段 (CMake・configure のオプションなど) は設けない (段階 2 で測って決めた。§3.3) | ライブラリ全体を haswell 向けにしても、JSON の解析の時間は変わらなかった (§4.2.2) |
| S15 | JSON を 1 行ずつ解析するかの判定では、最初のレコードがその行で閉じていること (`}` か `},` で終わること) も確かめる。rcssmonitor は `{` で始まることだけを見ている (段階 3 で決めた) | 整形された JSON (`{` だけの行が続く) を 1 行ずつ解析すると、すべての行が失敗する。そのようなログは、文書全体として解析する |
| S16 | ファイルを読む関数は `bool rcsc::gzread_file( const std::string & path, std::string * data )` (`rcsc/gz/gzread_file.h`) とする。gzip でないファイルは zlib を通さずにそのまま読む。zlib のないビルドでは、gzip のファイルは読めない (false) (段階 3 で決めた) | 名前は、gzip も普通のファイルも読める zlib の `gzread()` に倣った。gzip の末尾の大きさ (ISIZE) は、確保の見積もりだけに使い、deflate の最大の圧縮率 (約 1032 倍) で抑える (壊れたファイルで大きな値になりうるため) |
| S17 | 段階 3 では、解析できない行や、ハンドラが false を返したときの扱いを変えない (そこで読み込みをやめる)。変えるのは段階 4 (S13) と段階 6 (S5) (段階 3 で決めた) | 段階ごとに、全体の比較で出力が変わらないことを確かめるため |
| S18 | 段階 4 から、パーサが解析できなかった行 (`parseLine()`・`parseRecord()` が false を返すか、例外を出した行) は飛ばして続け、`report_skipped()` で報告する。`parse()` は true を返す (S5 の一部を先に入れる)。空行を無視すること、知らない記録などを失敗として扱うこと (B3、B5) は段階 6 で行う (段階 4 で決めた) | rcssmonitor の `parse_lines()` は、失敗した行を飛ばす作りで、並列のときに「最初の失敗でやめる」を保つには、記録の再生を途中で止める処理が要る。それは段階 6 で消すことになるため、作らない |
| S19 | 1 スレッドのときも、行ごとに記録用のハンドラに解析してから、すぐに本当のハンドラへ再生する。rcssmonitor は 1 スレッドのとき本当のハンドラを直接渡す (段階 4 で決めた) | スレッドの数によらず、ハンドラの扱いを同じにするため。直接渡すと、ハンドラの false を「解析できなかった行」と数え、ハンドラの例外を握りつぶす。並列のときは、戻り値は使われず、例外は `parse()` の外に出る。余計にかかる時間は 1 ms ほど (§4.4.2) |
| S21 | テキストの知らない記録は、JSON の知らないキーと同じく、記録の名前ごとにプロセスで 1 度だけ報告し、解析できなかった行には数えない (`parseLine()` は true)。rcssmonitor は、行ごとにエラーを出し、解析できなかった行として数える (段階 6 で決めた) | 新しいサーバが記録を足したとき、行ごとにエラーが並ばないようにするため。JSON と扱いをそろえる |
| S22 | `rcgreverse`・`rcgrenameteam`・`rcgverconv` は、出力の先頭 (version と timestamp) を `handleLogVersion()` で書かず、最初の記録を受け取るときに書く (`begin()`) (段階 6 で決めた) | JSON のログでは、version と timestamp の記録は `handleLogVersion()` の後に来る。パーサがそれを渡すようになっても (B8)、先頭を `handleLogVersion()` で書くと `"version":"unknown"` のままになる (段階 6 で確かめた) |
| S20 | libdeflate の検出と配布の扱い (段階 5 で決めた): CMake は `find_path`・`find_library` で探す (libdeflate の CMake のパッケージは使わない)。Autotools の `-ldeflate` は `LIBS` に入れず、`LIBDEFLATE_LIBS` として `librcsc.la` と単体テストにだけ渡す。`librcsc.pc`・`librcsc.spec.in`・CMake のパッケージ構成ファイルは変えない | 古いディストリビューション (Ubuntu 22.04 など) には libdeflate の CMake のパッケージがない (rcssmonitor と同じ)。ツールなどに余計な依存を付けないため。librcsc は共有ライブラリだけを作る (`disable-static`) ので、利用側は libdeflate をリンクしなくてよい。`librcsc.spec.in` には、任意の依存 (zlib も) を書いていない |

### 3.3 段階の初めに決めること

- 段階 2: fallback の測定の結果を見て、On-Demand の対象の CPU を選ぶ手段 (CMake・configure のオプションなど) を設けるか。→ 設けない (S14)。
- 段階 3: ファイルを読む関数の名前と置き場所 (案: gzip を扱う `rcsc/gz/` に置くか、rcg 専用として `rcsc/rcg/` に置くか)。→ `rcsc/gz/` に置く (D7)。名前は実装の時に決める。
- 段階 5: CI のどのジョブに `libdeflate-dev` を入れるか (rcssmonitor は、Autotools のジョブに入れず、両方の経路をビルドしている)。→ CMake のジョブにだけ入れる (D8)。
- 段階 6: ボールの高さ (`z`、`vz`) を `BallT` に加えるか (rcssmonitor の B-5)。加えるなら、書き出し (`serializer_json` など) も直すか。→ 今回は加えない (D9)。

## 4. 設計

### 4.1 段階 1: 計測と回帰テストの土台

#### 4.1.1 設計

- ベンチマーク `src/rcg_parser_bench.cpp` (S7)。
  - 使い方: `rcg_parser_bench [-n 回数 (既定 5)] <ログ>...`。
  - 読み込み (展開を含む) と解析を分けて計り、それぞれの最良の値とフレームの数を出す。ハンドラは show を `std::vector` にためる (soccerwindow2 の使い方に近づける)。
  - 段階 1 では今の経路 (`gzifstream` + `Parser::create( istream )`) を計る。段階 3 以降で、メモリの経路を加える。
- 回帰テスト `rcsc/rcg/test_parser.cpp` (`run_test_rcg_parser`、S6)。
  - テストのデータ: rcssmonitor の `test/data/head_json.rcg`・`head_v5.rcg` と、§2.7 の ULG6 のログの先頭から作る `head_v6.rcg`。
  - ハンドラに渡されたものを記録するテスト用のハンドラで、記録の数・順序・主な値を確かめる。
  - 段階 1 では、今の動作で通るものだけを入れる。後の段階で足していく。
- 全体のログでの比較 (CI では動かさない手順): 変更前と変更後の librcsc でビルドした `rcgverconv` (または `rcg2txt`) で §2.7 のログを書き直し、出力を比べる。手順をこの段階で決めて記録する。
- 今の librcsc で §2.7 のログの速さ、展開後の大きさ、フレームの数を記録する。

#### 4.1.2 実装 (2026-11-24)

- ベンチマーク `src/rcg_parser_bench.cpp` (S7): CMake (`src/CMakeLists.txt`) と Autotools (`src/Makefile.am` の `noinst_PROGRAMS`) でビルドする。インストールしない。
  - 3 つの時間を計る。`stream` (`gzifstream` + `parse(istream)`。ツールと soccerwindow2 の経路)、`read` (`gzifstream` でメモリに読むだけ。展開の時間)、`parse` (メモリの上のデータ (`std::istringstream`) に `create()`・`parse(istream)`)。それぞれの最良と中央の値を出す。
  - 初めに、読み込んだ `librcsc.so` の場所を出す (Linux だけ。`dl_iterate_phdr()`)。
- 計測で見つけた落とし穴: 手元の `LD_LIBRARY_PATH` に `~/local/lib` (Debug でビルドしてインストールした librcsc) が入っていて、初めはそれを測っていた (JSON が 7.4 秒になった)。`LD_LIBRARY_PATH` は RUNPATH より先に探されるため。`env -u LD_LIBRARY_PATH` で動かすことにし、ライブラリの場所を出すようにした。`docs/build.md` にも書いた。
- 回帰テスト `rcsc/rcg/test_parser.cpp` (`run_test_rcg_parser`、9 項目):
  - テストのデータ (`rcsc/rcg/test_data/`): `head_v5.rcg` (27 行)・`head_json.rcg` (30 行) は rcssmonitor の `test/data/` と同じもの。`head_v6.rcg` (32 行) は、ULG6 のログ (§2.7) の 1〜23 行目 (ヘッダ、パラメータ、チームのグラフィック 2 つ)、422〜429 行目 (playmode、team、show 1〜5)、結果の msg の行から作った。
  - テスト用のハンドラが、渡されたものを順に文字列で記録する (例: `show 4`、`team 1 HELIOS2022 CYRUS 0 0`)。記録の列全体と、最後の show の主な値 (ボール、4 人のプレイヤー)、パラメータの値を確かめる。
  - 項目: v5・v6・JSON のストリーム、テキストと JSON の `parse(filepath)`、ライブ受信の `ParserV4::parseLine()`・`ParserSimdJSON::parseData()`、`handleShow()` が false でもテキストは最後まで読むこと、ヘッダのない入力。
  - JSON の `vq` は確かめない (B9。段階 6 で足す)。スタミナなどの大きい値は `float` に入るため、許す誤差を 0.01 にした。
  - CMake は `TEST_DATA_DIR` にソースの `test_data` を、Autotools は `$(abs_srcdir)/test_data` を渡す。Autotools では `EXTRA_DIST` にテストのデータを加えた。
- 全体の比較のスクリプト `test/compare_rcg_output.sh` (S11): 2 つの bin ディレクトリの `rcgverconv` で、各ログをほかの 2 つの形式 (テキスト v4・v5 → JSON と v6、v6 → JSON と v5、JSON → v6 と v5) に変換し、`cmp` で比べる。`LD_LIBRARY_PATH` を外して動かす。版はヘッダを直接読んで判定する (`rcgversion` は v6 を判別できない。B12)。
- `docs/build.md`: テストの一覧に `run_test_rcg_parser` を加え、ベンチマークと比較のスクリプトの使い方を書いた。
- 確かめたこと:
  - CMake (Release): ビルドの警告 0。`ctest` で 17 個すべて通った。
  - Autotools (ソースを写した所で、ビルドディレクトリを分けて): コンパイラの警告 0 (libtool の既知のメッセージだけ)。`make check` で 13 個すべて通った。`make distcheck` (`--enable-unit-test`) も通り、テストのデータとソースが配布物に入っていることを確かめた。
  - 比較のスクリプト: 変更前のインストールとビルドの木で、6 つの変換がすべて同じだった (ライブラリは変えていない)。1 バイト変える偽の `rcgverconv` で、違いを検出することも確かめた。
  - ハンドラの戻り値の扱い (§2.1) を、小さなプログラムで確かめた。
- 今の librcsc の速さ (5 回の最良、ms):

| ログ | ビルド | stream | read | parse |
|---|---|---|---|---|
| JSON | CMake (-O3) | 205.9 | 77.1 | 152.0 |
| ULG5 | CMake (-O3) | 123.6 | 54.9 | 79.3 |
| ULG6 | CMake (-O3) | 90.5 | 41.3 | 57.3 |
| JSON | Autotools (-O2) | 217.5 | 82.0 | 161.8 |
| ULG5 | Autotools (-O2) | 130.8 | 59.0 | 84.0 |
| ULG6 | Autotools (-O2) | 95.6 | 43.1 | 61.1 |

- rcssmonitor の変更前 (§2.5 の A1。DispHolder に入れるまで) の JSON 235 ms・ULG5 130 ms と、同じくらいである。

### 4.2 段階 2: simdjson 5.1.1 への更新、`parser_json` の移動

#### 4.2.1 設計

- `rcsc/rcg/simdjson/` の `simdjson.h`・`simdjson.cpp`・`LICENSE` を 5.1.1 のものに置き換える (D2、S8)。GCC 15 の修正は本家に入っているため、当てない。
- API の違い: rcssmonitor で直したのは、`get_string( std::string & )` の結果を使っていない所 (チームのグラフィック) だけだった。librcsc の同じ所も直す。
- `parser_json.{h,cpp}` を `git mv` で `obsolete/rcg/` に移し、`nlohmann/json.hpp` は削除する (D3、D6)。`parser.cpp` の、コメントアウトされた参照を消す。
- GCC 15 の警告 9 件が消えたことを、CI (Ubuntu 26.04) で確かめる。
- `simdjson.cpp` のコンパイルの時間と、`librcsc.so` の大きさを記録する。
- On-Demand の実装 (§2.2、S9): `parser_simdjson.cpp` を仮に haswell 向けのフラグでコンパイルし、ベンチマークで差を測る (コミットしない)。結果を記録し、§3.3 の判断に使う。

#### 4.2.2 実装 (2026-11-24)

- simdjson: 本家のタグ v5.1.1 (`578a900`) の `singleheader/simdjson.h`・`singleheader/simdjson.cpp` と `LICENSE` に置き換えた。rcssmonitor の `ab78451` のものと同じであることを、SHA-256 で確かめた (S8)。3.8.0 に当てていた GCC 15 の修正 (3 行) は、5.1.1 に入っている。
- `parser_simdjson.cpp`: チームのグラフィックの文字列を `xpm_data.emplace_back( std::string_view( s.get_string() ) )` で読むようにした (rcssmonitor と同じ)。5.x では、`get_string( std::string & )` の結果を使わないと警告になるため。誤りのある値は例外になり、ほかの値と同じく catch される。ファイルの説明のコメント (`\file parser_json.cpp`) の誤りも直した。
- `parser_json.{h,cpp}` を `git mv` で `obsolete/rcg/` に移し、`nlohmann/json.hpp` を削除した (D6)。そのため、`obsolete/rcg/parser_json.cpp` はそのままではコンパイルできない。`parser.cpp` のコメントアウトされた参照を消した。
- テスト: `run_test_rcg_parser` に `testJsonTeamGraphic` を足した (10 項目)。変えた所 (JSON のチームのグラフィック) が `head_json.rcg` にないため。実際のログの記録を使う。変更前のライブラリでも通ることを確かめた。
- `RELEASE_NOTES.md`: 段階 1 のテストとツール、simdjson の更新、`parser_json` の移動と nlohmann の削除を書いた。
- 確かめたこと:
  - CMake (Release、GCC 13.3): ビルドの警告 0。`ctest` で 17 個すべて通った。
  - Autotools: コンパイラの警告 0。`make check` で 13 個すべて通り、`make distcheck` (`--enable-unit-test`) も通った。配布物に simdjson の 3 つのファイルが入り、nlohmann と `parser_json` は入らないことを確かめた。
  - 全体の比較 (S11): 変更前 (`212f5e7`) のインストールと比べて、6 つの変換がすべて同じだった。JSON のログにはチームのグラフィックが 256 個ある。
  - CI (`e778865`): Ubuntu 26.04 (GCC 15.2) の CMake と Autotools のジョブで、コンパイラの警告が 0 になった (段階 1 までは simdjson の 9 件)。
- 大きさとコンパイルの時間 (`-O3`、1 ファイルずつ `/usr/bin/g++` で。手元の CMake のビルドは ccache を通るため、全体のビルドの時間は比べられない):

| 項目 | 3.8.0 | 5.1.1 |
|---|---|---|
| `simdjson.h` | 3,840,917 バイト | 10,071,593 バイト |
| `simdjson.cpp` | 1,828,872 バイト | 3,382,592 バイト |
| `simdjson.cpp` のコンパイル | 2.95 秒 | 8.61 秒 |
| `parser_simdjson.cpp` のコンパイル | 5.19 秒 | 6.22 秒 |
| `librcsc.so` (CMake、Release) | 3,730,656 バイト | 4,066,064 バイト |

- 速さ (CMake の Release、5 回の最良、ms)。「haswell」は、ライブラリ全体を `-march=haswell` でビルドした別のビルド (コミットしない):

| ログ | stream (段階 1) | stream | parse (段階 1) | parse | parse (haswell) |
|---|---|---|---|---|---|
| JSON | 205.9 | 201.1 | 152.0 | 149.3 | 150.0 |
| ULG5 | 123.6 | 124.0 | 79.3 | 79.3 | 79.8 |
| ULG6 | 90.5 | 90.6 | 57.3 | 57.6 | 57.4 |

- simdjson の更新で、JSON は少し速くなった (rcssmonitor の A2 でも 235 → 222 ms だった)。テキストは変わらない (simdjson を使わない)。
- On-Demand の実装 (§2.2、§3.3、S9): haswell のビルドでは、On-Demand の解析部が `simdjson::haswell::ondemand` になった。それでも JSON の `parse` は 150.0 ms で、fallback (149.3 ms) と差がなかった。今の JSON の解析では、SIMD が効く部分 (文字列の処理など) 以外が時間の大半を占めているとみられる。対象の CPU を選ぶ手段は設けない (S14)。

### 4.3 段階 3: メモリからの解析、ファイルの一括読み込み

#### 4.3.1 設計

- `Parser` に、次の 2 つを加える。
  - `static Ptr create( const char * data, const std::size_t size )`
  - `virtual bool parse( const char * data, const std::size_t size, Handler & handler ) const`
- 基底の実装は、バッファを写さない `std::streambuf` (rcssmonitor の `MemoryBuf`。`seekoff`・`seekpos` を持つ) で包み、今の `create(istream)`・`parse(istream)` を呼ぶ。v1〜v3 のパーサは変えずにこの経路を使える。派生クラスには `using Parser::parse;` を加える。
- `ParserV4`:
  - `parse(data,size)` は、`std::getline` を使わずにメモリの上で行に分けて解析する。
  - `parse(istream)` は、`seekg(0)` の後にストリームを `std::string` に写して、メモリの経路を呼ぶ。
- `ParserSimdJSON`:
  - 1 レコード 1 行 (rcssserver の書き方) なら、行ごとにバッファの上で解析する (`padded_string_view`)。後ろに `SIMDJSON_PADDING` バイトの余裕がない、最後の数レコードだけを写す。
  - そうでなければ (1 行の文書など)、今のとおり文書全体を解析する。
  - `parse(istream)` での写しを 1 回に減らす。
  - `handleLogVersion()` が false なら、読み込みをやめる (S12)。
- ファイルを読む関数 (S3。`rcsc/gz/` に置く。D7。名前は実装の時に決める):
  - gzip でなければ、そのまま読む。
  - zlib があれば、`gzread` で 1 MB ずつ読む (`gzbuffer`。gzip の ISIZE で大きさを見積もる)。途中で切れた gzip は、切れた所まで読む。
  - 基底の `parse(filepath)` と `ParserSimdJSON::parse(filepath)` を、この関数で読むようにする。
- ツール (`src/`): 読む関数、`create(data,size)`、`parse(data,size,h)` に替える。`rcgresultprinter` の一時ファイルをやめる。

#### 4.3.2 実装 (2026-11-25)

- `rcsc/gz/gzread_file.{h,cpp}` (新規。D7、S16): `rcsc::gzread_file()`。インストールし、`rcsc/gz.h` にも加えた。
  - 先頭の 2 バイトで gzip かを見る。gzip でなければ、`std::ifstream` で大きさを調べて一度に読む。
  - gzip なら、zlib の `gzread()` で 1 MB ずつ読む (`gzbuffer()` も 1 MB)。複数のメンバ、後ろのゴミは zlib のとおり。途中で切れたファイルは、`gzerror()` が `Z_BUF_ERROR` のときに警告を出し、そこまでのデータで true を返す。
  - 確保の見積もりに、末尾の ISIZE を `min( ISIZE, ファイルの大きさ × 1032 )` で使う。
- `rcsc/rcg/line_parser.{h,cpp}` (新規。インストールしない): `LineRef`、`split_lines()` (rcssmonitor と同じ。`memchr` で分け、`std::getline()` と同じ分け方)、`read_stream()` (ストリームの残りを 1 MB ずつメモリに読む)。並列の解析は段階 4 で足す。
- `Parser` (`parser.h`・`parser.cpp`):
  - `static Ptr create( const char * data, std::size_t size )` と `virtual bool parse( const char * data, std::size_t size, Handler & ) const` を加えた。基底の実装は、写さない `MemoryBuf` (rcssmonitor と同じ) で包み、今の `create(istream)`・`parse(istream)` を呼ぶ。
  - `parse( const std::string & filepath, ... )` は `gzread_file()` で読み、`parse( data, size, ... )` を呼ぶ。gzip を読める。
  - `parser_v1.h`〜`parser_v4.h`・`parser_simdjson.h` に `using Parser::parse;` を加えた (§2.1 の、`parse(filepath)` が隠れる問題が直った)。`ParserSimdJSON::parse( filepath )` の上書きは消し、基底のものを使う。
- `ParserV4`: `parse( data, size, ... )` を上書きし、`split_lines()` で分けた行を順に `parseLine()` に渡す。`parse(istream)` は `read_stream()` でメモリに読んでから、これを呼ぶ。ヘッダの扱い、解析できない行でやめることは変えていない (S17)。
- `ParserSimdJSON`:
  - `parse( data, size, ... )` を上書きした。初めに `handleLogVersion( -1 )` を呼び、false ならやめる (S12)。
  - 1 行 1 レコードなら (`is_record_per_line()`。S15)、`Impl::parseLines()` で行ごとに解析する。行の前後の空白と末尾の `,` を除き、空の行と `[`・`]` だけの行は飛ばす。各レコードは `Impl::parseRecord()` で、後ろに `SIMDJSON_PADDING` バイトの余裕があれば `padded_string_view` でバッファの上で解析し、なければ (最後の数レコード) `padded_string` に写す。失敗した行で読み込みをやめる (S17)。
  - そうでなければ、`Impl::parseLog()` で今のとおり文書全体を解析する。
  - `parse(istream)` は `read_stream()` でメモリに読んでから `parse( data, size, ... )` を呼ぶ。写しは 2 回から 1 回 (文書全体のときは 2 回) になった。
- ツール (`src/`): `rcg2csv`・`rcg2txt`・`rcgrenameteam`・`rcgreverse`・`rcgvalidator`・`rcgverconv`・`rcgresultprinter` を、`gzread_file()`・`create( data, size )`・`parse( data, size, ... )` に替えた。
  - `rcgresultprinter` の、gzip を一時ファイル `temp.rcg` に展開する処理をなくした。
  - B13: `rcg2csv` と `rcgvalidator` の `handleLogVersion()` が JSON (-1) を受け付けるようにした。S12 で JSON のパーサが戻り値を見るようになり、`rcg2csv` の JSON の出力が空になったことで見つけた。
  - B14: `rcgresultprinter` の 2 か所の `tm` を初期化し、`mktime()` の前に `tm_isdst = -1` にした。ツールの出力を比べたとき、日時が変更前と 1 時間ずれたことで見つけた (変更前の出力も、テキストのログでは 1 時間ずれていた)。
- ベンチマーク: `file` (`gzread_file()`)、`memory` (`create( data, size )`・`parse( data, size, ... )`)、`total` (両方。ツールの経路) を加えた。
- テスト:
  - `run_test_rcg_parser` (20 項目。zlib のないビルドでは 19): メモリの経路 (v5・v6・JSON)、最後の行に改行がない場合、`ParserV4` から直接 `parse(filepath)`、gzip のファイルの `parse(filepath)` (テストの中で `gzofstream` で作る)、JSON の最後の `]` がないログ・1 行の文書・整形された文書・CRLF、`handleLogVersion()` が false のとき (JSON とテキスト) を足した。
  - `run_test_gzread_file` (新規。`rcsc/gz`、7 項目。zlib のないビルドでは 3): 普通のファイル (`\r\n` と `\0` を含む)、空のファイル、ないファイル、5 MB の gzip、複数のメンバ (最後のメンバが小さく、見積もりより大きくなる)、途中で切れた gzip、後ろのゴミ。
  - S15 の判定を一時的に rcssmonitor と同じにすると、`testJsonPrettyPrinted` だけが失敗することを確かめた。
- 確かめたこと:
  - CMake (Release): ビルドの警告 0。`ctest` で 18 個すべて通った。
  - CMake (zlib なし。`-DCMAKE_DISABLE_FIND_PACKAGE_ZLIB=ON`): 新しいファイルからの警告 0 (zlib なしでもとから出る、`rcsc/gz` の使わない引数の警告だけ)。rcg と gz のテストが通った。gzip のログは読めず、普通の rcg は読めることを確かめた。
  - Autotools: コンパイラの警告 0。`make check` で 14 個すべて通り、`make distcheck` (`--enable-unit-test`) も通った。`gzread_file.h` がインストールされ、`line_parser.h` はインストールされず配布物に入ることを確かめた。インストールしたヘッダは単独でコンパイルできた (`./cmake-helper.sh check-headers`)。
  - 全体の比較 (S11): 変更前 (`212f5e7`) と比べて、§2.7 の 3 つのログと、圧縮していない ULG6 のログ (`20250220172149-HELIOS2024_1-vs-RoboCIn_4.rcg`) の 8 つの変換がすべて同じだった。
  - ほかのツールの出力: `rcg2txt`・`rcg2csv`・`rcgvalidator`・`rcgresultprinter` の出力を変更前と比べた。違いは B13・B14 の修正によるもの (`rcgvalidator` の誤った警告がなくなった、`rcgresultprinter` の日時がログに合うようになった) だけだった。
- 速さ (CMake の Release、5 回の最良、ms):

| ログ | stream (段階 2) | stream | file | memory | total |
|---|---|---|---|---|---|
| JSON | 201.1 | 156.2 | 51.3 | 85.1 | 137.9 |
| ULG5 | 124.0 | 128.4 | 43.4 | 78.9 | 119.6 |
| ULG6 | 90.6 | 93.9 | 28.9 | 53.3 | 84.8 |

- ツールの経路 (`total`) は、JSON で 201.1 → 137.9 ms、テキストで 124.0 → 119.6 ms・90.6 → 84.8 ms になった。rcssmonitor の A3〜A5 (JSON 222 → 151 ms、テキスト 130 → 122 ms) と同じ傾向である。
- 読み込みは、`gzifstream` (`read`) より `gzread_file()` (`file`) が速い (JSON で 82.0 → 51.3 ms)。
- soccerwindow2 の経路 (`stream`) は、JSON で速くなった (201.1 → 156.2 ms) が、テキストではストリームをメモリに写す分だけ 3〜4 ms 遅くなった。段階 4 の並列化で取り戻せる見込み。

### 4.4 段階 4: 行の並列解析

#### 4.4.1 設計

- rcssmonitor の `line_parser.{h,cpp}` の残りを、段階 3 で作った `rcsc/rcg/line_parser.{h,cpp}` (インストールしない内部のヘッダ) に足す。
  - `split_lines()`: `memchr` で行に分ける。写さずに、バッファの中を指す (段階 3 で作った)。
  - `parse_lines()`: スレッドの数を決めて、行を連続したブロックに分け、ブロックごとに `std::thread` で解析する。`join` の後、ブロックの順に本当のハンドラへ再生する (S1)。
  - `RecordingHandler`: librcsc の `Handler` の仮想関数 (v4 と JSON のパーサが呼ぶもの) を、すべて記録する。`ServerParamT`・`PlayerParamT` は `copyFrom()` で写す。
  - `report_skipped()`: 飛ばした行の番号を、初めの 10 個と合計の数で出す。
- `Handler::handleTimestamp()` を仮想にする (記録できるようにするため。S4)。
- スレッドの数 (D5、S10):
  - `min( 上限, 行の数 / 1024 )`。上限の既定は `min( hardware_concurrency(), 8 )`。
  - `static void Parser::setMaxThreads( const int n )` と `static int Parser::maxThreads()` を加える。0 は既定、1 は並列にしない。値は `std::atomic<int>` で持つ。
- ハンドラは、呼び出したスレッドだけから、元の行の順に呼ばれる。ハンドラの戻り値で読み込みは止まらない (S13)。今の `ParserV4` も止まらないが、今の JSON のパーサは止まる (§2.1)。
- スレッドごとの状態: `ParserV4` は行のバッファ、`ParserSimdJSON` は `ondemand::parser`。解析の関数は const で、状態を持たない。
- ビルド:
  - CMake: `find_package(Threads REQUIRED)`、`target_link_libraries(rcsc PRIVATE Threads::Threads)`。
  - Autotools: `-pthread`。
  - パッケージの構成ファイルと `librcsc.pc` に影響がないかを、`test/` のインストールのテストで確かめる。

#### 4.4.2 実装 (2026-11-25)

- `Handler::handleTimestamp()` を仮想にした (S4)。
- `Parser::setMaxThreads( int n )`・`Parser::maxThreads()` (D5、S10): 値は `std::atomic< int >` で持つ。0 以下は既定 (`hardware_concurrency()` と 8 の小さい方)。明示した値は 8 より大きくてもそのまま使う。
- `rcsc/rcg/line_parser.{h,cpp}` に、rcssmonitor の残りを足した。
  - `count_threads( n_lines )`: `max( 1, min( Parser::maxThreads(), n_lines / 1024 ) )`。テストのために公開した。
  - `parse_lines()`: 行を連続したブロックに分け、ブロックごとに `std::thread` で記録用のハンドラに解析し、`join` の後、ブロックの順に本当のハンドラへ再生する (S1)。スレッドを作れなかった (`std::system_error`) ブロックは、呼び出したスレッドで解析する。1 スレッドのときも、行ごとに記録してから再生する (S19)。失敗した行の番号を返す。
  - `RecordingHandler`: rcssmonitor と同じく、仮想関数ごとに引数を写したラムダを記録する (`ServerParamT`・`PlayerParamT` は `copyFrom()`)。作る時に、本当のハンドラの `logVersion()`・`serverVersion()`・`timestamp()` を写す (パーサが参照するときのため)。
  - `report_skipped()`: 飛ばした行の番号を、初めの 10 個と合計で出す。
- `ParserV4::parse( data, size, ... )`: ヘッダの後の行を `parse_lines()` で解析する。スレッドごとに行のバッファ (`std::string`) を持つ。
- `ParserSimdJSON::Impl::parseLines()`: レコードの行を集めてから `parse_lines()` で解析する。スレッドごとに `simdjson::ondemand::parser` を持つ。`Impl` の関数は `Impl` を変えないので、スレッドで共有する。
- 解析の関数がスレッドで共有して安全なことを確かめた: `parser_v4.cpp`・`parser_simdjson.cpp`・`types.cpp`・`util.cpp` に、変わる静的な状態はない (読み出し専用の `to_playmode_enum()` の表だけ)。パーサはハンドラの状態 (`logVersion()` など) を参照していない。
- ビルド: CMake は `find_package(Threads REQUIRED)` を加え、`rcsc_rcg` と `rcsc` に `Threads::Threads` を PRIVATE でリンクする。Autotools は `rcsc/rcg/Makefile.am` に `-pthread` を、`librcsc.la` のリンクに `-pthread` を加えた。
- ベンチマーク: `-t <Threads>` (スレッドの数の上限) を加え、初めにその値を出す。
- テスト:
  - `run_test_rcg_line_parser` (新規、8 項目): `split_lines()`、`setMaxThreads()`・`maxThreads()`・`count_threads()`、20,000 行を 4 スレッドで解析して順序が保たれ、ハンドラが呼び出したスレッドだけから呼ばれること、1 スレッドのとき呼び出したスレッドだけで解析すること、失敗した行と例外を出した行を飛ばすこと、ハンドラの戻り値を使わないことと、ハンドラの例外が外に出ること (1 スレッドと 4 スレッドの両方)、`handleServerVersion()`・`handleTimestamp()` が渡り、スレッドの中で `logVersion()` を参照できること。
  - S19 を一時的に rcssmonitor と同じ (1 スレッドのとき本当のハンドラを直接渡す) にすると、ハンドラの戻り値と例外の 2 項目が失敗することを確かめた。
  - `run_test_rcg_parser` (25 項目): 5,000 個の show の大きなテキストと JSON のログを、4 スレッドと 1 スレッドで解析し、同じものが同じ順に渡ること、解析できない行を飛ばして続けること (テキストは今の `parseLine()` が失敗とする、括弧で始まらない行と空行。B3・B5 の例は段階 6 で足す)、JSON でもハンドラの false で止まらないこと (S13) を足した。
- 確かめたこと:
  - CMake (Release): ビルドの警告 0。`ctest` で 19 個すべて通った。
  - Autotools: コンパイラの警告 0。`make check` で 15 個すべて通り、`make distcheck` (`--enable-unit-test`) も通った。`librcsc.so` のリンクに `-pthread` が渡ることを確かめた。
  - ThreadSanitizer (`-fsanitize=thread -O1`。GCC 13 は `-O2` で `parser_simdjson.cpp` の内部エラーになった。カーネルの ASLR の設定のため `setarch -R` で動かした): `run_test_rcg_line_parser`・`run_test_rcg_parser` と、ベンチマークでの実際のログ 2 つの読み込みで、データ競合の報告はなかった。
  - 全体の比較 (S11): 変更前 (`212f5e7`) と比べて、4 つのログの 8 つの変換がすべて同じだった。
- 速さ (CMake の Release、5 回の最良、ms)。「1 スレッド」は `-t 1`:

| ログ | total (段階 3) | total | total (1 スレッド) | memory (段階 3) | memory | stream (段階 3) | stream |
|---|---|---|---|---|---|---|---|
| JSON | 137.9 | 73.6 | 138.6 | 85.1 | 21.0 | 156.2 | 91.7 |
| ULG5 | 119.6 | 59.8 | 120.6 | 78.9 | 19.1 | 128.4 | 70.0 |
| ULG6 | 84.8 | 43.8 | 85.8 | 53.3 | 12.7 | 93.9 | 53.1 |

- 手元の 32 コアの計算機では、既定の上限は 8 スレッド。解析 (`memory`) は 4 分の 1 ほどになり、ツールの経路 (`total`) は半分ほどになった。残りの大部分は展開 (`file`: JSON 51.5 ms、ULG5 43.5 ms、ULG6 29.1 ms) で、並列にできない (段階 5 の libdeflate で速くする)。rcssmonitor の A6 (JSON 84 ms、テキスト 60 ms) と同じ水準である。
- 1 スレッドのときは段階 3 とほぼ同じ (行ごとに記録して再生する分は 1 ms ほど。S19)。
- soccerwindow2 の経路 (`stream`) も、テキストで段階 2 より速くなった (ULG5 124.0 → 70.0 ms)。

### 4.5 段階 5: libdeflate による展開

#### 4.5.1 設計

- 任意の依存とする (D4)。
  - CMake: `option(ENABLE_LIBDEFLATE ... ON)`、`find_path`・`find_library` で探す (古いディストリビューションには構成ファイルがないため。rcssmonitor と同じ)。
  - Autotools: `--with-libdeflate` (既定は `check`。明示して見つからなければエラー)。
  - どちらも `HAVE_LIBDEFLATE` を定義する。
- ファイルを読む関数で、gzip なら libdeflate で展開する。
  - 連結された複数のメンバと、後ろのゴミに対応する。出力のバッファが足りなければ倍にする。
  - 失敗した (途中で切れたファイルなど) ときは、zlib で読み直す。
- `librcsc.so` に PRIVATE でリンクする。`librcsc.pc` (`Libs.private`)、`librcsc.spec.in`、CMake のパッケージ構成ファイルを、必要に応じて直す (→ 変えない。S20)。
- CI: 一部のジョブに `libdeflate-dev` を入れ、残りには入れず、両方の経路をビルドする (§3.3。→ CMake のジョブに入れる。D8)。
- `README.md`・`docs/build.md` に依存を書く。

#### 4.5.2 実装 (2026-11-25)

- `rcsc/gz/gzread_file.cpp`: `HAVE_LIBDEFLATE` のとき、gzip のファイルは、まず全体をそのまま読み、`inflate_with_libdeflate()` (rcssmonitor の `inflateWithLibdeflate()` と同じ) で展開する。
  - 出力のバッファは、末尾の ISIZE とファイルの大きさの大きい方から始め、足りなければ倍にする (上限は `ファイルの大きさ × 1032 + 1 MB`)。
  - 連結された複数のメンバを、先頭が gzip の印 (`1f 8b`) である間、`libdeflate_gzip_decompress_ex()` で続けて展開する。後ろの gzip でないバイトは無視する (zlib と同じ)。
  - 展開に失敗した (途中で切れたファイルなど) ときは、zlib があれば zlib で読み直し、切れた所まで読む。zlib がなければ、`broken gzip data` を出して false を返す。
  - `gzread_file.h` の説明に、libdeflate を使うことを書いた。
- 検出 (S20):
  - CMake: `option(ENABLE_LIBDEFLATE ... ON)`。`find_path( libdeflate.h )`・`find_library( deflate libdeflate )` で探し、見つかれば `HAVE_LIBDEFLATE` を定義する (`cmake-config.h.in`)。`rcsc_gz` に include のディレクトリを、`rcsc` にライブラリを PRIVATE で加える。構成の出力に `libdeflate: ...` の行を出す。
  - Autotools: `--with-libdeflate` (既定は `check`。`--without-libdeflate` で使わない。`--with-libdeflate` で見つからなければエラー)。`AC_CHECK_HEADER( libdeflate.h )` と `AC_CHECK_LIB( deflate, libdeflate_gzip_decompress_ex )`。`LIBDEFLATE_LIBS` を `librcsc.la` (`rcsc/Makefile.am`) と、`rcsc/gz`・`rcsc/rcg` の単体テストに渡す。configure の出力に、どちらで展開するかを出す。
- CI (D8): CMake のジョブにだけ `libdeflate-dev` を入れる。「Show the versions」に libdeflate の版を出す。
- 文書: `README.md` (必要なもの、Ubuntu のインストールのコマンド)、`docs/build.md` (必要なものの表、インストール、CMake の `ENABLE_LIBDEFLATE`、configure の `--without-libdeflate`、構成の出力)、`RELEASE_NOTES.md`。
- 確かめたこと:
  - CMake (Release、libdeflate あり): ビルドの警告 0。`ctest` で 19 個すべて通った。`librcsc.so` が `libdeflate.so.0` に依存する。
  - どちらで展開したかを一時的に出して、`run_test_gzread_file` の gzip のファイル (1 MB を超えるもの、複数のメンバ、後ろのゴミ) は libdeflate で、途中で切れたファイルだけが zlib に戻って切れた所まで読むことを確かめた (出力は戻した)。
  - CMake (`-DENABLE_LIBDEFLATE=OFF`): libdeflate に依存しない。
  - CMake (zlib なし・libdeflate あり): rcg と gz のテスト 4 個が通った。gzip のログを読めた (`rcg2txt` の出力の行の数が変更前と同じ)。途中で切れた gzip は `broken gzip data` で読まない。
  - Autotools: 既定 (見つかる) では `HAVE_LIBDEFLATE` が定義され、`--without-libdeflate` では定義されず、ない環境での `--with-libdeflate` (`ac_cv_header_libdeflate_h=no` で再現) は configure がエラーになった。既定と `--without-libdeflate` の両方で、コンパイラの警告 0、`make check` の 15 個が通った。`librcsc.so` が libdeflate に依存するのは既定のときだけ。`make distcheck` も通った。
  - 全体の比較 (S11): 変更前 (`212f5e7`) と比べて、4 つのログの 8 つの変換がすべて同じだった。
- 速さ (CMake の Release、5 回の最良、ms)。「zlib」は `-DENABLE_LIBDEFLATE=OFF`:

| ログ | file (zlib) | file (libdeflate) | total (zlib) | total (libdeflate) |
|---|---|---|---|---|
| JSON | 51.2 | 24.8 | 73.9 | 47.3 |
| ULG5 | 43.9 | 20.4 | 60.5 | 36.3 |
| ULG6 | 29.2 | 14.5 | 44.0 | 25.9 |

- 展開はおよそ半分になり、ツールの経路はおよそ 35〜40% 短くなった。rcssmonitor (展開だけで zlib 69 / 48 ms、libdeflate 34 / 22 ms) と同じ傾向である。
- soccerwindow2 の経路 (`gzifstream` + `parse(istream)`) は libdeflate を使わない (`gzifstream` は zlib のまま。`docs/todo.md` に記録した soccerwindow2 の切り替えで使えるようになる)。

### 4.6 段階 6: 読み込みの堅牢化

#### 4.6.1 設計

- `rcgversion` (B12): v6 を判別し、知らない版は `json` ではなく、知らないと表示する。
- テキスト (§2.6 の B1〜B7):
  - 記録の名前を `%31[^ ()]` で読む。`unum < 1` を拒む。`strtol`・`strtof` の失敗を、読み進んだかで調べる。ヘッダの版を `std::atoi` で読む。
  - `parseLine()` は各記録の解析の結果を返し、空行は true とする。読み込みは止めずに、飛ばした行をまとめて報告する (S5)。
  - チームのグラフィックの文字列を、長さによらず読む。`parseTeam()` でもチーム名 `null` を空にする。
- JSON (B8〜B11):
  - 知らない記録とキーは飛ばし、1 つのプロセスで 1 度だけ報告する (`std::mutex` で守る)。
  - `version`・`timestamp` をハンドラに渡す。
  - `vq` を `'h'`・`'l'` で入れる。
  - プレイヤーを side と unum の位置に置き、正しくない side・unum は飛ばす。省かれうるフィールドは既定の値のままにする。
- ボールの高さは §3.3 で決める。rcssmonitor の B-9 (パラメータの既定値) は対象外とする。

#### 4.6.2 実装 (2026-11-25)

- テキスト (`parser_v4.cpp`):
  - B1: 記録の名前を `" ( %31[^ ()] "` で読む。
  - B2: `unum < 1` を拒む (0 は `player_[-1]` に書いていた)。
  - B3: show の時刻とボールの 4 つの値を、`strtol()`・`strtof()` が読み進んだかで確かめる。時刻の確かめは rcssmonitor にはなく、librcsc で足した (ボールの確かめは rcssmonitor と同じ)。
  - B4: ヘッダの版を `std::atoi()` で読む。
  - B5: `parseLine()` は空白だけの行で true を返し、各記録の解析の結果を返す。知らない記録は、名前ごとに 1 度だけ報告して true を返す (S21)。
  - B6: チームのグラフィックの文字列を、`strchr()` で `"` の組を探して、長さによらず読む。
  - B7: `parseTeam()` でも、チーム名 `null` を空にする。
  - msg の最小の長さの確かめを、rcssmonitor と同じく `")` の 2 文字に直し、短すぎるときに報告するようにした。
- JSON (`parser_simdjson.cpp`。rcssmonitor の `read_double()`・`read_int()`・`read_team()`・`parsePlayer()`・`parseTeams()`・`parseShow()` を移した):
  - B8: `version`・`timestamp` を `handleServerVersion()`・`handleTimestamp()` に渡す。
  - B9: `vq` を `'h'`・`'l'` で入れる。
  - B10: show は時刻とボールだけを必須とし、プレイヤーを side と unum の位置に置く。正しくない side・unum は飛ばし、同じプレイヤーが 2 度あれば後のもので置き換える。省かれうるフィールドは既定の値のまま。ライブの show の `mode`・`team` は、show の時刻で渡す。ボールの高さは読まない (D9)。
  - B11: 知らない記録とキーは飛ばし、プロセスで 1 度だけ報告する (`std::mutex` で守る)。
  - ライブの入口 `parseData()` で例外を catch し、壊れたメッセージで false を返す。
- B12: `rcgversion` は v6 を判別し、知らない版と短すぎるファイルは `unknown` と表示する。
- S22: `rcgreverse`・`rcgrenameteam`・`rcgverconv` に `begin()` を加え、出力の先頭を最初の記録の前に 1 度だけ書く。B8 を直しても、ツールの JSON の出力が `"version":"unknown"` のままだったことで見つけた。
- テスト (`run_test_rcg_parser`、35 項目): JSON の値の確かめに version・timestamp・`vq` を足した (大きなログの並列の経路でも)。B1〜B11 の項目 (`testTextParseLine`、`testTextUnumZero`、`testTextBrokenHeader`、`testTextTeamGraphicLong`、`testTextTeamNull`、`testJsonPlayers`、`testJsonShowWithoutPlayers`、`testJsonUnknownRecords`、`testJsonLiveShowWithTeam`、`testJsonLiveBroken`) を足し、大きなテキストのログの壊れた行に、知らない記録・壊れた show・空白だけの行を加えた。
- 確かめたこと:
  - 新しいテストを段階 5 (`dfcbed2`) のライブラリにかけると、35 項目のうち 20 項目が失敗した (B1〜B11 の項目と、version・timestamp・`vq` を確かめる JSON の項目)。今のライブラリでは 35 項目すべて通る。
  - CMake (Release): ビルドの警告 0。`ctest` で 19 個すべて通った。
  - AddressSanitizer と UndefinedBehaviorSanitizer (`-fsanitize=address,undefined -O1`): `run_test_rcg_parser`・`run_test_rcg_line_parser`・`run_test_gzread_file` と、`rcgverconv` での実際のログの変換 (JSON → v6、v5 → JSON) で、エラーはなかった。テストの補助の関数 (`test_gzread_file.cpp` の `make_text()`) の `int` のあふれを見つけ、`long long` に直した。
  - Autotools: コンパイラの警告 0。`make check` で 15 個すべて通り、`make distcheck` (`--enable-unit-test`) も通った。
  - 全体の比較 (S11): テキストのログの 6 つの変換は、変更前 (`212f5e7`) と同じだった。JSON のログの 2 つの変換 (→ v6、→ v5) は、6,209 行の show の `vq` の所だけが違った (変更前は 136,598 か所に制御文字 `\x01` を書いていた。今は `h`)。
  - `rcgreverse`・`rcgrenameteam`: テキストのログの出力は変更前と同じ。JSON のログの出力の違いは、version (`"unknown"` → `"18.1.3"`)、timestamp の行が加わったこと、`vq` (変更前は 136,598 か所すべてを `"l"` と書いていた。元のログはすべて `"h"`) だけだった。
  - `rcgverconv` は、同じ版への変換を今までどおり断る (出力は空)。
  - `rcgversion`: JSON は `json`、ULG5 は `5`、ULG6 は `6`、`ULG9` と短すぎるファイルは `unknown`。
- 速さ (CMake の Release、5 回の最良、ms): ツールの経路 (`total`) は JSON 43.2 (段階 5 は 47.3)、ULG5 36.7 (36.3)、ULG6 26.1 (25.9)。JSON の解析 (`memory`) は 21.0 → 17.5 ms で、少し速くなった。

### 4.7 段階 7: 仕上げ

#### 4.7.1 設計

- 最終の計測: zlib だけ・libdeflate、CMake・Autotools の組み合わせ。
- 全体のログで、変更前と変更後の出力を比べる (§4.1)。違いが、段階 6 の修正による想定どおりのものだけであることを確かめる。
- soccerwindow2 を新しい librcsc でビルドし、ログを開いて、速さと表示を確かめる (ユーザの確認)。soccerwindow2 側の変更は別の計画で行う。
- `RELEASE_NOTES.md`、`docs/build.md` (テストの一覧、依存、ベンチマークの使い方) を書く。

#### 4.7.2 実装 (2026-11-25)

- 最終の計測 (5 回の最良、ms)。段階 6 (`9e29dc9`) のコードを、4 つの組み合わせでビルドした。CMake は Release (`-O3`)、Autotools は既定 (`-g -O2`)。「zlib」は libdeflate なし (`-DENABLE_LIBDEFLATE=OFF`、`--without-libdeflate`)。`total` はツールの経路 (`gzread_file()` + `parse( data, size, ... )`)、`stream` は soccerwindow2 の経路 (`gzifstream` + `parse( istream )`):

| ログ | 段階 1 の stream | CMake・libdeflate | CMake・zlib | Autotools・libdeflate | Autotools・zlib | stream (CMake・libdeflate) |
|---|---|---|---|---|---|---|
| JSON | 205.9 | 43.9 | 72.4 | 45.1 | 70.9 | 89.1 |
| ULG5 | 123.6 | 35.8 | 60.8 | 35.9 | 61.2 | 74.2 |
| ULG6 | 90.5 | 26.1 | 43.8 | 25.9 | 43.7 | 54.8 |

- ツールの経路は、段階 1 と比べて、libdeflate ありで JSON 4.7 倍、テキスト 3.5 倍ほど速くなった。zlib だけでも 2〜3 倍ほど。soccerwindow2 の経路 (soccerwindow2 を変えない場合) も、JSON 2.3 倍、テキスト 1.7 倍ほどになった。
- 全体の比較 (S11): 変更前 (`212f5e7`) と新しい librcsc をそれぞれインストールした所の `rcgverconv` で、手元の 7 つのログ (§2.7 の 3 つ、圧縮していない ULG6、ほかの ULG6 3 つ) の 14 の変換を比べた。12 は同じで、JSON のログの 2 つ (→ v5、→ v6) は、`vq` の所 (B9) だけが違った。
- soccerwindow2 (`6c9d09c`、`develop`) での確認:
  - soccerwindow2 を変えずに、変更前 (`212f5e7`) と新しい librcsc をそれぞれ scratchpad にインストールした所に向けて (`-DLIBRCSC_INSTALL_DIR`)、CMake (Release、Qt 6.4) でビルドした。どちらも警告 0。soccerwindow2 のテスト (`ctest`) は、どちらでも 9 個のうち 8 個が通り、ライブのサーバが要る 1 個 (`monitor_client_live`) は飛ばされた。
  - 画面を使わずに (`QT_QPA_PLATFORM=offscreen`) `-l <ログ>` で開き、soccerwindow2 が出すパースの時間 (`RCG paser elapsed time`) を 3 回ずつ測った。フレームの数は同じだった:

| ログ | 変更前 (ms) | 新しい librcsc (ms) |
|---|---|---|
| JSON | 234.4〜242.3 | 112.7〜115.1 |
| ULG5 | 131.7〜149.5 | 94.3〜96.1 |
| ULG6 | 116.2〜120.3 | 78.2〜84.0 |

  - ユーザが画面で、3 つのログの表示と操作を確かめた (問題なし)。
- soccerwindow2 が rcssmonitor より遅い理由 (ユーザの問い): rcssmonitor は同じ JSON のログを 60 ms 弱で開く。soccerwindow2 の `MainData::openRCG()` を、`gzread_file()`・`create( data, size )`・`parse( data, size, ... )` に替えた試しのビルド (scratchpad のソースの写しで。soccerwindow2 のリポジトリは変えていない) で、読み込みと解析の時間を分けて測った (3 回ずつ):

| ログ | 今の soccerwindow2 (ms) | 読み込みを替えた場合 (ms) | 内訳: 読み込み + 解析 (ms) |
|---|---|---|---|
| JSON | 114.0〜117.5 | 54.0〜54.5 | 32.8〜33.0 + 21.2〜21.5 |
| ULG5 | 94.9〜96.9 | 47.5〜47.7 | 26.7〜27.6 + 20.0〜20.8 |
| ULG6 | 76.3〜78.3 | 41.1〜41.8 | 20.0〜20.1 + 21.1〜21.6 |

  - 解析 (soccerwindow2 のハンドラ `ViewHolder` に渡す分を含む) は約 21 ms で、どちらも同じ。差のほとんどは、`rcsc::gzifstream` を通した展開 (zlib を 8 KB ずつ。libdeflate を使わない) にかかる時間である。ベンチマークでは、`gzifstream` で読むのに約 83 ms、`gzread_file()` (zlib) で約 51 ms、`gzread_file()` (libdeflate) で約 25 ms かかる (JSON)。
  - soccerwindow2 の読み込みを替えれば、JSON で 54 ms ほどになり、rcssmonitor と同じ水準になる。この切り替えは `docs/todo.md` の項目 (soccerwindow2 の別の計画) で行う。測った値を `docs/todo.md` に書き足した。

## 5. 段階

| # | 作業 | 完了条件 |
|---|---|---|
| 1 | ベンチマーク、`run_test_rcg_parser` (今の動作)、全体の比較の手順 | 今の librcsc の速さを記録した。テストが両方のビルドで通る |
| 2 | simdjson 5.1.1、`parser_json` の移動 | GCC 15 の警告が 0 件 (CI)。テストが通る。速さとコンパイルの時間を記録した |
| 3 | メモリからの解析、ファイルを読む関数、ツールの切り替え | メモリの経路とストリームの経路で、ハンドラに渡るものが同じ。`parse(filepath)` が gzip を読める。速さを記録した |
| 4 | 行の並列解析、`setMaxThreads()` | 多くの行のテストで順序が保たれる。`setMaxThreads( 1 )` で並列にならない。速さを記録した |
| 5 | libdeflate | libdeflate あり・なしの両方でビルドとテストが通る (CI)。速さを記録した |
| 6 | 読み込みの堅牢化 | B1〜B12 のテストが通り、修正を戻すと失敗する |
| 7 | 仕上げ | 全体の比較で想定外の違いがない。soccerwindow2 で確かめた (ユーザ)。リリースノートを書いた |

## 6. テスト

- `run_test_rcg_parser` (`rcsc/rcg/test_parser.cpp`、段階 1 から足していく)
  - テキスト (v5・v6) と JSON の、ストリームとメモリの両方の経路。
  - JSON: 最後の `]` がないログ、末尾の `,`、壊れた行、1 行の文書 (文書全体の経路)、CRLF、知らないキー。
  - 多くの行 (5,000 個の show) で、並列の経路を通しても順序が保たれること。飛ばした行が報告されること。
  - 堅牢化 (B1〜B11) の各項目。
- `run_test_rcg_line_parser` (`rcsc/rcg/test_line_parser.cpp`、段階 4)
  - `split_lines()` (`\r` を残す、最後の `\n` で空の行を作らない、空の入力)。
  - 20,000 行の順序。失敗した行・例外を出した行の報告。`setMaxThreads( 1 )`。
- `run_test_gzread_file` (`rcsc/gz/test_gzread_file.cpp`、段階 3・5。gzip の項目は zlib があるときだけ)
  - 普通のファイル、1 MB を超える gzip、連結された複数のメンバ、途中で切れた gzip、後ろのゴミ、ないファイル。
- 各段階のテストは、その段階の修正を戻すと失敗することを確かめる。
- 手元での動かし方:
  - CMake: `./cmake-helper.sh test` (または `cd build && ctest --output-on-failure -R rcg`)。
  - Autotools: `./configure --enable-unit-test` の後に `make check`。
  - ベンチマーク: `env -u LD_LIBRARY_PATH build/src/rcg_parser_bench -n 5 <ログ>... 2>/dev/null`。初めに、読み込んだ `librcsc.so` の場所を出す。
  - 全体の比較: `test/compare_rcg_output.sh <古い bin> <新しい bin> <ログ>...`。

## 7. 変更するファイル (見込み)

- `rcsc/rcg/`: `parser.{h,cpp}`、`parser_v1.h`〜`parser_v3.h` (`using`)、`parser_v4.{h,cpp}`、`parser_simdjson.{h,cpp}`、`handler.h`、`line_parser.{h,cpp}` (新規)、`simdjson/*`、`test_parser.cpp`・`test_line_parser.cpp` (新規)、`test_data/` (新規)、`CMakeLists.txt`、`Makefile.am`
- `rcsc/gz/gzread_file.{h,cpp}`・`rcsc/gz/test_gzread_file.cpp` (新規。D7)、`rcsc/gz/Makefile.am`・`rcsc/gz/CMakeLists.txt`・`rcsc/gz.h`
- `obsolete/rcg/` (移動先)
- `src/`: rcg を読むツール、`rcg_parser_bench.cpp` (新規)、`CMakeLists.txt`、`Makefile.am`
- `CMakeLists.txt`、`configure.ac`、`cmake-config.h.in`、`librcsc.pc.in`、`librcsc.spec.in`、CMake のパッケージ構成ファイル (必要なら)
- `test/compare_rcg_output.sh` (新規)
- `.github/workflows/build.yml`
- `RELEASE_NOTES.md`、`docs/build.md`、`README.md`

## 8. 対象外

この章の項目は、`docs/todo.md` にも記録した (2026-11-25)。

- soccerwindow2 の変更 (メモリの経路への切り替え)。別の計画で行う。今の API のままでも解析は速くなるが、`gzifstream` での展開は遅いまま残る。
- ライブ受信 (`parseData()`、`parseLine()`) の速さ。1 パケットずつの解析で、問題にならない。
- simdjson のシンボルを `librcsc.so` から隠すこと。利用側が別の版の simdjson を使うと、ぶつかるおそれがある。記録だけする。
- On-Demand の、コンパイル時に選ばれる実装の既定を変えること、選ぶ手段を設けること (S9、S14。段階 2 で測って差がなかった)。
- パラメータの既定値の見直し (rcssmonitor の B-9)。
- `rcsc::gzifstream` 自体の速さ (バッファの大きさなど)。
- v1〜v3 (バイナリ) のパーサの速さ。
- `example/` の修正。
- ボールの高さ (`ball.z`、`ball.vz`) を読むこと (D9)。

## 9. 記録

### 2026-11-24: 計画を作成した

- librcsc の RCG パーサ (§2.1〜§2.4)、rcssmonitor の PR #65 (§2.5)、見つかっている不具合 (§2.6) を調べた。
- simdjson の On-Demand の解析部が `fallback` で動いていることを、手元のビルドのシンボルで確かめた (§2.2)。
- ユーザの決定 D1〜D5 を記録した。D2 は、本家の最新 (5.3.0、2026-11-22) ではなく、rcssmonitor と同じ 5.1.1 とした。
- この後: ユーザが GitHub でブランチを作った後、段階 1 に着手する。計画書は段階 1 の実装と一緒にコミットする。

### 2026-11-24: 段階 1 を実装した

- ユーザが `feature/rcg-parser-speedup` を作った (`212f5e7` から)。
- ベンチマーク `rcg_parser_bench`、回帰テスト `run_test_rcg_parser` (9 項目)、全体の比較のスクリプト `test/compare_rcg_output.sh` を作った (§4.1.2)。
- 今の librcsc の速さを記録した (§4.1.2)。CMake の Release で、JSON 205.9 ms、ULG5 123.6 ms、ULG6 90.5 ms (`stream`)。
- 初めは、`LD_LIBRARY_PATH` の Debug の librcsc を測っていた (JSON が 7.4 秒)。ベンチマークが読み込んだライブラリを出すようにした。
- 見つけたこと: `rcgversion` が v6 を判別できない (B12)。`ParserV4` などで `parse(filepath)` が隠れている (§2.1。段階 3 の `using Parser::parse;` で直る)。JSON のパーサはハンドラの戻り値で止まり、`handleLogVersion()` の戻り値は無視する (§2.1)。これらから S11〜S13 を決めた。
- CMake の `ctest` 17 個、Autotools の `make check` 13 個と `make distcheck` が通った。
- ユーザの確認の後、`b4d1b5c` としてコミットし、プッシュした。CI は、PR のない機能ブランチへのプッシュでは動かない (ワークフローの起動は `master`・`develop` へのプッシュ、PR、手動の実行)。
- ユーザが下書きの PR #123 (`Speed up the rcg parser and update the bundled simdjson to 5.1.1`) を作り、CI が動いた。
- CI の結果 (`b4d1b5c`): 5 ジョブすべて成功した。4 つのビルドのジョブで `run_test_rcg_parser` が通った (CMake は 17 個すべて、Autotools は `make check` と `make distcheck` で 13 個ずつ)。コンパイラの警告は、Ubuntu 26.04 の同梱の simdjson の既知の 9 件 (`-Wtemplate-id-cdtor`。Autotools では `distcheck` でもう一度ビルドするため 18 件) だけ。
- この後: 段階 2 (simdjson 5.1.1、`parser_json` の移動) に着手する。

### 2026-11-24: 段階 2 を実装した

- ユーザの決定 D6: `nlohmann/json.hpp` は `obsolete/` にも移さずに削除する (D3 を変えた)。
- 同梱の simdjson を 5.1.1 にし、`parser_json.{h,cpp}` を `obsolete/rcg/` に移し、`nlohmann/json.hpp` を削除した (§4.2.2)。API の違いで直したのは、rcssmonitor と同じ 1 か所 (チームのグラフィック) だけだった。
- `run_test_rcg_parser` に JSON のチームのグラフィックのテストを足した (10 項目)。
- CMake の `ctest` 17 個、Autotools の `make check` 13 個と `make distcheck` が通った。全体の比較で、変更前と出力が同じだった。
- JSON は 205.9 → 201.1 ms (`stream`) になった。`simdjson.cpp` のコンパイルは 2.95 → 8.61 秒になった。
- On-Demand を haswell 向けにしても速さは変わらなかったため、対象の CPU を選ぶ手段は設けないことにした (S14)。
- ユーザの確認の後、`e778865` としてコミットし、プッシュした。
- CI の結果 (`e778865`): 5 ジョブすべて成功した。4 つのビルドのジョブで `run_test_rcg_parser` が通った (CMake は 17 個すべて、Autotools は `make check` と `make distcheck`)。Ubuntu 26.04 (GCC 15.2) でも、コンパイラの警告は 0 になった。GCC 15 の警告 9 件は解消した。Autotools のジョブの `warning:` は、libtool の既知のメッセージだけ。
- この後: 段階 3 (メモリからの解析、ファイルの一括読み込み) に着手する。初めに、ファイルを読む関数の名前と置き場所を決める (§3.3)。

### 2026-11-24: 段階 3 の置き場所を決め、作業を中断した

- ユーザの決定 D7: ファイルを (gzip なら展開して) メモリに読む関数は `rcsc/gz/` に置く。
- ユーザが作業の場所を移すため、段階 3 の前で作業を中断した。計画書だけをコミットしてプッシュした。
- 再開するときの手順 (手元の環境に依るもの):
  - 計測に使うログ (§2.7) は Git で追跡していない。新しい場所にも写す。
  - 全体の比較 (S11) の「変更前」には、`212f5e7` (`develop`) をビルドしてインストールしたものを使う。例: `git worktree add <dir> 212f5e7` の後、`cmake -S <dir> -B <build> -DCMAKE_BUILD_TYPE=Release` と `cmake --install <build> --prefix <base>` で作り、`test/compare_rcg_output.sh <base>/bin <新しいビルド>/src <ログ>...` で比べる。
  - ベンチマークと比較のスクリプトは、`LD_LIBRARY_PATH` を外して動かす (§4.1.2)。インストールした librcsc (Debug など) を測ってしまうため。
  - 単体テストには CppUnit (Ubuntu では `libcppunit-dev`) が要る。
  - 速さは、ビルドの種類 (CMake の Release) と計算機によって変わる。新しい場所では、段階 2 の状態 (`e778865`) で測り直してから比べる。
- この後: 段階 3 に着手する。

### 2026-11-25: 段階 3 を実装した

- 作業の場所は移らなかった。scratchpad に残っていた変更前 (`212f5e7`) のインストールとビルドで続けた。
- `rcsc::gzread_file()`、`Parser` のメモリの入口 (`create( data, size )`・`parse( data, size, ... )`)、`ParserV4` と `ParserSimdJSON` のメモリ上での解析、ツールの切り替えを実装した (§4.3.2)。
- 決めたこと: S15 (JSON の 1 行ずつの判定を rcssmonitor より厳しくする)、S16 (`gzread_file()` の名前と動作)、S17 (段階 3 では失敗の扱いを変えない)。
- 見つけて直したこと: B13 (`rcg2csv`・`rcgvalidator` が JSON を断っていた)、B14 (`rcgresultprinter` の日時のずれ)。
- テスト: `run_test_rcg_parser` を 20 項目に増やし、`run_test_gzread_file` (7 項目) を足した。CMake の `ctest` 18 個、Autotools の `make check` 14 個と `make distcheck` が通った。zlib のないビルドも確かめた。
- 全体の比較で、変更前と出力が同じだった。ツールの経路は、JSON で 201.1 → 137.9 ms、ULG5 で 124.0 → 119.6 ms、ULG6 で 90.6 → 84.8 ms になった。
- この後: ユーザの確認の後、段階 3 をコミットしてプッシュし、CI を確かめる。その後、段階 4 (行の並列解析) に着手する。

### 2026-11-25: 段階 3 の CI の結果と、GCC 15 の警告の修正

- ユーザの確認の後、段階 3 を `20f8b9d` としてコミットし、プッシュした。
- CI の結果 (`20f8b9d`): 5 ジョブすべて成功した。4 つのビルドのジョブで `run_test_rcg_parser` と `run_test_gzread_file` が通った (CMake は 18 個すべて、Autotools は `make check` と `make distcheck`)。
- ただし、Ubuntu 26.04 (GCC 15.2) で、`test_parser.cpp` の `testGzipFilePath` に `-Wdangling-reference` の警告が 2 件出た (Autotools では `distcheck` でもう一度ビルドするため 4 件)。範囲 for 文で `{ std::make_pair( ... ), ... }` を回していた所で、実際には問題のない誤検出。手元の GCC 13.3 では、`-Wdangling-reference` を付けても出ない。gzip のファイルを作る関数 `write_gzip()` を 2 回呼ぶ形に書き換えた。
- CLAUDE.md に、対象外にしたり見送ったりした作業を `docs/todo.md` に記録するという指示が加わった。§8 の項目と S14 (On-Demand の CPU を選ぶ手段を設けない) を `docs/todo.md` に記録した。
- ユーザの確認の後、`a0adabe` としてコミットし、プッシュした。
- CI の結果 (`a0adabe`): 5 ジョブすべて成功した。Ubuntu 26.04 (GCC 15.2) を含む 4 つのビルドのジョブで、コンパイラの警告は 0 になった。
- この後: 段階 4 (行の並列解析) に着手する。

### 2026-11-25: 段階 4 を実装した

- 行の並列解析 (`parse_lines()`、`RecordingHandler`)、`Parser::setMaxThreads()`・`maxThreads()`、`Handler::handleTimestamp()` の仮想化を実装した (§4.4.2)。
- 決めたこと: S18 (段階 4 から、解析できない行は飛ばして続ける)、S19 (1 スレッドのときも記録してから渡す)。
- テスト: `run_test_rcg_line_parser` (8 項目) を足し、`run_test_rcg_parser` を 25 項目に増やした。CMake の `ctest` 19 個、Autotools の `make check` 15 個と `make distcheck` が通った。ThreadSanitizer でデータ競合の報告はなかった。
- 全体の比較で、変更前と出力が同じだった。ツールの経路は、JSON で 137.9 → 73.6 ms、ULG5 で 119.6 → 59.8 ms、ULG6 で 84.8 → 43.8 ms になった。
- この後: ユーザの確認の後、段階 4 をコミットしてプッシュし、CI を確かめる。その後、段階 5 (libdeflate) に着手する。初めに、CI のどのジョブに `libdeflate-dev` を入れるかを決める (§3.3)。

### 2026-11-25: 段階 4 の CI の結果

- ユーザの確認の後、段階 4 を `dd51505` としてコミットし、プッシュした。
- CI の結果 (`dd51505`): 5 ジョブすべて成功した。GCC 13.3・15.2 とも、コンパイラの警告は 0。4 つのビルドのジョブで `run_test_rcg_line_parser` と `run_test_rcg_parser` が通った (CMake は 19 個すべて、Autotools は `make check` と `make distcheck`)。テストはスレッドの数の上限を 4 にしているので、CI の計算機のコアの数によらず並列の経路を通る。
- この後: 段階 5 (libdeflate) に着手する。初めに、CI のどのジョブに `libdeflate-dev` を入れるかを決める (§3.3)。

### 2026-11-25: 段階 5 を実装した

- ユーザの決定 D8: CI では CMake のジョブにだけ `libdeflate-dev` を入れる。
- `rcsc::gzread_file()` で、gzip を libdeflate で展開するようにした。壊れたファイルは zlib で読み直す。CMake (`ENABLE_LIBDEFLATE`) と Autotools (`--with-libdeflate`) に検出を加えた (§4.5.2)。決めたこと: S20。
- libdeflate のあり・なし、zlib のない場合を、CMake と Autotools で確かめた。全体の比較で、変更前と出力が同じだった。
- ツールの経路は、JSON で 73.9 → 47.3 ms、ULG5 で 60.5 → 36.3 ms、ULG6 で 44.0 → 25.9 ms になった (段階 1 の `stream` の 205.9 / 123.6 / 90.5 ms と比べて、4 分の 1 から 3 分の 1)。
- この後: ユーザの確認の後、段階 5 をコミットしてプッシュし、CI (CMake のジョブで libdeflate が使われること) を確かめる。その後、段階 6 (読み込みの堅牢化) に着手する。初めに、ボールの高さ (`z`、`vz`) を `BallT` に加えるかを決める (§3.3)。

### 2026-11-25: 段階 5 の CI の結果

- ユーザの確認の後、段階 5 を `dfcbed2` としてコミットし、プッシュした。
- CI の結果 (`dfcbed2`): 5 ジョブすべて成功した。コンパイラの警告は 0。
  - CMake のジョブは libdeflate で展開した (Ubuntu 24.04 は libdeflate 1.19、26.04 は 1.23)。構成の出力の `libdeflate:` の行で確かめた。
  - Autotools のジョブは libdeflate を入れず、zlib だけで展開した (configure の出力で確かめた。`distcheck` でも同じ)。
  - libdeflate あり・なしの両方の経路が、GCC 13.3 と 15.2 でビルドされ、テストが通った。
- この後: 段階 6 (読み込みの堅牢化) に着手する。初めに、ボールの高さ (`z`、`vz`) を `BallT` に加えるかを決める (§3.3)。

### 2026-11-25: 段階 6 を実装した

- ユーザの決定 D9: ボールの高さは今回は加えない。`docs/todo.md` に記録した。
- テキストの B1〜B7、JSON の B8〜B11、`rcgversion` の B12 を直した (§4.6.2)。決めたこと: S21 (テキストの知らない記録は 1 度だけ報告して飛ばす)、S22 (ツールは出力の先頭を最初の記録の前に書く)。
- `run_test_rcg_parser` を 35 項目に増やした。段階 5 のライブラリでは 20 項目が失敗し、今のライブラリでは通ることを確かめた。サニタイザでもエラーはなかった。CMake の `ctest` 19 個、Autotools の `make check` 15 個と `make distcheck` が通った。
- 全体の比較とツールの出力の比較で、違いは B8・B9 の修正による想定どおりのものだけだった。
- この後: ユーザの確認の後、段階 6 をコミットしてプッシュし、CI を確かめる。その後、段階 7 (仕上げ: 最終の計測、soccerwindow2 での確認、文書) に着手する。

### 2026-11-25: 段階 6 の CI の結果

- ユーザの確認の後、段階 6 を `9e29dc9` としてコミットし、プッシュした。
- CI の結果 (`9e29dc9`): 5 ジョブすべて成功した。GCC 13.3・15.2 とも、コンパイラの警告は 0。CMake のジョブは 19 個すべて、Autotools のジョブは `make check` と `make distcheck` で 15 個ずつ通った。
- この後: 段階 7 (仕上げ) に着手する。最終の計測 (zlib だけ・libdeflate、CMake・Autotools)、全体の比較の最終の確かめ、soccerwindow2 を新しい librcsc でビルドしてログを開くこと (ユーザの確認)、文書の仕上げを行う。

### 2026-11-25: 段階 7 の計測と確かめ

- 最終の計測を、CMake・Autotools と zlib だけ・libdeflate の 4 つの組み合わせで行った (§4.7.2)。ツールの経路は、段階 1 と比べて JSON で 205.9 → 43.9 ms、ULG5 で 123.6 → 35.8 ms、ULG6 で 90.5 → 26.1 ms (CMake・libdeflate) になった。
- 全体の比較を 7 つのログの 14 の変換で行い、違いは JSON のログの `vq` (B9) だけだった。
- soccerwindow2 を変えずに新しい librcsc でビルドでき、テストが通り、ログを開くパースの時間は JSON で約 235 → 113 ms、テキストで約 140 → 95 ms・118 → 80 ms になった。
- この後: ユーザが soccerwindow2 の画面で、ログの表示と操作を確かめる。その後、段階 7 の記録をコミットしてプッシュし、PR の説明を最終のものにして、下書きを解く。

### 2026-11-25: soccerwindow2 での確認と、遅さの理由

- ユーザが、新しい librcsc でビルドした soccerwindow2 の画面で、3 つのログ (JSON、ULG5、ULG6) の表示と操作を確かめた。問題はなかった。
- ユーザの問い「rcssmonitor は同じ JSON のログを 60 ms 弱で開くが、soccerwindow2 は何が原因で遅いか」を調べた (§4.7.2)。原因は soccerwindow2 の読み込み (`gzifstream` + `parse( istream )`) で、`gzifstream` を通した展開に約 90 ms かかっていた。`gzread_file()` とメモリの経路に替えると、JSON で 115 → 54 ms になることを、試しのビルドで確かめた。soccerwindow2 の切り替えは、`docs/todo.md` の項目として、この PR の後に soccerwindow2 の計画で行う。
- 段階 7 の作業は、これで終わった。
- この後: ユーザの確認の後、段階 7 の記録をコミットしてプッシュし、CI を確かめる。ユーザが PR #123 の説明を最終のものにし、下書きを解く。取り込みの前に、取り込みの記録をこの PR のブランチにコミットする。

### 2026-11-25: develop への取り込み

- ユーザの確認の後、段階 7 の記録を `3e87052` としてコミットし、プッシュした。
- CI の結果 (`3e87052`): 5 ジョブすべて成功した。GCC 13.3・15.2 とも、コンパイラの警告は 0。CMake のジョブは 19 個すべて、Autotools のジョブは `make check` と `make distcheck` で 15 個ずつ通った。
- ユーザが PR #123 の説明を最終のものにし、下書きを解いた。
- PR #123 (`Speed up the rcg parser and update the bundled simdjson to 5.1.1`) を、Squash Merge で develop に取り込む。この記録は、取り込みの前に PR のブランチにコミットした。
- この計画の結果:
  - ツールがログを読む時間は、段階 1 と比べて JSON で 205.9 → 43.9 ms (4.7 倍)、ULG5 で 123.6 → 35.8 ms、ULG6 で 90.5 → 26.1 ms (3.5 倍) になった (CMake の Release、libdeflate あり、8 スレッド)。
  - 同梱の simdjson を 5.1.1 にし、GCC 15 の警告 9 件を解消した。古い JSON パーサを `obsolete/rcg/` に移し、nlohmann/json を削除した。
  - 読み込みの不具合 B1〜B14 を直した。
- この後: soccerwindow2 の読み込みを `rcsc::gzread_file()` とメモリの経路に替える (soccerwindow2 の計画で行う)。ほかの見送った作業は `docs/todo.md` にある。
