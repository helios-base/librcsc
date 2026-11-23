# PlayerTypeT が出力する v19 の noise rate の値の修正 計画書

- 作成日: 2026-11-24
- ステータス: 実装・手元での確認が完了。コミット待ち
- ベースブランチ: `develop`
- 作業ブランチ: `fix/player-type-json-noise-rate`（ユーザが作成済み。`4a344df` から分岐）
- 関連: `docs/plan/2026-11-24-00_tcp-ws-params.md`（PR #121。その作業中にこの不具合を見つけ、別の PR で直すことにした）

## 1. 背景と目的

`rcsc::rcg::PlayerTypeT` は、rcg のプレイヤータイプ（`player_type`）を表す型である。その出力関数が、v19 で追加された4つのパラメータのうち3つに、正しくない値を書いている。この計画では、その値を正しく出力するように直す。

## 2. 調査結果（2026-11-24）

### 2.1 不具合の内容

`rcsc/rcg/types.cpp` の `PlayerTypeT::toJSON()` と `PlayerTypeT::toServerString()` が、次の4つのパラメータに、すべて `dist_noise_rate_` の値を出力している。

| パラメータ | 出力すべきメンバ | 実際に出力しているメンバ |
|---|---|---|
| `dist_noise_rate` | `dist_noise_rate_` | `dist_noise_rate_`（正しい） |
| `focus_dist_noise_rate` | `focus_dist_noise_rate_` | `dist_noise_rate_` |
| `land_dist_noise_rate` | `land_dist_noise_rate_` | `dist_noise_rate_` |
| `land_focus_dist_noise_rate` | `land_focus_dist_noise_rate_` | `dist_noise_rate_` |

- 当初は `toJSON()` だけの不具合と考えていた（ブランチ名はそのため `json` を含む）が、調べたところ `toServerString()`（テキスト形式の rcg v4〜v6）にも同じ誤りがあった。どちらも #72（`Support v19 protocol`、2024-03-25、`17eda40`）で入った。
- 4つの値を変えた `PlayerTypeT` を出力するプログラムで、テキスト形式・JSON 形式とも、4つすべてが `dist_noise_rate_` の値（0.0125）になることを確かめた。
- 読み込み（`fromServerString()`、JSON の `setDouble()`）はパラメータの表を使っており、正しい。
- `ServerParamT`・`PlayerParamT` の出力関数はパラメータの表から出力するため、同じ種類の誤りは起きない。手で書いた出力関数は `PlayerTypeT` の2つだけである。`PlayerTypeT` のほかのパラメータ（26個）は、名前とメンバが合っていることを確かめた。

### 2.2 影響

- rcssserver が書いた rcg は正しい。librcsc で rcg を**書き出した**ときだけ値が誤る。
  - librcsc のツール: `rcgrenameteam`・`rcgreverse`・`rcgverconv`（rcg を読んで書き直す）。
  - soccerwindow2 の rcg の保存（`PlayerType::convertTo()` で `PlayerTypeT` にしてから出力する）。
- rcssserver 19 の既定値では、`dist_noise_rate` と `focus_dist_noise_rate` は 0.0125、`land_dist_noise_rate` と `land_focus_dist_noise_rate` は 0.00125 である。したがって、書き直した rcg では `land_*` の2つが10倍の値（0.0125）になる。`focus_dist_noise_rate` は、既定値なら同じ値のため結果的に正しい。
- これらの rcg を読んで解析すると、プレイヤータイプの `land_*` のノイズが誤った値になる。

### 2.3 値の丸め

- `toJSON()` は4つを `quantize( ..., 0.000001 )` で丸めて出力し、`toServerString()` は丸めずに出力している（ほかのパラメータは両方とも丸めている）。
- rcssserver は、テキスト形式（`to_sexp`）・JSON 形式とも4つを丸めずに出力している（`src/heteroplayer.cpp`）。
- 丸めの違いは今回の不具合とは関係がないため、変えない。出力するメンバだけを直す。

## 3. 決定事項

