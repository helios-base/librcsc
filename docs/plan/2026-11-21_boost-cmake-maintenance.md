# librcsc Boost 対応の見直し・CMake ビルドの整備 計画書

- 作成日: 2026-11-21
- ステータス: 実施中（Phase 2。Phase 0・1・6a は完了）
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
3. librcsc 自身の CI を用意し、Ubuntu 24.04/26.04 で両方のビルドとテストを確かめ続けられるようにする（当初は 22.04 も対象にしていたが、Phase 6a の作業中に外した。「3. 決定事項」）。
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
- Autotools の単体テスト（`--enable-unit-test`）は、今の Ubuntu では使えない（Phase 6a の作業中に確認）。
  - `m4/cppunit.m4` の `AM_PATH_CPPUNIT` は `cppunit-config` で CppUnit を探すが、`cppunit-config` は CppUnit 1.13 で廃止された。Ubuntu 24.04 の `libcppunit-dev`（1.15.1）には `cppunit.pc` しかないため、configure が `*** CppUnit not found! ***` で止まる。
  - テスト自体も、2つがコンパイルできない（`rcsc/geom/test_rundom_convex_hull.cpp` の `std::random_devince` などの打ち間違い、`rcsc/clang/test_clang_parser.cpp` の範囲 for 文の誤り）。どちらも C++11 への書き換え（#18）で入った誤り。ほかのテスト（geom の8個）は通る。
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
| 最低バージョン | CMake 3.16。Boost は 1.41 から 1.70 に上げる（ユーザの決定）。Boost 1.70 以上は自身の CMake 設定ファイル（`BoostConfig.cmake`）を提供するため、CMake では CMake のバージョンによらず Config モードで探せる（FindBoost とポリシー CMP0167 に関わらなくて済む）。Autotools・`librcsc.spec.in`・README も 1.70 に揃える。対象の Ubuntu 24.04 以降は Boost 1.83 以上なので問題ない（22.04 の Boost 1.74 も満たす）。なお RHEL 8 系（Boost 1.66）ではビルドできなくなる |
| `CLAUDE.md` の扱い | soccerwindow2 と同じく `.gitignore` に加え、Git の管理対象外にする |
| 手元でのテストの手段 | soccerwindow2 と同じように使える `cmake-helper.sh` を用意する（configure、build、test、install、uninstall、clean。実行するコマンドを表示する）。手順は README にも書く |
| CI の時期（Phase 1 の後に決定） | テスト・CI がないまま CMake を大きく書き換えることを避けるため、Phase 6 を2つに分ける。最小構成の CI（Phase 6a）を Phase 2 の前に行い、テストを整備した後に CI を拡充する（Phase 6b）。CMake のジョブが通るよう、Phase 2 の `netinet/in.h` の修正を Phase 6a に前倒しする |
| CI の運用（Phase 1 の後に決定） | CI のきっかけは soccerwindow2 と同じ（master・develop への push、PR、手動実行）。作業ブランチの PR を早めに Draft で作り、push のたびに CI を動かす |
| 対象の Ubuntu（Phase 6a の作業中に決定） | CI と README の対象は Ubuntu 24.04・26.04 にする。22.04 は使われなくなったと思われるため外す（ユーザの決定）。最低バージョン（CMake 3.16、Boost 1.70）は変えない |
| 同梱の simdjson（Phase 6a の作業中に決定） | GCC 15 でのエラーは、同梱の 3.8.0 に本家と同じ修正（3行）を当てて直す。新しい版（本家の最新は v5.2.0）への更新は、この計画の後に予定している RCG パーサの高速化の計画で行う（ユーザの決定） |
| `rcsc/action`（Phase 2 の作業中に決定） | 2022年（`374b41e`）から両方のビルドで使われていないので、`obsolete/action` へ移す（`obsolete/` は使わなくなったソースを移す場所。`92c4e99`、#113）。中の `Makefile.am`・`CMakeLists.txt` は、ビルドの設定と誤解されないように削除する |
| 使われていないファイル（Phase 2 の作業中に決定） | `rcsc/action` のほかにも、どちらのビルドにも入っていないファイルがある（「6. 実装記録」の Phase 2）。この計画では触らず、記録だけする。`rcsc/rcg/parser_json.*` は RCG パーサの高速化の計画で扱う |
| `object_table_printer`（Phase 2 の作業中に決定） | CMake でもビルドする（インストールはしない。Autotools と同じ） |
| リリースノートの日付（Phase 1 の後に決定） | `CLAUDE.md` の決まり（2026-11-21 追加）により、各項目の先頭に変更の日付を `(YYYY-MM-DD)` の形で付ける |

