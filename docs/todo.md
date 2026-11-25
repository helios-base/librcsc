# 後で対応する作業

計画書で対象外にしたり見送ったりした作業と、後で対応することになった作業を記録する。
各項目には、記録した日付と、元の計画書の節や決定事項の番号を書く。
対応したら、その項目に、対応した計画書や PR を書き添える。

## RCG パーサの高速化 (`docs/plan/2026-11-24-02_rcg-parser-speedup.md`)

- (2026-11-24) soccerwindow2 で rcg のファイルを読む所を、`rcsc::gzread_file()` とメモリの経路 (`Parser::create( data, size )`・`Parser::parse( data, size, handler )`) に替える。soccerwindow2 の別の計画で行う。今の `gzifstream` と `parse( std::istream & )` のままでも解析は速くなるが、`gzifstream` での展開の遅さは残る。(§8)
- (2026-11-24) simdjson のシンボルを `librcsc.so` から公開しないようにする。librcsc を使うプログラムが別の版の simdjson を使うと、ぶつかるおそれがある。(§8)
- (2026-11-24) ライブ受信 (`ParserSimdJSON::parseData()`、`ParserV4::parseLine()`) の速さ。1 パケットずつの解析で、今は問題にならないため見送った。(§8)
- (2026-11-24) `rcsc::gzifstream` 自体の速さ (8192 バイトのバッファなど)。(§8)
- (2026-11-24) v1〜v3 (バイナリ) のパーサの速さ。(§8)
- (2026-11-24) パラメータの既定値を rcssserver に合わせる見直し (rcssmonitor の PR #65 の B-9)。(§8)
- (2026-11-24) `example/` の修正。古いライブラリ名や、ないヘッダ (`rcsc/rcg/factory.h` など) を使っていて、ビルドできない。(§8。`docs/plan/2026-11-21_boost-cmake-maintenance.md` にも記録がある)
- (2026-11-25) simdjson の On-Demand の対象の CPU を選ぶ手段 (CMake・configure のオプション) は、測って差がなかったため設けなかった。JSON の解析がさらに速くなり、SIMD が効く部分の割合が増えたら、測り直す。(S14、§4.2.2)
- (2026-11-25) JSON の show のボールの高さ (`ball.z`) と鉛直方向の速さ (`ball.vz`) を読むこと。rcssmonitor は、計画中の 2.5 次元のシミュレータのために `BallT` に `z_`・`vz_` を足して読んでいるが、今のサーバもログもその値を書かず、キーの名前も仮のため見送った。サーバの形式が決まったら、`rcsc::rcg::BallT` に足し、読み込みと書き出し (`serializer_json` など) を直す。今は知らないキーとして飛ばす。(D9、§4.6)
