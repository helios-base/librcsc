# librcsc Boost 対応の見直し・CMake ビルドの整備 計画書

- 作成日: 2026-11-21
- ステータス: 実施中（Phase 0）
- ベースブランチ: `develop`（librcsc の既定ブランチ。master は年1回程度の更新）
- 作業ブランチ: `feature/boost-cmake-maintenance`（ユーザが GitHub 上で develop から作る）
- 関連: soccerwindow2 の計画書 `docs/plan/2026-11-19_qt6-cmake-migration.md`（soccerwindow2 リポジトリ。PR #94 でマージ済み）

## 1. 背景と目的

soccerwindow2 の Qt6 対応・CMake のメイン化の作業中に、librcsc に次の課題があることが分かった。

- Boost 1.89 で Boost.System のコンパイル済みライブラリが廃止された。librcsc は Boost.System を使っていないのに、ビルド設定でこのライブラリを要求しているため、Boost 1.89 以降の環境ではビルドできない。Ubuntu 26.04（Boost 1.90）では、configure が `Could not find a version of the Boost::System library!` で失敗することを、soccerwindow2 の CI で確認した（2026-11-20）。
- CMake のビルドがメンテナンスされておらず、ビルドに失敗する。さらに、ビルドできたとしても、インストールされたライブラリをそのままでは使えない（「2.2 CMake のビルド」を参照）。

このため soccerwindow2 の CI では、librcsc を Autotools でビルドし、Ubuntu 26.04 をテストの対象から外している。

この計画の目的は次のとおり。

1. Boost 1.89 以降（Ubuntu 26.04 など）で、Autotools と CMake のどちらでもビルドできるようにする。
2. CMake のビルドを、Autotools のビルドと同じものがインストールされる状態に直し、使える状態で保つ。
3. librcsc 自身の CI を用意し、Ubuntu 22.04/24.04/26.04 で両方のビルドとテストを確かめ続けられるようにする。
4. ドキュメントを現状に合わせる。
5. CMake の既定のインストール先を `~/local` から `~/.local` に変える（ユーザの指示。2026-11-21）。soccerwindow2 も同じ変更をしている。

この計画の完了後、soccerwindow2 側で CI に Ubuntu 26.04 を戻し、librcsc のビルドを CMake にすることも検討する（「Phase 8」）。

## 2. 現状の調査結果（2026-11-21、develop の `65e1393`）

調査は Ubuntu 24.04（GCC 13.3、CMake 3.28.3、Boost 1.83）で行った。

### 2.1 Boost の使い方

- ソースで使っている Boost は、すべてヘッダだけのもの。`boost::system` はどこからも使われていない。
  - 公開ヘッダ（インストールされ、利用側のプログラムからも読み込まれる）のうち4つが、Boost のヘッダを読み込んでいる: `boost/scoped_ptr.hpp`（`rcsc/gz/gzfilterstream.h`、`rcsc/net/abstract_socket.h`、`rcsc/net/udp_socket.h`）、`boost/array.hpp`（`rcsc/ann/rbf.h`）。このため、librcsc を使うプログラムも Boost のヘッダが必要になっている。
  - **ただし、これら4つのヘッダの中で Boost は使われていない**（追加の確認）。`gzfilterstream.h` はすでに `std::unique_ptr` を使っており、ほかの3つには Boost の型が出てこない。読み込みを消すだけでよく、クラスの大きさや関数のシグネチャ（ABI）は変わらない。
  - 利用側も、これらの読み込みに頼っていない。soccerwindow2 は `scoped_ptr`・`boost::array` を使っておらず、helios-base は `boost/scoped_ptr.hpp` を使う所（`src/player/planner/actgen_action_chain_length_filter.h`）で自分で読み込んでいる。
  - ソースの中だけ: `boost/property_tree`（JSON・INI の読み込み。`rcsc/formation/formation_data.cpp`、`formation_parser_json.cpp`、`rcsc/param/ini_parser.cpp`）、`boost/spirit/include/classic.hpp`（`rcsc/clang/clang_parser.cpp`）、`boost/algorithm/string.hpp`（`rcsc/formation/formation_static.cpp`）、`boost/random.hpp`（テスト `rcsc/geom/test_qhull_voronoi.cpp`）。