## 4. 実装計画

作業ブランチ（`feature/boost-cmake-maintenance`）はユーザが GitHub 上で develop から作る。各フェーズを1つ以上のコミットにし、段階ごとにユーザが差分を確認してからコミットする。

フェーズの順序: Phase 0 → 1 → 6a → 2 → 3 → 4 → 6b → 7 → 8（Phase 6a を Phase 2 の前に行う。「3. 決定事項」。Phase 5 は Phase 1 に統合した）。

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
- これで、Boost 1.89 以降（Ubuntu 26.04）でもビルドできる見込み。確認は Phase 6a の CI で行う（手元の Boost は 1.83）。

### Phase 6a: 最小構成の CI（Phase 2 の前に行う）

- `.github/workflows/build.yml` を作る（soccerwindow2 のワークフローを参考にする）。
- 組み合わせ: Ubuntu 24.04・26.04 × CMake・Autotools（計4ジョブ。22.04 は外した。「3. 決定事項」）。ビルドとインストールを行い、Autotools では単体テスト（`make check`、`--enable-unit-test`）も行う。インストールされたファイルの一覧をログに出す（Phase 6b での比較の準備）。
- きっかけ: master・develop への push、PR、手動実行（soccerwindow2 と同じ）。
- CMake のジョブが通るよう、`check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)` を追加する（Phase 2 から前倒し）。
- Autotools の単体テストを今の Ubuntu で動かせるよう、CppUnit を pkg-config で探すようにする（`PKG_CHECK_MODULES`。`m4/cppunit.m4` を削除）。コンパイルできないテスト2つを直す（「2.3」）。
- Ubuntu 26.04 で Boost 1.90 でもビルドできることを、ここで確かめる（Phase 1 の確認）。
- CI を入れた後、ユーザが作業ブランチの PR を Draft で作る（以後 push のたびに CI が動く）。

### Phase 2: CMake ビルドの不具合の修正

- `check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)` を追加する（Phase 6a で前倒しして実施）。
- インストールするヘッダに `rcsc/player/world_state.h` と `rcsc/time/time_stamp.h` を加える。ほかのディレクトリも含め、`Makefile.am` と `CMakeLists.txt` のソース・ヘッダの一覧が揃っていることを確かめる。
- `rcgvalidator` をビルドし、`rcg2csv` と `rcgvalidator` をインストールするようにする。
- `configure.ac` の `AC_INIT` のバージョンを、CMake と同じ 2026 に揃える。
- （Phase 1 で見つかった問題）CMake の `rcsc` ターゲットが ZLIB にリンクしておらず、CMake でビルドした `librcsc.so` は zlib のシンボルが未解決のまま `libz` を NEEDED に持たない（ツールは自分で ZLIB にリンクしている）。`rcsc` を ZLIB にリンクする。
- （Phase 1 で見つかった問題）`librcsc-config.in` の `--libs` が、今はない分割ライブラリ（`-lrcsc_agent`、`-lrcsc_clang` など）を出力する。`librcsc.pc.in` と同じ `-lrcsc -lz -lm` にする。
- （Phase 1 で見つかった問題）`src/Makefile.am` の `rcgverconv` の所に `rcgversion_CXXFLAGS` がある（名前の誤り）。一覧を揃えるときに直す。
- （Phase 1 で見つかった問題）`rcsc/action` には `Makefile.am` と `CMakeLists.txt` があるが、どちらのビルドからも使われていない（`rcsc/Makefile.am` の `SUBDIRS` にも `rcsc/CMakeLists.txt` の `add_subdirectory` にもない）。`obsolete/action` へ移す（「3. 決定事項」）。
- （Phase 1 で見つかった問題）Autotools でビルドした `librcsc.so` は `libstdc++` を NEEDED に持たない（develop の時点から。利用側は C++ のプログラムなので実害はない）。原因を調べ、扱いを決める。
- （Phase 2 の作業中に見つかった問題）CMake の `rclmtableprinter` が、`tableprinter.cpp` ではなく `resultprinter.cpp`（`rcgresultprinter` のソース）からビルドされている。また、librcsc を使わない `rclmscheduler`・`rclmtableprinter` が librcsc にリンクしている（Autotools ではリンクしていない）。
- （Phase 2 の作業中に決定）開発用のツール `object_table_printer`（Autotools ではビルドだけしてインストールしない）を、CMake でもビルドする（「3. 決定事項」）。
- 以後このような食い違いが起きないよう、Phase 4 で「インストールされたヘッダがそれぞれ単独でコンパイルできるか」の検査を加える。

