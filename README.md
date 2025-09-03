# CyberFlick

## ディレクトリ概要

### 各ディレクトリの運用

それぞれのマイコンのinoファイルは、それぞれのディレクトリの``"(マイコン名)_main"``に格納される。
ライブラリはすべてlibrariesディレクトリに格納される。

```
p/CyberFlick
├─ 3Dmodel
│  └─ CyberFlick_Assy v?.step
├─ assets
│  └─ hoge.png
├─ esp32
│  ├─ esp32_main
│  │  └─ esp32main.ino
│  └─ README.md
├─ Leonald
│  ├─ Leonald_main
│  │  └─ Leonald_main.ino
│  ├─ old
│  ├─ web
│  └─ README.md
├─ libraries
│  └─ (省略)
├─ SeeedStudio
│  ├─ SeeedStudio_A_main
│  │  └─ SeeedStudio_A_main.ino
│  ├─ SeeedStudio_B_main
│  │  └─ SeeedStudio_B_main.ino
│  └─ README.md
├─ .gitatributes
└─ README.md
```

### Arduino IDEの環境設定

各編集者のライブラリ差分をなくすため、全ファイルのライブラリを共通化し、編集効率を上げる。以下はそのためのAruduino IDEの環境設定。

1. ファイル>基本設定を開く

    ![top_README_kankyou1](assets/top_README_kankyou1.png)

2. スケッチブックの場所を``..(yourDocument)/CyberFlick``に設定。下記赤枠部分。
    ![top_README_kankyou2](assets/top_README_kankyou2.png)

3. このように設定することでスケッチブック内に全ファイルが表示される。
    各マイコンのファイルに飛ぶ際は、``"(マイコン名)_main"``をダブルクリック。
    ![top_README_kankyou3](assets/top_README_kankyou3.png)

- (余談)　全てのinoファイルのライブラリは``..(yourDocument)/CyberFlick2025/CyberFlick/libraries``に保存される。コミットの際はこのディレクトリごとコミットすること。

- (余談2) VS Codeでの編集も容易になる。というよりCopilotやgitとの連携のしやすさからVS Codeで編集し、IDEで焼きこみ、というフローにすることを推奨する。
    ![top_README_kankyou5](assets/top_README_kankyou5.png)


> Tips
Arduino IDEを使わずに[VS Codeだけで開発する方法](https://qiita.com/kotek/items/265f6eadfddfd3364e2f)もある

## マイコンの接続

接続図は以下。

![top_README_setuzokuzu1](assets/top_README_setsuzokuzu1.png)