- ビルド設定では Boost.System のライブラリを要求している。
  - Autotools: `configure.ac` の `AX_BOOST_SYSTEM`、`src/Makefile.am` の各ツール（9個）の `$(BOOST_SYSTEM_LIB)`。
  - CMake: `CMakeLists.txt` の `find_package(Boost 1.41.0 COMPONENTS system REQUIRED)`、`src/CMakeLists.txt` の各ツールの `Boost::system` へのリンク。
  - `librcsc.spec.in`: `BuildRequires` と devel パッケージの `Requires` に `boost-devel >= 1.32`（`configure.ac` の最低バージョンは 1.41）。
- ソースに、Boost のバージョンによる分岐（`BOOST_VERSION` など）はない。
- Ubuntu では、`BoostConfig.cmake` と `boost_headers-config.cmake` が `libboostX.Y-dev`（`libboost-dev` から入る）に含まれている（24.04 の `libboost1.83-dev` で確認）。

### 2.2 CMake のビルド

同じ HEAD を CMake と Autotools でそれぞれビルドし、一時ディレクトリにインストールして比べた。

- **ビルドに失敗する:** `CMakeLists.txt` に `check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)` がない。`cmake-config.h.in` には `HAVE_NETINET_IN_H` があるが設定されないため、`<netinet/in.h>` が読み込まれず、`sockaddr_in` が不完全な型、`ntohl` が未宣言になる。この1行を加えると、ビルドは警告0件で通った。
- **インストールされたライブラリを使えない:** CMake のインストールには `rcsc/player/world_state.h` と `rcsc/time/time_stamp.h` が含まれない（#115・#117 で追加されたヘッダが CMake のインストール対象に加えられていない）。`rcsc/player/world_model.h` と `rcsc/time/timer.h` がこれらを読み込むため、CMake でインストールした librcsc を使うプログラムはコンパイルできない（`world_model.h` を読み込むだけでエラーになることを確認した）。
- **ツールの違い:** CMake では `rcgvalidator` がビルドされず、`rcg2csv` はビルドされるがインストールされない。
- **バージョンの不一致:** #114 で CMake の `project(VERSION 2026)` は更新されたが、`configure.ac` の `AC_INIT` は 2024 のまま。pkg-config の `Version` も、CMake 版は 2026、Autotools 版は 2024 になる。ライブラリのバージョン（SOVERSION 20、`librcsc.so.20.0.0`）は一致している。
- **ライブラリのバージョンの履歴:** master の SOVERSION は 19（パッケージのバージョン 2024）。20 は #114 で develop にだけ入っており、master にはまだ出ていない。
- **ライブラリの中身:** 公開しているシンボル（弱シンボルを除く）は一致した。違いは、最適化オプション（CMake の Release は `-O3`、Autotools の既定は `-g -O2`）によるテンプレートの実体化だけだった。
- **既定のインストール先:** CMake は `~/local`（`CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT` のときに設定）、Autotools は `/usr/local`（`AC_PREFIX_DEFAULT` を使っていない）。
- **テスト:** cppunit の単体テスト（`rcsc/clang`、`rcsc/color`、`rcsc/geom`）は Autotools（`--enable-unit-test`）にしかない。
- **古い書き方:**
  - `cmake_minimum_required(VERSION 3.5)`。CMake 3.31 以降は 3.10 未満の指定に警告を出し、CMake 4 は 3.5 未満の互換性を削除している。
  - `CMAKE_INCLUDE_CURRENT_DIR` の古い分岐、`set(CMAKE_CXX_FLAGS "-W -Wall")` によるユーザ指定の上書き。
  - `FindBoost` の使用（CMake 3.30 以降は非推奨。ポリシー CMP0167）。各サブディレクトリでの `${Boost_INCLUDE_DIRS}` の指定。
  - インストール先の一部（`bin`、`lib/pkgconfig`）が `GNUInstallDirs` を使わずに固定されている。
  - CMake のパッケージ設定ファイル（`librcscConfig.cmake`）を提供していない（`install(EXPORT)` がコメントアウトされている）。利用側（soccerwindow2 など）は、独自の `FindLibrcsc.cmake` で探している。

### 2.3 その他