### Phase 3: CMake ビルドの整備（近代化）

soccerwindow2 の Phase 1 と同じ方針で整える。

- `cmake_minimum_required(VERSION 3.16)` にし、古い分岐を削除する。`CMAKE_CXX_EXTENSIONS OFF`。（CI の CMake 3.31・4.5 で、今の `VERSION 3.5` に非推奨の警告が出ることを確認した）
- 警告オプションは `target_compile_options` で付け、`CMAKE_CXX_FLAGS` の上書きをやめる。
- インクルードディレクトリと依存ライブラリを、ターゲット単位（`target_include_directories`、`target_link_libraries`）で整理する。
- 既定のインストール先を `~/local` から `~/.local` に変える（「3. 決定事項」）。設定済みのビルドディレクトリにはキャッシュに `~/local` が残るため、移行の方法（古いインストールの削除、ビルドディレクトリの作り直し、または `-DCMAKE_INSTALL_PREFIX=$HOME/.local`）をリリースノートと README に書く。
- インストール先はすべて `GNUInstallDirs` を使う（`bin`、`lib/pkgconfig` の固定をやめる）。
- `install(EXPORT)` で `librcscConfig.cmake` とバージョンファイルを `lib/cmake/librcsc` にインストールし、インポートターゲット `librcsc::rcsc` を使えるようにする（「3. 決定事項」）。パッケージのバージョンが年（2026）なので、バージョンの互換性の指定（`AnyNewerVersion` など）は実装時に決める。
- `uninstall` ターゲットを用意する（soccerwindow2 と同じく `install_manifest.txt` に載っているファイルを削除する）。

### Phase 4: テストの整備

- CMake: cppunit の単体テスト（`rcsc/clang`、`rcsc/color`、`rcsc/geom`）を CTest に登録する（`BUILD_TESTING` で切り替える。cppunit は pkg-config で探す）。
- Autotools: 既存の `make check`（`--enable-unit-test`）を維持する（Phase 6a で今の Ubuntu でも動くようにした）。
- インストールされたヘッダが、それぞれ単独でコンパイルできるかを検査するテストを加える（Phase 2 で見つかったような、ヘッダのインストール漏れを検出するため）。
- テストが通らないものがあれば、原因を調べて直すか、扱いをユーザと決める。Phase 1 で試したところ、インストールされる163個のヘッダのうち次の2つが単独でコンパイルできなかった（develop の時点から）。
  - `rcsc/ann/bpn1.h`: `<iterator>` を読み込まずに `std::ostream_iterator` を使っている。
  - `rcsc/geom/polygon_2d.h`: 不完全な型の `Vector2D` を `std::vector` の要素として使っている。
- 手元には cppunit がない（`libcppunit-dev` が入っていない）。手元で単体テストを動かすにはインストールが必要（ユーザが行う）。Phase 6a では、`apt-get download` で取ったパッケージを一時ディレクトリに展開し、`PKG_CONFIG_PATH`・`PKG_CONFIG_SYSROOT_DIR`・`LD_LIBRARY_PATH` で使って確かめた。
- ユーザが手元でテストを動かせるよう、`cmake-helper.sh` を用意する（「3. 決定事項」。configure、build、test、install、uninstall、clean。実行するコマンドを表示する。install の既定のインストール先は `~/.local`）。実行方法は README にも書く。

### Phase 5: 公開ヘッダからの Boost の除去（Phase 1 に統合）

使われていない読み込みを消すだけでよいと分かったため（「2.1」）、Phase 1 で行う。Phase 3 で `librcscConfig.cmake` を作る前に済ませておくと、Config に Boost への依存を書かずに済む。フェーズの番号は、ほかのフェーズからの参照のため残す。

### Phase 6b: CI の拡充（Phase 4 の後に行う）

最小構成の CI（Phase 6a）に、Phase 4 で整備したテストを加える。

- CMake のジョブで、CTest の単体テストと、インストールされたヘッダの単独コンパイルの検査を行う（`cmake-helper.sh` を使うかは実装時に決める）。
- CMake と Autotools でインストールされたファイルの一覧を比べる手順も、CI に入れることを検討する。
- `make distcheck`（ソースの配布物の検査）を入れるかを検討する。

