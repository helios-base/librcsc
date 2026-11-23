# rcssserver 20 の TCP・WebSocket 用パラメータへの対応 計画書

- 作成日: 2026-11-24
- ステータス: 実装・手元での確認が完了。コミット待ち
- ベースブランチ: `develop`
- 作業ブランチ: `feature/tcp-ws-params`（ユーザが作成済み。`897bc90` から分岐）
- 関連: rcssserver の `develop`（`8f1097c`、PR #155・#156 で TCP・WebSocket に対応。NEWS の `[20.0.0]`）

## 1. 背景と目的

rcssserver に TCP と WebSocket による接続が取り込まれ、サーバパラメータが5つ追加された。

この計画では、librcsc がこれらのパラメータを**読めるようにすることだけ**を行う。librcsc の通信（クライアントが TCP・WebSocket で接続すること）の改修は、後日、別の計画で行う。

## 2. 調査結果（2026-11-24）

### 2.1 rcssserver の新しいパラメータ

rcssserver の `src/serverparam.cpp`・`src/serverparam.h`・`NEWS` で確認した。

| パラメータ | 型 | 既定値 | 意味 |
|---|---|---|---|
| `tcp_port` | int | 0 | プレイヤーとモニタの TCP ポート。0 で無効、-1 で `port` と同じ番号 |
| `tcp_coach_port` | int | 0 | オフラインコーチ（トレーナ）の TCP ポート。0 で無効、-1 で `coach_port` と同じ番号 |
| `tcp_olcoach_port` | int | 0 | オンラインコーチの TCP ポート。0 で無効、-1 で `olcoach_port` と同じ番号 |
| `ws_port` | int | 0 | WebSocket のポート。1つのポートで全サービスを扱い、パス（`/player`・`/monitor`・`/coach`・`/olcoach`）で区別する。0 で無効 |
| `ws_allowed_origins` | string | `""` | WebSocket で受け付ける Origin ヘッダの値（カンマ区切り）。空ならすべて受け付ける |

- rcssserver の取得関数は、負の値（-1）のとき UDP のポート番号を返す（`M_tcp_port < 0 ? playerPort() : M_tcp_port`）。
- 5つとも、rcssserver の内部のバージョンは `999` で登録されている。
  - プレイヤー・コーチ・トレーナへの `server_param` メッセージは、クライアントのバージョン以下のパラメータだけを送るため、**これらは送られない**（NEWS の「not sent to the clients」）。
  - **モニタへの送信とゲームログ（rcg）では、クライアントのバージョンを `999` として全パラメータを出力する**（`initsendermonitor.cpp`、`initsenderlogger.cpp`）。したがって、rcssserver 20 の rcg とモニタ接続の `server_param` には、これらが含まれる。
- このほか、rcssserver 20 では `synch_micro_sleep` の既定値が 1 から 0 に変わった。これは新しいパラメータではないため、この計画の対象外とする（「6. 対象外」）。

### 2.2 librcsc で server_param を読む経路

- `rcsc::ServerParam::parse()`: プレイヤー・コーチ・トレーナが、サーバからの `server_param` メッセージを読む。`ServerParam` の `ParamMap`（`createMap()` で登録）を使う。新しいパラメータは送られてこない。
- `rcsc::rcg::ServerParamT`: rcg（テキスト形式 v4〜v6、JSON 形式）の `server_param` を読む。独自のパラメータの表（`ServerParamT::Impl` の `param_map_`）を使い、表にない名前は `Unsupported parameter.` と `server_param` の行全体を標準エラーに出す。読んだ値は `ServerParam::convertFrom( const rcg::ServerParamT & )` で `ServerParam` に移される。soccerwindow2 は、モニタ接続で受け取った `server_param` もこの経路で読む。
- `rcsc::ServerParam::toServerString()`: `ServerParam` の `ParamMap` の全パラメータを出力する。soccerwindow2 が rcg を保存するときに使う（`src/model/view_holder.cpp`）。
- バイナリ形式の rcg（v2・v3）の `server_params_t` は固定の構造体のため、新しいパラメータは扱えない（扱う必要もない）。