- librcsc には CI がない。
- `README.md` は Autotools の手順だけで、Ubuntu 20.04 と `libboost-all-dev` を前提にしている。インストール先の例は `~/local`。
- この環境にはコンテナのツールがないため、Ubuntu 26.04（Boost 1.90）での動作は手元では確かめられない。CI で確かめる。
- 利用側の librcsc の探し方（既定のインストール先の変更に関係する）:
  - soccerwindow2: CMake（`cmake/FindLibrcsc.cmake`）は `~/.local`、`~/local` の順に探す。Autotools（`configure.ac`）は `$HOME/local`、`$HOME/.local`、`/opt/local`、`/usr/local`、`/usr` の順に探すため、両方にあると古い `~/local` のものが使われる。
  - helios-base: CMake・Autotools とも `~/.local` を `~/local` より先に探す。
- 手元の librcsc は、Autotools で `~/local` にインストールされている（`~/.local` にはない）。

## 3. 決定事項（2026-11-21、ユーザと決定）

| 項目 | 決定 |
|---|---|
| メインのビルドシステム | soccerwindow2 と同じく CMake をメインにし、Autotools も維持する |
| 既定のインストール先 | CMake の既定を `~/local` から `~/.local` に変える（ユーザの指示）。Autotools は GNU の慣例どおり `/usr/local` のままにし、README で `--prefix=$HOME/.local` を案内する（soccerwindow2 と同じ） |
| 公開ヘッダの Boost | 使われていない Boost の読み込みを消し、利用側に Boost を要求しないようにする（「2.1」の追加の確認による）。ABI は変わらないため、SOVERSION は 20 のままにする（20 は master にまだ出ていない）。作業は Phase 1 で行う（もとの Phase 5 を統合） |
| ソース内の Boost | property_tree・spirit などはヘッダだけの利用なので残す。置き換えは別の計画とする |
| パッケージのバージョン | `AC_INIT` を CMake と同じ 2026 に揃える |
| CMake のパッケージ設定ファイル | `librcscConfig.cmake` を提供し、`find_package(librcsc CONFIG)` とインポートターゲット（`librcsc::rcsc`）で使えるようにする。pkg-config（`librcsc.pc`）と `librcsc-config` も残す |
| 最低バージョン | CMake 3.16。Boost は 1.41 から 1.70 に上げる（ユーザの決定）。Boost 1.70 以上は自身の CMake 設定ファイル（`BoostConfig.cmake`）を提供するため、CMake では CMake のバージョンによらず Config モードで探せる（FindBoost とポリシー CMP0167 に関わらなくて済む）。Autotools・`librcsc.spec.in`・README も 1.70 に揃える。対象の Ubuntu 22.04 以降は Boost 1.74 以上なので問題ない。なお RHEL 8 系（Boost 1.66）ではビルドできなくなる |
| `CLAUDE.md` の扱い | soccerwindow2 と同じく `.gitignore` に加え、Git の管理対象外にする |
| 手元でのテストの手段 | soccerwindow2 と同じように使える `cmake-helper.sh` を用意する（configure、build、test、install、uninstall、clean。実行するコマンドを表示する）。手順は README にも書く |

## 4. 実装計画

作業ブランチ（`feature/boost-cmake-maintenance`）はユーザが GitHub 上で develop から作る。各フェーズを1つ以上のコミットにし、段階ごとにユーザが差分を確認してからコミットする。

### Phase 0: 準備

- 本計画書をコミットする（`docs/plan/` は新しいディレクトリ）。
- 「3. 決定事項」を決め、計画書に反映する。
- リリースノートのファイル（soccerwindow2 と同じく `RELEASE_NOTES.md`）を用意する（`CLAUDE.md` の決まりによる）。以後、各フェーズで新機能・主要な変更点を追記する。
- `CLAUDE.md` を `.gitignore` に加える。

### Phase 1: Boost.System への依存の除去（Boost 1.89 以降への対応）と、公開ヘッダからの Boost の除去