### Phase 7: ドキュメントの更新

- `README.md`: 依存要件（Boost 1.70 以上のヘッダのみ、CMake 3.16 以上など）、Ubuntu 24.04/26.04 向けの apt コマンド（`libboost-all-dev` ではなく必要なものだけ）、CMake と Autotools のビルド手順、テストの実行方法、アンインストールの方法。
- `README.md`: インストール先（CMake の既定は `~/.local`、Autotools は `--prefix=$HOME/.local` を案内）、`~/local` からの移行、`LD_LIBRARY_PATH`・`PATH` の例を `~/.local` にする。
- `README.md`: Git から取ったソースで `./bootstrap` を実行するには pkg-config が必要になったこと（Phase 6a で `PKG_CHECK_MODULES` を使ったため）、単体テストには CppUnit（`libcppunit-dev`）が必要なこと。
- `RELEASE_NOTES.md` と、本計画書の「実装記録」を更新する。

### Phase 8: soccerwindow2 への反映（soccerwindow2 リポジトリでの作業）

librcsc のこの計画が完了し、develop にマージされた後に行う。

- soccerwindow2 の CI（`.github/workflows/build.yml`）に、Ubuntu 26.04 の4ジョブを戻す。librcsc に合わせて、Ubuntu 22.04 を外すかも検討する。
- CI の librcsc のビルドを CMake にすることを検討する。
- librcsc が提供する `librcscConfig.cmake` を使い、soccerwindow2 の `cmake/FindLibrcsc.cmake` を `find_package(librcsc CONFIG)` に置き換えることを検討する。
- soccerwindow2 の `configure.ac` が `$HOME/local` を `$HOME/.local` より先に探す順序を見直す（librcsc の既定が `~/.local` になり、両方にあると古い方が使われるため）。
- soccerwindow2 の計画書（またはその後継の計画書）に記録する。

### この計画の後に行う作業（別の計画）

- RCG パーサの高速化（この計画の次に予定）。同梱の simdjson（3.8.0。GCC 15 のための修正を当ててある）の新しい版への更新も、この計画で行う。GCC 15 で `simdjson.cpp` に出る警告（`-Wtemplate-id-cdtor`、9件）も、更新で解消する見込み。どちらのビルドにも入っていない古い JSON パーサ（`rcsc/rcg/parser_json.*`）の扱いも、この計画で決める。
- どちらのビルドにも入っていないファイル（「6. 実装記録」の Phase 2）の整理（`obsolete/` へ移すか、削除するか）。
- ソース内の Boost（property_tree・spirit など）の置き換え（「3. 決定事項」）。

## 5. 検証方法

1. **手元（Ubuntu 24.04）でのビルド:** CMake と Autotools の両方でビルドとインストールを行い、警告がないこと、インストールされたファイルの一覧が（libtool の `.la` を除いて）一致すること、公開シンボルが一致することを確かめる。
2. **インストールされたヘッダ:** それぞれ単独でコンパイルできること（Phase 4 のテスト）。Boost のヘッダなしでも読み込めること。
3. **単体テスト:** CMake の `ctest` と Autotools の `make check`（`--enable-unit-test`）が通ること。
4. **既定のインストール先:** CMake で `CMAKE_INSTALL_PREFIX` を指定せずに新しく構成したとき、`~/.local` になること。
5. **Boost 1.89 以降:** Ubuntu 26.04 の CI で、両方のビルドとテストが通ること（手元では確かめられない）。
6. **利用側:** CMake で `~/.local` にインストールした librcsc を使って、soccerwindow2（`./cmake-helper.sh build`、`test`）と helios-base が、librcsc の場所を指定せずにビルドできること。
7. **CI:** 4ジョブ（Ubuntu 24.04・26.04 × CMake・Autotools）がすべて成功すること。

## 6. 実装記録

### Phase 0（2026-11-21）

- 「3. 未決事項」をユーザと決め、「3. 決定事項」にした。ユーザの指示により、CMake の既定のインストール先を `~/.local` に変えることを加えた（目的の5、Phase 3、Phase 7、Phase 8）。
- 公開ヘッダの Boost を追加で調べ、4つのヘッダは Boost を読み込んでいるだけで使っていないことが分かった。読み込みを消すだけでよく ABI も変わらないため、SOVERSION の検討は不要になった。Phase 5 を Phase 1 に統合した。
- 既定のインストール先の変更に関係する、利用側（soccerwindow2・helios-base）の librcsc の探し方を調べ、「2.3」に記録した。
- ユーザの決定により、Boost の最低バージョンを 1.41 から 1.70 に上げることにした。これに合わせ、CMake では `find_package(Boost 1.70 CONFIG REQUIRED)` で探すことにした（Phase 1）。
- `RELEASE_NOTES.md` を作った。
- `CLAUDE.md` を `.gitignore` に加えた。