### 2.3 作業開始時点の実装（ユーザが追加したもの）

`rcsc/common/server_param.h`・`server_param.cpp` に、次が追加されていた。

- メンバ変数 `M_tcp_player_port`・`M_tcp_trainer_port`・`M_tcp_online_coach_port`・`M_ws_port`（`// 20.0` の下）と `M_ws_allowed_origins`（`// 999` の下）。
- 既定値（`setDefaultParam()`、既存のポートの既定値の後）と `createMap()` への登録（rcssserver と同じ名前）。
- 取得関数 `tcpPlayerPort()`・`tcpTrainerPort()`・`tcpOnlineCoachPort()`・`wsPort()`・`wsAllowedOrigins()`。名前は librcsc の既存の `playerPort()`・`trainerPort()`・`onlineCoachPort()` に合わせている。

この状態で確かめたところ、次の問題があった。

1. **rcg 経由では読めない。** `rcg::ServerParamT` が新しいパラメータを知らないため、rcssserver 20 の `server_param` を読むと、5つのパラメータそれぞれについて `Unsupported parameter.` と行全体が標準エラーに出て、値も `ServerParam` に渡らない（`tcpPlayerPort()` などは既定値の 0 のまま）。
2. **-1 のとき UDP のポート番号にならない。** 取得関数の条件が `< -1` のため、`tcp_port` が -1 のとき `tcpPlayerPort()` が -1 を返す。rcssserver と同じ `< 0` にする必要がある。
3. （小さな点）`M_ws_allowed_origins` だけが `// 999` の下にあり、ほかの4つと分かれている。

なお、`ServerParam::toServerString()` の出力を `rcg::ServerParamT` で読み直すと、新しいパラメータのほかに、以前からある `module_dir` と `replay` も `Unsupported parameter.` になる（`ServerParam` の表にあって `rcg::ServerParamT` の表にない）。これは今回の変更とは関係がないため、この計画では扱わない。

## 3. 決定事項

| 項目 | 決定 |
|---|---|
| 対応の範囲 | パラメータを読めるようにするだけ。通信の改修は後日の別計画 |
| 対象のクラス | `rcsc::ServerParam` と `rcsc::rcg::ServerParamT` の両方（rcg・モニタ接続で受け取る値を `ServerParam` まで届けるため） |
| メンバ・取得関数の名前 | ユーザの実装のまま（librcsc の既存のポートの名前に合わせる）。`rcg::ServerParamT` のメンバは、パラメータ名に合わせて `tcp_port_`・`tcp_coach_port_`・`tcp_olcoach_port_`・`ws_port_`・`ws_allowed_origins_` |
| -1 の扱い | rcssserver と同じく、負の値のとき対応する UDP のポート番号を返す |
| 取得関数の型 | `int`（librcsc の既存のポートの取得関数に合わせる。rcssserver は `std::uint16_t`） |
| メンバの並び | 5つとも `// 20.0` の下にまとめる |
| ABI | `ServerParam`（インライン関数が後ろのメンバを参照する）と `rcg::ServerParamT` の大きさ・配置が変わる。SOVERSION 20 は develop にしかなく、まだ公開していないため、SOVERSION は上げない。develop の librcsc で作ったプログラムは作り直しが必要（リリースノートに書く） |
| テスト | `rcsc/common/test_server_param.cpp`（CppUnit、テスト名 `run_test_server_param`）を追加し、CMake（CTest）と Autotools（`make check`）の両方で動かす。CI は既存の手順のまま実行される |
| Autotools でのテストのリンク | `rcsc/common` は `librcsc.la` より先にビルドされるため、`librcsc_common.la`・`librcsc_rcg.la`・`librcsc_param.la`・`librcsc_geom.la`（便宜ライブラリ）を直接リンクする |
| CMake の単体テストの `LD_LIBRARY_PATH` | 単体テストを、ビルドディレクトリのライブラリを指す `LD_LIBRARY_PATH` で実行する（`librcsc_set_unit_test_environment()`。geom・clang の既存の単体テストにも適用）。「7. 実装の記録」を参照 |