- Autotools: `configure.ac` から `AX_BOOST_SYSTEM` を削除し、`src/Makefile.am` の `$(BOOST_SYSTEM_LIB)` を削除する。`m4/ax_boost_system.m4` を削除する。ヘッダだけを使うので `BOOST_LDFLAGS` も不要。`AX_BOOST_BASE` の最低バージョンを 1.70 にする。
- CMake: `find_package(Boost ... COMPONENTS system)` を `find_package(Boost 1.70 CONFIG REQUIRED)` に変えてヘッダだけ（`Boost::headers`）を使い、`src/CMakeLists.txt` の `Boost::system` へのリンクを削除する。Config モードで探すので FindBoost を使わず、CMake 3.30 以降のポリシー CMP0167 の警告も出ない。
- 公開ヘッダ（`rcsc/gz/gzfilterstream.h`、`rcsc/net/abstract_socket.h`、`rcsc/net/udp_socket.h`、`rcsc/ann/rbf.h`）から、使われていない Boost の読み込みを消す（もとの Phase 5）。librcsc の中に、これらの読み込みに頼っているソースがないことをビルドで確かめる。
- `librcsc.spec.in` の Boost の記述を見直す（`BuildRequires` の最低バージョンを 1.70 にする。公開ヘッダが Boost を読み込まなくなるため、devel パッケージの `Requires` から Boost を外す）。
- Boost の最低バージョンを 1.70 に上げたことを、リリースノートに書く。
- これで、Boost 1.89 以降（Ubuntu 26.04）でもビルドできる見込み。確認は Phase 6 の CI で行う（手元の Boost は 1.83）。

### Phase 2: CMake ビルドの不具合の修正

- `check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)` を追加する。
- インストールするヘッダに `rcsc/player/world_state.h` と `rcsc/time/time_stamp.h` を加える。ほかのディレクトリも含め、`Makefile.am` と `CMakeLists.txt` のソース・ヘッダの一覧が揃っていることを確かめる。
- `rcgvalidator` をビルドし、`rcg2csv` と `rcgvalidator` をインストールするようにする。
- `configure.ac` の `AC_INIT` のバージョンを、CMake と同じ 2026 に揃える。
- 以後このような食い違いが起きないよう、Phase 4 で「インストールされたヘッダがそれぞれ単独でコンパイルできるか」の検査を加える。

### Phase 3: CMake ビルドの整備（近代化）

soccerwindow2 の Phase 1 と同じ方針で整える。

- `cmake_minimum_required(VERSION 3.16)` にし、古い分岐を削除する。`CMAKE_CXX_EXTENSIONS OFF`。
- 警告オプションは `target_compile_options` で付け、`CMAKE_CXX_FLAGS` の上書きをやめる。
- インクルードディレクトリと依存ライブラリを、ターゲット単位（`target_include_directories`、`target_link_libraries`）で整理する。
- 既定のインストール先を `~/local` から `~/.local` に変える（「3. 決定事項」）。設定済みのビルドディレクトリにはキャッシュに `~/local` が残るため、移行の方法（古いインストールの削除、ビルドディレクトリの作り直し、または `-DCMAKE_INSTALL_PREFIX=$HOME/.local`）をリリースノートと README に書く。
- インストール先はすべて `GNUInstallDirs` を使う（`bin`、`lib/pkgconfig` の固定をやめる）。
- `install(EXPORT)` で `librcscConfig.cmake` とバージョンファイルを `lib/cmake/librcsc` にインストールし、インポートターゲット `librcsc::rcsc` を使えるようにする（「3. 決定事項」）。パッケージのバージョンが年（2026）なので、バージョンの互換性の指定（`AnyNewerVersion` など）は実装時に決める。
- `uninstall` ターゲットを用意する（soccerwindow2 と同じく `install_manifest.txt` に載っているファイルを削除する）。

### Phase 4: テストの整備

- CMake: cppunit の単体テスト（`rcsc/clang`、`rcsc/color`、`rcsc/geom`）を CTest に登録する（`BUILD_TESTING` で切り替える。cppunit は pkg-config で探す）。
- Autotools: 既存の `make check`（`--enable-unit-test`）を維持する。
- インストールされたヘッダが、それぞれ単独でコンパイルできるかを検査するテストを加える（Phase 2 で見つかったような、ヘッダのインストール漏れを検出するため）。
- テストが通らないものがあれば、原因を調べて直すか、扱いをユーザと決める。
- ユーザが手元でテストを動かせるよう、`cmake-helper.sh` を用意する（「3. 決定事項」。configure、build、test、install、uninstall、clean。実行するコマンドを表示する。install の既定のインストール先は `~/.local`）。実行方法は README にも書く。

### Phase 5: 公開ヘッダからの Boost の除去（Phase 1 に統合）

使われていない読み込みを消すだけでよいと分かったため（「2.1」）、Phase 1 で行う。Phase 3 で `librcscConfig.cmake` を作る前に済ませておくと、Config に Boost への依存を書かずに済む。フェーズの番号は、ほかのフェーズからの参照のため残す。