### Phase 1（2026-11-21）

実施内容:

- Autotools: `configure.ac` から `AX_BOOST_SYSTEM` と `BOOST_LDFLAGS` を削除し、`AX_BOOST_BASE` を 1.70.0 にした。`src/Makefile.am` の9個の `$(BOOST_SYSTEM_LIB)` と、`m4/ax_boost_system.m4` を削除した。
- CMake: `find_package(Boost 1.70 CONFIG REQUIRED)` にした。Boost を使う3つのオブジェクトライブラリ（`rcsc_clang`、`rcsc_formation`、`rcsc_param`）だけを `Boost::headers` に PRIVATE でリンクし、ほかのディレクトリ（ビルドされない `rcsc/action` も含む）、`rcsc` ターゲット（PUBLIC）、`src/` の `${Boost_INCLUDE_DIRS}` と、`src/` の `Boost::system` へのリンクを削除した。変数 `Boost_INCLUDE_DIRS` を使わないのは、Boost の古い版の `BoostConfig.cmake` が設定するかを確かめていないため。
- 公開ヘッダ4つから Boost の読み込みを削除した。
- `librcsc.spec.in`: `BuildRequires` を `boost-devel >= 1.70` にし、devel パッケージの `Requires` から Boost を外した。
- `RELEASE_NOTES.md` に Boost の変更を書いた。

計画からの変更点・追加:

- Boost のヘッダ経由で読み込まれていた標準ヘッダに頼っている所が3つあったため、それぞれに読み込みを加えた: `rcsc/gz/gzfilterstream.h`（`std::unique_ptr` のための `<memory>`）、`rcsc/ann/rbf.cpp`（`std::ostream_iterator` のための `<iterator>`）、`rcsc/common/online_client.cpp`（`assert` のための `<cassert>`）。
- 以降のフェーズで扱う問題が見つかったため、Phase 2・Phase 4 に加えた（CMake の `librcsc.so` が zlib にリンクしていない、`librcsc-config` の `--libs` が古い、`src/Makefile.am` の名前の誤り、Autotools の `librcsc.so` に `libstdc++` の NEEDED がない、ビルドされない `rcsc/action`、単独でコンパイルできない公開ヘッダ2つ）。

検証結果（Ubuntu 24.04、GCC 13.3、CMake 3.28.3、Boost 1.83。Git の管理下のファイルを一時ディレクトリに写してビルドした）:

- Autotools: `bootstrap`、`configure`（`checking for boostlib >= 1.70.0 (107000)... yes`）、`make`、`make install` が通った。コンパイラの警告は0件（libtool の convenience library についての警告だけ）。リンクのコマンドに `-lboost` はない。
- CMake: 構成、ビルド、インストールが警告なしで通った。Boost は `/usr/lib/x86_64-linux-gnu/cmake/Boost-1.83.0` の `BoostConfig.cmake` で見つかった。ビルドに失敗する既知の問題（`HAVE_NETINET_IN_H`、Phase 2 で直す）は、`-DHAVE_NETINET_IN_H=1` を与えて避けた。
- 両方のビルドとも、ライブラリとツールの NEEDED に Boost のライブラリはない。インストールされたヘッダに `#include <boost` はない。
- インストールされた163個のヘッダを単独でコンパイルし、161個が通った（失敗した2つは develop の時点からの問題で、Phase 4 で直す）。
- 単体テストは、手元に cppunit がないため動かしていない。Boost 1.89 以降でのビルドと、CMake 3.30 以降で CMP0167 の警告が出ないことは、CI（Phase 6a）で確かめる。

### Phase 6a（2026-11-21）

実施内容:

- `.github/workflows/build.yml` を作った。Ubuntu 24.04・26.04 × CMake・Autotools の4ジョブで（当初は 22.04 も含む6ジョブにしたが、コミット前にユーザの決定で 22.04 を外した）、使われているコンパイラ・CMake・Boost・CppUnit のバージョンを表示し、ビルド、インストール（`$RUNNER_TEMP/install`）、インストールされたファイルの一覧の表示を行う。Autotools では `--enable-unit-test` で構成し、`make check` も行う。失敗したときは、`config.log`、テストのログ、CMake のログを artifact として残す。
- CMake: `check_include_file_cxx("netinet/in.h" HAVE_NETINET_IN_H)` を追加した（Phase 2 から前倒し）。これで `cmake-config.h.in` の `HAVE_*` はすべて設定される。
- Autotools: CppUnit を `PKG_CHECK_MODULES([CPPUNIT], [cppunit >= 1.12.0])` で探すようにし、`m4/cppunit.m4` を削除した。`PKG_PROG_PKG_CONFIG` は条件の外で呼び、`AM_CONDITIONAL([UNIT_TEST], ...)` も条件の外で1回だけ呼ぶ形に整理した。変数名（`CPPUNIT_CFLAGS`、`CPPUNIT_LIBS`）は変わらないので、`Makefile.am` はそのまま。
- コンパイルできなかったテスト2つを直した（`rcsc/geom/test_rundom_convex_hull.cpp` の打ち間違い、`rcsc/clang/test_clang_parser.cpp` の範囲 for 文）。
- `RELEASE_NOTES.md`: 各項目に日付を付け、この作業の変更を書いた。

検証結果（手元、Ubuntu 24.04。Git の管理下のファイルを一時ディレクトリに写してビルドした）:

- Autotools: `--enable-unit-test` で構成し（`checking for cppunit >= 1.12.0... yes`）、`make`、`make check`、`make install` が通った。単体テストは10個すべて通った（geom 9個、clang 1個）。コンパイラの警告は0件。CppUnit は、`apt-get download` で取った `libcppunit-dev`・`libcppunit-1.15-0` を展開して使った。
- Autotools: `--enable-unit-test` なしの既定の構成も通り、`make check` ではテストがビルドされないことを確かめた。
- CMake: `-DHAVE_NETINET_IN_H=1` なしで構成（`netinet/in.h - found`）、ビルド、インストールが警告なしで通った。
- ワークフロー: YAML として読めることと、シェルのコマンド（バージョンの表示、インストールされたファイルの一覧）が手元で動くことを確かめた（`actionlint` は手元にない）。
CI の結果（1回目、`ec19348`、PR #119）:

| ジョブ | 環境 | 結果 |
|---|---|---|
| ubuntu-24.04 / cmake | GCC 13.3.0、CMake 3.31.6、Boost 1.83 | 成功 |
| ubuntu-24.04 / autotools | 同上、CppUnit 1.15.1 | 成功（単体テスト10個すべて通過） |
| ubuntu-26.04 / cmake | GCC 15.2.0、CMake 4.5.0、Boost 1.90 | 失敗（ビルド） |
| ubuntu-26.04 / autotools | 同上、CppUnit 1.15.1 | 失敗（ビルド） |

- Ubuntu 26.04（Boost 1.90）でも、両方のビルドの構成は通った（`checking for boostlib >= 1.70.0 (107000)... yes`）。Boost.System のために構成で止まっていた問題は、Phase 1 で解消した。
- CMake 3.31.6・4.5.0 とも、CMP0167 の警告は出ていない（Config モードで探しているため）。`cmake_minimum_required(VERSION 3.5)` についての非推奨の警告（`Compatibility with CMake < 3.10 will be removed`）は出ている（Phase 3 で直す）。
- `actions/checkout@v4` について、Node.js 20 は非推奨で Node.js 24 で動かしたという警告が出た。
- Ubuntu 26.04 のビルドの失敗は、GCC 15 による次の2つのエラーだった。
  - `rcsc/geom/delaunay_triangulation.cpp`: `<algorithm>` を読み込まずに `std::sort` を使っている（GCC 15 の標準ライブラリでは、ほかのヘッダ経由で読み込まれなくなった）。
  - 同梱の simdjson 3.8.0（`rcsc/rcg/simdjson/simdjson.h`）: `base_formatter` が派生クラスのメンバ関数を `this->print_newline()` などで呼んでいる。GCC 15 は、実体化されていないテンプレートの中のこの誤りもエラーにする（`-Wtemplate-body`）。

1回目の CI の後の修正:

