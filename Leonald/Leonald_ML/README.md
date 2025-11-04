# CyberFlick ジェスチャー認識システム

このプログラムは、Arduino LeonardoとTensorFlow Lite for Microcontrollersを使用して、CyberFlickシステムでジェスチャー認識を実行します。

## 概要

2つのIMUセンサー（sensorA、sensorB）からのデータを使用して、4種類のジェスチャーを認識します：

1. **MouseDown** - マウスボタンを押下
2. **MouseUp** - マウスボタンを解放  
3. **WheelDown** - マウスホイールを下にスクロール
4. **WheelUp** - マウスホイールを上にスクロール

## 技術仕様

### ハードウェア要件
- Arduino Leonardo (ATmega32u4)
- ESP32 (センサーデータ送信用)
- 2つのIMUセンサー (LSM6DS3)
- FSRセンサー (圧力検知用)

### ソフトウェア要件
- Arduino IDE
- TensorFlow Lite for Microcontrollers ライブラリ
- Mouse ライブラリ（Arduino標準）
- SoftwareSerial ライブラリ（Arduino標準）

### ML モデル仕様
- **入力**: 119サンプル × 12軸データ (ax,ay,az,gx,gy,gz × 2センサー)
- **出力**: 4クラスの確率分布
- **データ正規化**: 
  - 加速度: ±16000 → ±4.0
  - ジャイロ: ±20000 → ±2000 (÷10000)
- **メモリ**: 8KB テンソルアリーナ

## システム動作

### データフロー
1. **センサーデータ受信**: ESP32から115.2kbpsでシリアル通信
2. **データ正規化**: センサー値を機械学習モデルの入力範囲に変換
3. **バッファリング**: 119サンプルのスライディングウィンドウ
4. **推論実行**: TensorFlow Liteモデルでジェスチャー分類
5. **アクション実行**: 認識されたジェスチャーに応じたマウス操作

### データ形式
ESP32からの送信データ形式:
```
time,fsrA,ax_A,ay_A,az_A,gx_A,gy_A,gz_A,ax_B,ay_B,az_B,gx_B,gy_B,gz_B
```

### 判定条件
- **推論頻度**: 1秒間隔
- **信頼度閾値**: 60%以上
- **動作条件**: 電源ON状態かつバッファ準備完了

## 使用方法

### 1. ライブラリインストール
Arduino IDEから以下をインストール：
```
- Arduino_TensorFlowLite (Arduinoライブラリマネージャー)
```

### 2. ファイル配置
```
Leonald_ML/
├── Leonald_ML.ino    // メインプログラム
└── model.h           // TensorFlow Liteモデル
```

### 3. アップロード
1. Arduino IDEでLeonald_ML.inoを開く
2. Arduino Leonardoを選択
3. アップロード実行

### 4. 動作確認
1. シリアルモニタを開く (9600 baud)
2. ESP32からのデータ受信を確認
3. ジェスチャー認識結果の表示を確認

## デバッグ情報

シリアル出力例:
```
ready - ML Gesture Recognition Ready
Power_on: 1, sensorB.az - sensorA.az: 150, 1200, 1350
Gesture Recognized: MouseDown (Confidence: 85.2%)
Action: Mouse Down
```

## カスタマイズ

### 感度調整
```cpp
// 信頼度閾値の変更
if (maxConfidence > 0.6) {  // 0.6 = 60%

// 推論頻度の変更
const unsigned long gestureInterval = 1000; // 1000ms = 1秒
```

### アクションの有効化
マウス操作を実際に実行する場合、コメントアウトを解除：
```cpp
// Mouse.press();   → Mouse.press();
// Mouse.release(); → Mouse.release();
// Mouse.move(0, 0, -1); → Mouse.move(0, 0, -1);
```

## トラブルシューティング

### よくある問題

1. **コンパイルエラー**
   - TensorFlow Liteライブラリがインストールされているか確認
   - model.hファイルが同じディレクトリにあるか確認

2. **推論が動作しない**
   - バッファが準備完了しているか確認（119サンプル収集後）
   - 電源がON状態か確認
   - シリアル通信が正常か確認

3. **認識精度が低い**
   - センサーの校正を確認
   - 正規化パラメータを調整
   - 信頼度閾値を調整

## ファイル構成

```
/CyberFlick2025/CyberFlick/Leonald/
├── Leonald_main/           # 基本システム
│   └── Leonald_main.ino
├── Leonald_ML/            # ジェスチャー認識システム
│   ├── Leonald_ML.ino     # メインプログラム
│   └── model.h            # MLモデル
└── ML/                    # 機械学習関連
    ├── model.h           # 元モデルファイル
    ├── *.csv             # トレーニングデータ
    └── master.xlsx       # データセット
```

## 今後の改善点

1. **リアルタイム性向上**: バッファサイズの最適化
2. **新ジェスチャー追加**: モデルの再トレーニング
3. **省電力化**: 推論頻度の動的調整
4. **エラーハンドリング**: 通信エラー時の復旧機能

---
作成日: 2024年12月
更新日: 2024年12月