## 4. 作業内容

- [x] `rcsc/common/server_param.h`: メンバを `// 20.0` にまとめる。取得関数の条件を `< 0` に直し、説明のコメントを付ける。
- [x] `rcsc/common/server_param.cpp`: `convertFrom( const rcg::ServerParamT & )` と `convertTo( rcg::ServerParamT & )` に5つを追加する。
- [x] `rcsc/rcg/types.h`・`types.cpp`: `ServerParamT` にメンバ・既定値・パラメータの表への登録を追加する。
- [x] テストの追加（CMake・Autotools）。`docs/build.md` のテストの一覧にも追加する。
- [x] リリースノート（`RELEASE_NOTES.md`）への追記。
- [x] CMake と Autotools でビルドし、テストを通す。

## 5. テストの内容

`rcsc/common/test_server_param.cpp` で、次を確かめる。

- `ServerParam` と `rcg::ServerParamT` の既定値（0 と空文字列）。
- `ServerParam::parse()`（v8 以降の `server_param` メッセージ）で5つを読めること。
- TCP のポートが -1 のとき、対応する UDP のポート番号（`port`・`coach_port`・`olcoach_port`）が返ること。0 のときは 0 が返ること。
- `rcg::ServerParamT::fromServerString()` で5つを読めること、警告が出ないこと（標準エラーを捕まえて確かめる）。引用符付き・空の `ws_allowed_origins` も確かめる。
- `ServerParam::convertFrom()`・`convertTo()`（`rcg::ServerParamT`）で値が受け渡されること。
- `ServerParam::toServerString()` の出力に5つが含まれ、`rcg::ServerParamT` で読み直せること（soccerwindow2 が保存する rcg の読み直しに相当）。

手元では `./cmake-helper.sh test`（または `cd build && ctest --output-on-failure -R server_param`）で動かせる。Autotools では `./configure --enable-unit-test` の後に `make check`。

## 6. 対象外（後日・別計画）

- librcsc のクライアント（プレイヤー・コーチ・トレーナ）、soccerwindow2 のモニタの TCP・WebSocket による接続。
- `synch_micro_sleep` の既定値の変更（rcssserver 20 で 1 → 0）。サーバから `server_param` で送られるため、librcsc の既定値が使われるのは受信前だけである。必要なら別に対応する。
- `module_dir`・`replay` が `rcg::ServerParamT` で `Unsupported parameter.` になる問題（以前からあるもの）。対応は不要で、警告は無視する（2026-11-24、ユーザが決定）。
- `rcg::PlayerTypeT::toJSON()` が、v19 の `focus_dist_noise_rate`・`land_dist_noise_rate`・`land_focus_dist_noise_rate` に `dist_noise_rate_` の値を出力している問題（`rcsc/rcg/types.cpp`。作業中に見つけた、以前からあるもの）。別の PR で直す（2026-11-24、ユーザが決定）。

## 7. 実装の記録

### 7.1 変更したファイル（2026-11-24）

- `rcsc/common/server_param.h`
  - 5つのメンバを `// 20.0` の下にまとめ、説明のコメントを付けた（`M_ws_allowed_origins` を `// 999` から移した）。
  - `tcpPlayerPort()`・`tcpTrainerPort()`・`tcpOnlineCoachPort()` の条件を `< -1` から `< 0` に直した。5つの取得関数に Doxygen のコメントを付けた。