### Phase 6: GitHub Actions による CI

- `.github/workflows/build.yml` を作る（soccerwindow2 のワークフローを参考にする）。
- 組み合わせ: Ubuntu 22.04・24.04・26.04 × CMake・Autotools（計6ジョブ）。ビルド、テスト、インストールを行う。
- Ubuntu 26.04 で Boost 1.90 でもビルドできることを、ここで確かめる。
- CMake と Autotools でインストールされたファイルの一覧を比べる手順も、CI に入れることを検討する。

### Phase 7: ドキュメントの更新

- `README.md`: 依存要件（Boost 1.70 以上のヘッダのみ、CMake 3.16 以上など）、Ubuntu 22.04/24.04/26.04 向けの apt コマンド（`libboost-all-dev` ではなく必要なものだけ）、CMake と Autotools のビルド手順、テストの実行方法、アンインストールの方法。
- `README.md`: インストール先（CMake の既定は `~/.local`、Autotools は `--prefix=$HOME/.local` を案内）、`~/local` からの移行、`LD_LIBRARY_PATH`・`PATH` の例を `~/.local` にする。
- `RELEASE_NOTES.md` と、本計画書の「実装記録」を更新する。

### Phase 8: soccerwindow2 への反映（soccerwindow2 リポジトリでの作業）

librcsc のこの計画が完了し、develop にマージされた後に行う。

- soccerwindow2 の CI（`.github/workflows/build.yml`）に、Ubuntu 26.04 の4ジョブを戻す。
- CI の librcsc のビルドを CMake にすることを検討する。
- librcsc が提供する `librcscConfig.cmake` を使い、soccerwindow2 の `cmake/FindLibrcsc.cmake` を `find_package(librcsc CONFIG)` に置き換えることを検討する。
- soccerwindow2 の `configure.ac` が `$HOME/local` を `$HOME/.local` より先に探す順序を見直す（librcsc の既定が `~/.local` になり、両方にあると古い方が使われるため）。
- soccerwindow2 の計画書（またはその後継の計画書）に記録する。

## 5. 検証方法

1. **手元（Ubuntu 24.04）でのビルド:** CMake と Autotools の両方でビルドとインストールを行い、警告がないこと、インストールされたファイルの一覧が（libtool の `.la` を除いて）一致すること、公開シンボルが一致することを確かめる。
2. **インストールされたヘッダ:** それぞれ単独でコンパイルできること（Phase 4 のテスト）。Boost のヘッダなしでも読み込めること。
3. **単体テスト:** CMake の `ctest` と Autotools の `make check`（`--enable-unit-test`）が通ること。
4. **既定のインストール先:** CMake で `CMAKE_INSTALL_PREFIX` を指定せずに新しく構成したとき、`~/.local` になること。
5. **Boost 1.89 以降:** Ubuntu 26.04 の CI で、両方のビルドとテストが通ること（手元では確かめられない）。
6. **利用側:** CMake で `~/.local` にインストールした librcsc を使って、soccerwindow2（`./cmake-helper.sh build`、`test`）と helios-base が、librcsc の場所を指定せずにビルドできること。
7. **CI:** 6ジョブがすべて成功すること。

## 6. 実装記録

### Phase 0（2026-11-21）

- 「3. 未決事項」をユーザと決め、「3. 決定事項」にした。ユーザの指示により、CMake の既定のインストール先を `~/.local` に変えることを加えた（目的の5、Phase 3、Phase 7、Phase 8）。
- 公開ヘッダの Boost を追加で調べ、4つのヘッダは Boost を読み込んでいるだけで使っていないことが分かった。読み込みを消すだけでよく ABI も変わらないため、SOVERSION の検討は不要になった。Phase 5 を Phase 1 に統合した。
- 既定のインストール先の変更に関係する、利用側（soccerwindow2・helios-base）の librcsc の探し方を調べ、「2.3」に記録した。
- ユーザの決定により、Boost の最低バージョンを 1.41 から 1.70 に上げることにした。これに合わせ、CMake では `find_package(Boost 1.70 CONFIG REQUIRED)` で探すことにした（Phase 1）。
- `RELEASE_NOTES.md` を作った。
- `CLAUDE.md` を `.gitignore` に加えた。