- 同梱の simdjson に、本家と同じ修正（`static_cast<formatter *>(this)->print_newline()` など3行）を当てた。本家の最新版は v5.2.0（2026-11-15）だが、メジャーバージョンが2つ上がる大きな変更なので、この計画では更新しない。更新は、この計画の後に予定している RCG パーサの高速化の計画で行う（ユーザの決定。「3. 決定事項」）。simdjson のヘッダはインストールされない（`noinst_HEADERS`）ので、利用側には影響しない。
- `<algorithm>` の関数（`std::sort`、`std::find_if`、`std::min`・`std::max` など）を使っているのに `<algorithm>` を直接読み込んでいない26個のファイル（公開ヘッダ `rcsc/common/player_type.h` を含む）に、`#include <algorithm>` を加えた。CMake のジョブは最初のエラーで止まり、ほかのファイルがまだコンパイルされていない可能性があるため、エラーが出たファイルだけでなく、まとめて直した。
- ワークフローの `actions/checkout` と `actions/upload-artifact` を v7（Node.js 24 で動く）にした。v5〜v7 の変更点に、このワークフローの使い方に影響するものはない。
- 手元（GCC 13）で、両方のビルド（警告0件）と単体テスト（10個すべて通過）を確かめた。GCC 15 での確認は CI で行う。

CI の結果（2回目、`46d0818`）: 4ジョブすべて成功した。

| ジョブ | 結果 |
|---|---|
| ubuntu-24.04 / cmake | 成功（コンパイラの警告0件） |
| ubuntu-24.04 / autotools | 成功（コンパイラの警告0件。単体テスト10個すべて通過） |
| ubuntu-26.04 / cmake | 成功（警告9件。下記） |
| ubuntu-26.04 / autotools | 成功（警告9件。単体テスト10個すべて通過） |

- Boost 1.90（Ubuntu 26.04）で、両方のビルドとテストが通ることを確かめた（Phase 1 の目的）。
- Node.js 20 についての警告は出なくなった。
- GCC 15 の警告9件は、すべて同梱の simdjson の `simdjson.cpp`（`simd8<bool>()` などのコンストラクタの名前にテンプレート引数がある。`-Wtemplate-id-cdtor`）。librcsc 自身のコードからは出ていない。simdjson の更新（RCG パーサの高速化の計画）で解消する見込みなので、この計画では直さない。
- CMake の `Compatibility with CMake < 3.10` の警告は残っている（Phase 3 で直す）。
- インストールされたファイルの数は、CMake が175個、Autotools が180個。`.la` を除いた違いは `bin/rcg2csv`、`bin/rcgvalidator`、`include/rcsc/player/world_state.h`、`include/rcsc/time/time_stamp.h` の4つで、CMake にないもの。「2.2」で調べた違いと同じで、Phase 2 で直す。

Phase 6a は完了した。

### Phase 2（2026-11-21）

一覧の比較: `Makefile.am` と `CMakeLists.txt` のソース・インストールするヘッダの一覧と、Git の管理下のファイルを、スクリプトで全ディレクトリについて比べた。

- `rcsc/` の下のディレクトリの構成（`SUBDIRS` と `add_subdirectory`）と、ライブラリのソースの一覧は一致していた。インストールするヘッダの違いは、既知の `rcsc/time/time_stamp.h` と `rcsc/player/world_state.h` だけだった。
- ツール（`src/`）では、既知の `rcgvalidator`・`rcg2csv` のほかに、CMake の `rclmtableprinter` のソースの誤りが見つかった（Phase 2 の項目に追加）。
- どちらのビルドにも入っていないファイル（両方のビルドで同じなので、食い違いではない。「3. 決定事項」により記録だけする。最終更新は 2012〜2024年）:
  - `rcsc/rcsc.h`
  - `rcsc/geom/`: `segment_intersection.{h,cpp}`、`test_qhull_delaunay.cpp`、`test_qhull_voronoi.cpp`（qhull を使うテストで、`Makefile.am` ではコメントアウトされている）
  - `rcsc/rcg/`: `parser_json.{h,cpp}`（simdjson のパーサ（#66）より前の JSON パーサ）
  - `rcsc/common/`: `soccer_condition.{h,cpp}`
  - `rcsc/player/`: `action_types.h`、`ball_state.{h,cpp}`、`intercept_simulator_self_v7.{h,cpp}`、`intercept_simulator_self_v13.{h,cpp}`、`player_object_updater.{h,cpp}`、`table_maker.cpp`
  - `rcsc/coach/`: `clang_manager.{h,cpp}`、`cycle_data.{h,cpp}`、`player_change_state.{h,cpp}`、`soccer_adviser.h`
  - `src/`: `rcg3to4.in`、`rcg4to3.in`
  - なお、Phase 6a の GCC 15 への対応（`46d0818`）では、このうち `intercept_simulator_self_v7.cpp`・`intercept_simulator_self_v13.cpp`・`player_object_updater.cpp` にも `#include <algorithm>` を加えた（ビルドされないので不要だったが、害はない）。

