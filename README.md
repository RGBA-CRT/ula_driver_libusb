# ula_driver_libusb
USB Linker Advance’s libusb implementation for AN2131

ULA(USB Linker Advance)は2000年代初頭に開発された、GBAのmultiboot機能を使ってPCとGBAを接続するシステムです。
古いシステムということもあり、USB～GBAの通信に使用されているAN2131のezusbドライバがx64 Windowsでは動作しません。

このソフトウェアはezusbをlibusbに置き換えることで現代の環境でも動作できることを確認するためのテストプログラムです。
EZUSBのロード～GBAへのfwlib転送までをカバーしたライブラリとして設計されています。

Windows x64で動作確認しています。Linuxでも使えるはずですが動作確認はしていません。

+ ULAについてはこちら
	+ http://www2.teamknox.com/teamknox_old/ula/ula.html
	+ http://optimize.ath.cx/bootcable/index.html