# Leonald ソフトウェア開発ディレクトリ

マウス操作制御モジュールとしてのArduino Leonaldの開発

***

## 環境構築

ボードマネージャーからArduino AVR Boardを入れる必要がある。

## 実機書き込み方法
Arduino Leonaldへの書き込みは、**Arduino IDE** を用いて行う。

- [Arduino Leonald概要](https://elchika.com/dic/Arduino_Leonardo/)
- [Arduino IDEのインストール方法](https://www.indoorcorgielec.com/resources/arduinoide%E8%A8%AD%E5%AE%9A/arduino-ide%E3%81%AE%E3%82%A4%E3%83%B3%E3%82%B9%E3%83%88%E3%83%BC%E3%83%AB%E3%81%A8%E8%A8%AD%E5%AE%9A/)

## ディレクトリ構成
```
/
├─ leonald_main
│  └─ leonald_main.ino
├─ leonald_record
│  └─ leonald_record.ino
├─ ML
├─ old
└─ README.md
```

- `ML` は機械学習用データ
- `old` は2024年度の実装を格納

***

## 機械学習
### 参考
以下のサイトを参考にした。
- [Seeed Studio XIAO nRF52840 Sense での TensorFlow Lite 入門](https://wiki.seeedstudio.com/ja/XIAO-BLE-Sense-TFLite-Getting-Started/)
- [Tiny ML on Arduino](https://colab.research.google.com/github/arduino/ArduinoTensorFlowLiteTutorials/blob/master/GestureToEmoji/arduino_tinyml_workshop.ipynb#scrollTo=kGNFa-lX24Qo)

### 実施
Tiny ML on Aruduinoから、一部ソースを変更し、実行した。変更箇所は以下。
- Setup Python Environment
```
# Setup environment
!apt-get -qq install xxd
!pip install pandas numpy matplotlib
!pip install tensorflow==2.0.0-rc1
```
↓↓
```
# Setup environment
!apt-get -qq install xxd
!pip install pandas numpy matplotlib
!pip install tensorflow
```
- Graph Data(optical)
```
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
...
```
↓↓
```
(全削除)
```
- Parse and prepare the data
```
GESTURES = [
    "punch",
    "flex",
]

SAMPLES_PER_GESTURE = 119

...
      # normalize the input data, between 0 to 1:
      # - acceleration is between: -4 to +4
      # - gyroscope is between: -2000 to +2000
      tensor += [
          (df['aX'][index] + 4) / 8,
          (df['aY'][index] + 4) / 8,
          (df['aZ'][index] + 4) / 8,
          (df['gX'][index] + 2000) / 4000,
          (df['gY'][index] + 2000) / 4000,
          (df['gZ'][index] + 2000) / 4000
      ]
```
↓↓
```
GESTURES = [
    "MouseDown",
    "MouseUp",
    "WeelDown",
    "WeelUp",
]

SAMPLES_PER_GESTURE = 174

...
      # normalize the input data, between 0 to 1:
      # - acceleration is between: -4000 to +4000
      # - gyroscope is between: -10000 to +10000
      tensor += [
          (df['Aax'][index] + 4000) / 8000,
          (df['Aay'][index] + 4000) / 8000,
          (df['Aaz'][index] + 4000) / 8000,
          (df['Agx'][index] + 10000) / 20000,
          (df['Agy'][index] + 10000) / 20000,
          (df['Agz'][index] + 10000) / 20000,
          (df['Bax'][index] + 4000) / 8000,
          (df['Bay'][index] + 4000) / 8000,
          (df['Baz'][index] + 4000) / 8000,
          (df['Bgx'][index] + 10000) / 20000,
          (df['Bgy'][index] + 10000) / 20000,
          (df['Bgz'][index] + 10000) / 20000
      ]
```

- Run with Test Data
```
# use the model to predict the test inputs
predictions = model.predict(inputs_test)

# print the predictions and the expected ouputs
print("predictions =\n", np.round(predictions, decimals=3))
print("actual =\n", outputs_test)

# Plot the predictions along with to the test data
plt.clf()
plt.title('Training data predicted vs actual values')
plt.plot(inputs_test, outputs_test, 'b.', label='Actual')
plt.plot(inputs_test, predictions, 'r.', label='Predicted')
plt.show()
```
↓↓
```
(全削除)
```

### モデル評価
学習の際、取得したモデル評価の結果を示す。

- Graph the loss
    lossをグラフ化してモデルの改善が止まるタイミングを確認できる。
    0.5に収束しているように見える？
    ![Leonald_README_ML1](../assets/Leonald_README_ML1.png)

- Graph the loss again, skipping a bit of the start
    上と同じデータ。モデルが収束し始めたあたりの拡大図。
    ![Leonald_README_ML2](../assets/Leonald_README_ML2.png)

- Graph the mean absolute error
    平均絶対誤差。モデルのパフォーマンスを判断するもう一つの指標だそう。
    ![Leonald_README_ML3](../assets/Leonald_README_ML3.png)

> AIによると過学習の傾向だとか、、、