実施内容:

- CMake: `rcsc/time/time_stamp.h` と `rcsc/player/world_state.h` をインストールするようにした。
- CMake（`src/CMakeLists.txt`）: `rcgvalidator` をビルドし、`rcg2csv`・`rcgvalidator` をインストールするようにした。`rclmtableprinter` のソースを `tableprinter.cpp` に直し、`rclmscheduler`・`rclmtableprinter` を librcsc にリンクしないようにした（Autotools と同じ）。`object_table_printer` をビルドするようにした（インストールしない）。ツールの ZLIB へのリンクと `${ZLIB_INCLUDE_DIRS}` を削除した（ツールは `zlib.h` を使わない。公開ヘッダの `zlib.h` はコメントの中だけ）。
- CMake: `rcsc` ターゲットを ZLIB に PRIVATE でリンクした（ZLIB が見つかったときだけ。ZLIB は Autotools と同じく必須ではない）。
- Autotools（`rcsc/Makefile.am`）: `nodist_EXTRA_librcsc_la_SOURCES = dummy.cxx` を加え、`librcsc.la` を C++ のリンカでリンクするようにした（便宜ライブラリだけから作るライブラリは、automake が C のリンカ（`gcc`）でリンクするため、`libstdc++` がリンクされていなかった。automake のマニュアルにある方法）。昔の分割ライブラリ（`librcsc_agent`）の、コメントアウトされた記述を削除した（`action` を参照しており、最後の行はコメントの継続（`\`）でコメントになっているだけだった）。
- `configure.ac` の `AC_INIT` を 2026 にした。`CMakeLists.txt`・`configure.ac`・`rcsc/Makefile.am` に、パッケージのバージョンとライブラリのバージョンを一緒に更新するよう注意書きを加えた。
- `librcsc-config.in`: `--libs` の出力を `-lrcsc -lz -lm` にした。
- `src/Makefile.am`: `rcgverconv` の所の `rcgversion_CXXFLAGS` を `rcgverconv_CXXFLAGS` に直し、`rcgversion` にも `rcgversion_CXXFLAGS` を加えた。
- `rcsc/action` の Git の管理下のファイル（114個）を `obsolete/action` へ移し、`Makefile.am`・`CMakeLists.txt` を削除した。手元の `rcsc/action` には、Git の管理外の古い生成物（`.lo`、`Makefile` など）が残る。
- `RELEASE_NOTES.md` に、これらの変更を書いた。

検証結果（手元、Ubuntu 24.04、GCC 13.3、CMake 3.28.3。Git の管理下のファイルを一時ディレクトリに写してビルドした）:

- Autotools: `bootstrap`、`configure --enable-unit-test`、`make`（コンパイラの警告0件）、`make check`（単体テスト10個すべて通過）、`make install` が通った。
- CMake: 構成、ビルド（警告0件）、インストールが通った。`object_table_printer` もビルドされた。
- インストールされたファイルの一覧は、`.la` を除いて完全に一致した（179個）。
- `librcsc.so` と各ツールの NEEDED が両方のビルドで一致した（`librcsc.so` は `libz`・`libstdc++`・`libm`・`libgcc_s`・`libc`。`rclmscheduler`・`rclmtableprinter` は librcsc を使わない）。
- `librcsc-config --version` と `pkg-config --modversion librcsc` は両方とも 2026。`librcsc-config --libs` と `pkg-config --libs librcsc` は両方とも `-L... -lrcsc -lz -lm`。
- 公開シンボル（弱シンボルを除く）は、両方とも2,243個で完全に一致した。
- インストールされたヘッダ（両方とも163個）を単独でコンパイルし、161個が通った（失敗は既知の `bpn1.h`・`polygon_2d.h` で、Phase 4 で直す）。
- CMake でインストールした librcsc を使い、`rcsc/player/world_model.h`・`rcsc/time/time_stamp.h`・`rcsc/version.h` を読み込むプログラムを pkg-config でビルドし、実行できた（「2.2」の問題が解消した）。
- CI の結果: （push 後に追記する）

