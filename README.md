# ula_driver_libusb
USB Linker Advance’s libusb implementation for AN2131

ULA(USB Linker Advance)は2000年代初頭に開発された、GBAのmultiboot機能を使ってPCとGBAを接続するシステムです。
古いシステムということもあり、USB～GBAの通信に使用されているAN2131のezusbドライバがx64 Windowsでは動作しません。

このソフトウェアはezusbをlibusbに置き換えることで現代の環境でも動作できることを確認するためのテストプログラムです。
EZUSBのロード～GBAへのfwlib転送までをカバーしたライブラリとして設計されています。

Windows x64で動作確認しています。Linuxでも使えるはずですが動作確認はしていません。

ULAはいくつか種類があるようですが、AN2131を使ったオリジナルのULAがターゲットです。

+ ULAについてはこちら
	+ http://www2.teamknox.com/teamknox_old/ula/ula.html
	+ http://optimize.ath.cx/bootcable/index.html
	
+ このライブラリを使用しているもの
	+ https://github.com/RGBA-CRT/GBA-Memviewer-ULA	
	
# ドライバのインストール方法
1. ULAを接続します
1. [Zadig](https://zadig.akeo.ie/)を使ってULAにWinUSBドライバを当てます。
	+ Editでデバイス名を決められるので「USB Linker Advance(AN2131)」などとつけておきます。
1. このテストプログラムを使用するなどしてAN2131へファームを転送します
1. [Zadig](https://zadig.akeo.ie/)を使ってULAファーム用のデバイスにWinUSBドライバを当てます。
	+ Editでデバイス名を決められるので「USB Linker Advance (Online)(AN2131)」などとつけておきます。