| 項目 | 決定 |
|---|---|
| 修正の範囲 | `toJSON()` と `toServerString()` の両方で、出力するメンバを正しくする。丸めは変えない |
| ブランチ名 | `fix/player-type-json-noise-rate` のまま（テキスト形式も直すが、Squash Merge のためブランチ名は履歴に残らない） |
| テスト | `rcsc/rcg/test_types.cpp`（CppUnit、テスト名 `run_test_rcg_types`）を追加し、CMake と Autotools の両方で動かす |
| テストの方式 | 4つの値を明示的に確かめるほか、出力されたすべてのパラメータに別々の値を入れて、各パラメータの出力がその値になることを確かめる（パラメータを追加したときに同じ誤りが起きても検出できるように） |

## 4. 作業内容

- [x] `rcsc/rcg/types.cpp`: `toJSON()`・`toServerString()` の修正。
- [x] テストの追加（`rcsc/rcg/test_types.cpp`、`rcsc/rcg/CMakeLists.txt`、`rcsc/rcg/Makefile.am`）。`docs/build.md` のテストの一覧にも追加する。
- [x] リリースノート（`RELEASE_NOTES.md`）への追記。
- [x] CMake と Autotools でビルドし、テストを通す。修正前のコードでテストが失敗することも確かめる。

## 5. テストの内容

`rcsc/rcg/test_types.cpp` で、`PlayerTypeT` について次を確かめる。

- 4つの noise rate に別々の値を入れて、`toServerString()`・`toJSON()` の出力にそれぞれの値が書かれること。
- `toServerString()` の出力を `fromServerString()` で読み直すと、4つの値が元に戻ること。
- 出力に現れるすべてのパラメータ（`id` を含む）に、名前ごとに別々の値を `setInt()`・`setDouble()` で入れ、`toServerString()`・`toJSON()` の出力で、各パラメータの値が入れた値になること。

手元では `./cmake-helper.sh test`（または `cd build && ctest --output-on-failure -R rcg_types`）で動かせる。Autotools では `./configure --enable-unit-test` の後に `make check`。

## 6. 実装の記録

### 6.1 変更したファイル（2026-11-24）

- `rcsc/rcg/types.cpp`: `PlayerTypeT::toServerString()` と `toJSON()` で、`focus_dist_noise_rate`・`land_dist_noise_rate`・`land_focus_dist_noise_rate` に、それぞれ `focus_dist_noise_rate_`・`land_dist_noise_rate_`・`land_focus_dist_noise_rate_` を出力するようにした。丸めは変えていない。
- `rcsc/rcg/test_types.cpp`（新規）、`rcsc/rcg/CMakeLists.txt`、`rcsc/rcg/Makefile.am`: 単体テスト `run_test_rcg_types`。
  - CMake では、PR #121 で追加した `librcsc_set_unit_test_environment()` を使う。
  - Autotools では `librcsc_rcg.la` だけをリンクする（`PlayerTypeT` は rcg の中だけで完結している）。
  - 出力の解析は正規表現（`<regex>`）で行う。`testPlayerTypeAllParams` は、`toServerString()` の出力からパラメータの名前を取り出し、`id` には `setInt()` で 100 を、ほかには `setDouble()` で 1.125 から 0.125 ずつ増やした値を入れる（2進数で正確に表せ、出力時の丸めでも変わらない値）。テキスト形式・JSON 形式で、出力されるパラメータの数が同じであることも確かめる。
- `docs/build.md`: テストの一覧に `run_test_rcg_types` を加えた。
- `RELEASE_NOTES.md`: 追記した。

### 6.2 確認の結果（2026-11-24、Ubuntu 24.04、GCC 13.3）

- 修正前のコードで、4つの値を変えた `PlayerTypeT` を出力すると、テキスト形式・JSON 形式とも4つすべてが 0.0125 になることを確かめた（2.1）。
- 修正だけを一時的に戻すと、`run_test_rcg_types` が失敗した（`testPlayerTypeNoiseRate` は `Expected: 0.0225`・`Actual: 0.0125`、`testPlayerTypeAllParams` も失敗）。修正を戻し直して通ることを確かめた。
- CMake（`build/`、Debug）: 全16個のテストが通った（警告なし）。
- Autotools（作業ツリーのコピーで `./bootstrap`、`./configure --enable-unit-test`、`make`、`make check`、`make distcheck DISTCHECK_CONFIGURE_FLAGS="--enable-unit-test"`）: すべて通った（`make check` の単体テストは12個）。`run_test_rcg_types` は配布物でも実行された。