- `rcsc/common/server_param.cpp`: `convertFrom( const rcg::ServerParamT & )`・`convertTo( rcg::ServerParamT & )` に5つを追加した。既定値と `createMap()` はユーザの実装のまま。
- `rcsc/rcg/types.h`・`types.cpp`: `ServerParamT` に `tcp_port_`・`tcp_coach_port_`・`tcp_olcoach_port_`・`ws_port_`・`ws_allowed_origins_` を追加した（既定値は 0 と空文字列、パラメータの表に登録）。表に登録すれば、テキスト形式（`fromServerString()`）、JSON 形式（`setInt()`・`setString()`）、`copyFrom()` のすべてで扱われる。
- `rcsc/common/test_server_param.cpp`（新規）、`rcsc/common/CMakeLists.txt`、`rcsc/common/Makefile.am`: 単体テスト `run_test_server_param`。
- `CMakeLists.txt`、`rcsc/geom/CMakeLists.txt`、`rcsc/clang/CMakeLists.txt`: 単体テストの `LD_LIBRARY_PATH`（7.2）。
- `docs/build.md`: テストの一覧に `run_test_server_param` を加え、単体テストの `LD_LIBRARY_PATH` について書いた。
- `RELEASE_NOTES.md`: 追記した。

### 7.2 単体テストの `LD_LIBRARY_PATH`

手元で CMake のテストを動かしたところ、`run_test_server_param` が `testDefault` でセグメンテーション違反を起こした。原因は、手元の `LD_LIBRARY_PATH` に `~/.local/lib` が含まれていて、テストのプログラムに記録されたビルドディレクトリ（RUNPATH）より先に探されるため、インストール済みの（この変更の前の）`librcsc.so.20` が読み込まれたことだった。新しいヘッダのインライン関数が、古い配置の `ServerParam` を読んでいた（3. の ABI の変更が実際に起きたもの）。

つまり、`LD_LIBRARY_PATH` にインストール済みの librcsc があると、既存の geom・clang の単体テストも、ビルドしたものではなくインストール済みのものを試験していたことになる。CI は `LD_LIBRARY_PATH` を設定していないため、影響はない。

対策として、トップの `CMakeLists.txt` に `librcsc_set_unit_test_environment()` を追加し、すべての単体テストを `LD_LIBRARY_PATH=$<TARGET_FILE_DIR:rcsc>` で実行するようにした。CppUnit などほかのライブラリは、RUNPATH かシステムのディレクトリから見つかる。Autotools の単体テストは、libtool のラッパーや便宜ライブラリの静的リンクを使うため、この問題はない。

### 7.3 確認の結果（2026-11-24、Ubuntu 24.04、GCC 13.3）

- 変更前の状態を再現するプログラムで、次を確かめた。
  - `rcg::ServerParamT` で rcssserver 20 の `server_param` を読むと、5つのパラメータそれぞれについて `Unsupported parameter.` と行全体が出て、`ServerParam` の値は 0 のままだった。
  - `ServerParam::parse()` で `(tcp_port -1)` を読むと、`tcpPlayerPort()` が -1 を返した。
- 変更後は、両方の経路で値が正しく読め、-1 のときは UDP のポート番号が返る。`ServerParam::toServerString()` の出力を `rcg::ServerParamT` で読み直したときの警告は、以前からある `module_dir`・`replay` の2つだけになった。
- テストが不具合を検出できることを確かめるため、条件を一時的に `< -1` に戻すと、`testTcpPortSameAsUdp` と `testConvert` が失敗した（確認後に戻した）。
- CMake（`build/`、Debug）: 全15個のテストが通った（警告なし）。
- Autotools（作業ツリーのコピーで `./bootstrap`、`./configure --enable-unit-test`、`make`、`make check`、`make distcheck DISTCHECK_CONFIGURE_FLAGS="--enable-unit-test"`）: すべて通った。`run_test_server_param` は配布物でも実行された。
- rcssserver が rcg に書く文字列のパラメータの形式（テキスト形式は `(name "value")`、JSON 形式は `"name":"value"`）を rcssserver のソースで確かめ、テストの入力をそれに合わせた。
