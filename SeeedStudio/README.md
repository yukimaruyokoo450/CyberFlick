# Seeed Studio XIAO nRF52840

## 環境構築

基本的には下記の参考サイト①を見ながら環境構築を進めること。
ただし、下記の点に注意する
- ボードのインストールは「Seeed nRF52 Boards」ではなく、「Seeed nRF52 mbed-enabled Boards」をインストールする
- ライブラリは以下をインストールする
    - ArduinoBLE
    （参考サイト②にあるようにSeeed nRF52 Boardsでは入れられない）
    - Seeed_Arduino_LSM6DS3
- 上記の「Seeed_Arduino_LSM6DS3」ライブラリ内のLSM6DS3.cppの108行目``SPI.setBitOrder(MSBFIRST);``をコメントアウトすること。（コミットしているこのライブラリはコメントアウト済みなのでそれを使うのがよい）

> 参考サイト
>- ①[Seeed Studio XIAO nRF52840 シリーズの始め方](https://wiki.seeedstudio.com/ja/XIAO_BLE/)
>- ②[Seeed XIAO BLE nRF52840を試す](https://lipoyang.hatenablog.com/entry/2022/09/18/163140)


## 書き込み時の注意

圧力センサ(FSR)がある側がA。