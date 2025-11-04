#include <Mouse.h>
#include <SoftwareSerial.h>
#include <TensorFlowLite.h>
#include <tensorflow/lite/micro/all_ops_resolver.h>
#include <tensorflow/lite/micro/micro_error_reporter.h>
#include <tensorflow/lite/micro/micro_interpreter.h>
#include <tensorflow/lite/schema/schema_generated.h>
#include <tensorflow/lite/version.h>

#include "model.h"  // ML model header

SoftwareSerial espSerial(10, 11);  // RX = 10 (ESP32 TX接続)

// TensorFlow Lite関連のグローバル変数
tflite::MicroErrorReporter micro_error_reporter;
tflite::AllOpsResolver resolver;
const tflite::Model* tflite_model = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;
TfLiteTensor* output = nullptr;

// TensorFlow Liteのテンソルアリーナ（メモリプール）
constexpr int kTensorArenaSize = 8 * 1024;  // 8KB
uint8_t tensor_arena[kTensorArenaSize];

// ジェスチャークラス名
const char* gestureClasses[] = {
  "MouseDown",
  "MouseUp", 
  "WheelDown",
  "WheelUp"
};

// センサデータのバッファ（119サンプル）
const int numSamples = 119;
const int numAxes = 12;  // ax,ay,az,gx,gy,gz (A) + ax,ay,az,gx,gy,gz (B)
float sensorBuffer[numSamples][numAxes];
int bufferIndex = 0;
bool bufferReady = false;
int time=0;
int fsrA=0;

////////////////////////

int threshold_x = 200;
int threshold_y = 200;
int click_judge = 300;

////////////////////////

// センサデータ構造体
struct SensorData {
  int16_t ax, ay, az, gx, gy, gz;
};

SensorData sensorA, sensorB;

const int ledPin = 2; // GPIO2
const int PowerPin = 4; // GPIO4

// LED関連の変数
unsigned long ledPrevMillis = 0;
const unsigned long ledInterval = 200;
bool ledBlinking = false;
bool ledState = LOW;

// 電源関連の変数
bool Power_on = false;
unsigned long lastClickMillis = 0;
unsigned long fsrA_255_start = 0;
bool fsrA_255_flag = false;

// ML推論関連の変数
unsigned long lastGestureTime = 0;
const unsigned long gestureInterval = 1000; // 1秒間隔でジェスチャー判定

void setup() {
  Serial.begin(9600);         // PCとの通信
  espSerial.begin(9600);      // ESP32との通信
  Serial.print("ready");
  
  // ピンの初期化
  pinMode(ledPin, OUTPUT);
  pinMode(PowerPin, OUTPUT);
  
  // マウス機能の初期化
  Mouse.begin();
  
  // TensorFlow Liteの初期化
  setupTensorFlow();
  
  // センサバッファの初期化
  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numAxes; j++) {
      sensorBuffer[i][j] = 0.0f;
    }
  }
  
  Serial.println(" - TensorFlow Lite ML Gesture Recognition Ready");
}

void setupTensorFlow() {
  // モデルをロード
  tflite_model = tflite::GetModel(model);
  if (tflite_model->version() != TFLITE_SCHEMA_VERSION) {
    Serial.println("Model schema version not supported!");
    return;
  }

  // インタープリターの作成
  interpreter = new tflite::MicroInterpreter(
      tflite_model, resolver, tensor_arena, kTensorArenaSize, &micro_error_reporter);

  // テンソルの割り当て
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk) {
    Serial.println("AllocateTensors() failed!");
    return;
  }

  // 入力と出力テンソルのポインタを取得
  input = interpreter->input(0);
  output = interpreter->output(0);

  Serial.println("TensorFlow Lite initialized successfully!");
  
  // デバッグ情報
  Serial.print("Input shape: ");
  for (int i = 0; i < input->dims->size; i++) {
    Serial.print(input->dims->data[i]);
    if (i < input->dims->size - 1) Serial.print(" x ");
  }
  Serial.println();
  
  Serial.print("Output shape: ");
  for (int i = 0; i < output->dims->size; i++) {
    Serial.print(output->dims->data[i]);
    if (i < output->dims->size - 1) Serial.print(" x ");
  }
  Serial.println();
}

void loop() {
  delay(5);
  if (espSerial.available()) {
    // 1行読み込み
    // 時間、AのFSR、Aの加速度3軸、Aのジャイロ3軸、Bの加速度3軸、Bのジャイロ3軸
    String line = espSerial.readStringUntil('\n');
    line.trim();  // 先頭と末尾の空白を削除
    int16_t values[14];
    int startIdx = 0;
    int commaIdx;
    for (int i = 0; i < 14; i++) {
      commaIdx = line.indexOf(',', startIdx);
      if (commaIdx == -1) commaIdx = line.length();
      values[i] = line.substring(startIdx, commaIdx).toInt();
      startIdx = commaIdx + 1;
    }

    // 変数に格納
    time = values[0];
    fsrA = values[1];
    int16_t* ptrA = (int16_t*)&sensorA;
    int16_t* ptrB = (int16_t*)&sensorB;
    for (int i = 0; i < 6; i++) {
      ptrA[i] = values[2 + i];
      ptrB[i] = values[8 + i];
    };

    // センサデータをバッファに追加
    addSensorDataToBuffer();

    // sensorA.azが255以上ならLED点滅開始
    if (sensorA.az >= 255 && !ledBlinking) {
      ledBlinking = true;
      ledPrevMillis = millis();
      ledState = HIGH;
      digitalWrite(ledPin, ledState);
    }

    if(Power_on)
    {
      digitalWrite(PowerPin, HIGH); // 電源ON
    }
    else
    {
      digitalWrite(PowerPin, LOW); // 電源OFF
    }

    // 0.5秒間fsrAが130以上だったら電源制御（1回だけ反転）
    static bool powerToggled = false;
    if (fsrA >= 130) {
      if (!fsrA_255_flag) {
        fsrA_255_flag = true;
        fsrA_255_start = millis();
        powerToggled = false;
      } else if (!powerToggled && millis() - fsrA_255_start >= 500) {
        Power_on = !Power_on;
        powerToggled = true; // 1回だけ切り替え
      }
    } else {
      fsrA_255_flag = false;
      powerToggled = false;
    }

    // クリック判定（0.2秒間は再判定しない）
    if ((sensorB.az - sensorA.az > click_judge) && (millis() - lastClickMillis >= 200)) {
    //   Mouse.click();
      Serial.println("Click!!");
      lastClickMillis = millis();
    }

    // ジェスチャー認識（1秒間隔）
    if (bufferReady && Power_on && (millis() - lastGestureTime >= gestureInterval)) {
      performGestureRecognition();
      lastGestureTime = millis();
    }
  }

  // 点滅処理（非ブロッキング）
  if (ledBlinking) {
    if (millis() - ledPrevMillis >= ledInterval) {
      ledPrevMillis = millis();
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      // 1回ON/OFFで点滅終了（必要なら連続点滅に変更可）
      if (!ledState) {
        ledBlinking = false;
      }
    }
  }
}

void addSensorDataToBuffer() {
  // センサデータを正規化してバッファに追加
  // 加速度: ±16000 → ±4.0に正規化
  // ジャイロ: ±20000 → ±2000に正規化（参考実装に合わせて）
  
  sensorBuffer[bufferIndex][0] = (float)sensorA.ax / 4000.0f;  // ax_A
  sensorBuffer[bufferIndex][1] = (float)sensorA.ay / 4000.0f;  // ay_A  
  sensorBuffer[bufferIndex][2] = (float)sensorA.az / 4000.0f;  // az_A
  sensorBuffer[bufferIndex][3] = (float)sensorA.gx / 10000.0f; // gx_A (20000/2000)
  sensorBuffer[bufferIndex][4] = (float)sensorA.gy / 10000.0f; // gy_A
  sensorBuffer[bufferIndex][5] = (float)sensorA.gz / 10000.0f; // gz_A
  
  sensorBuffer[bufferIndex][6] = (float)sensorB.ax / 4000.0f;  // ax_B
  sensorBuffer[bufferIndex][7] = (float)sensorB.ay / 4000.0f;  // ay_B
  sensorBuffer[bufferIndex][8] = (float)sensorB.az / 4000.0f;  // az_B
  sensorBuffer[bufferIndex][9] = (float)sensorB.gx / 10000.0f; // gx_B
  sensorBuffer[bufferIndex][10] = (float)sensorB.gy / 10000.0f; // gy_B
  sensorBuffer[bufferIndex][11] = (float)sensorB.gz / 10000.0f; // gz_B

  bufferIndex++;
  if (bufferIndex >= numSamples) {
    bufferIndex = 0;
    bufferReady = true;
  }
}

void performGestureRecognition() {
  // 入力テンソルにセンサデータをコピー
  for (int i = 0; i < numSamples; i++) {
    for (int j = 0; j < numAxes; j++) {
      int inputIndex = i * numAxes + j;
      input->data.f[inputIndex] = sensorBuffer[i][j];
    }
  }

  // 推論実行
  TfLiteStatus invoke_status = interpreter->Invoke();
  if (invoke_status != kTfLiteOk) {
    Serial.println("Invoke failed!");
    return;
  }

  // 出力結果の解析
  float maxConfidence = 0;
  int predictedClass = 0;
  for (int i = 0; i < 4; i++) {  // 4つのジェスチャークラス
    float confidence = output->data.f[i];
    if (confidence > maxConfidence) {
      maxConfidence = confidence;
      predictedClass = i;
    }
  }

  // 信頼度が閾値を超えた場合のみジェスチャーを認識
  if (maxConfidence > 0.6) {  // 60%以上の信頼度
    Serial.print("Gesture Recognized: ");
    Serial.print(gestureClasses[predictedClass]);
    Serial.print(" (Confidence: ");
    Serial.print(maxConfidence * 100);
    Serial.println("%)");
    
    // ジェスチャーに応じたアクション実行
    executeGestureAction(predictedClass);
  }
}

void executeGestureAction(int gestureType) {
  switch (gestureType) {
    case 0:  // MouseDown
      Serial.println("Action: Mouse Down");
      // Mouse.press();
      break;
    case 1:  // MouseUp  
      Serial.println("Action: Mouse Up");
      // Mouse.release();
      break;
    case 2:  // WheelDown
      Serial.println("Action: Wheel Down");
      // Mouse.move(0, 0, -1);
      break;
    case 3:  // WheelUp
      Serial.println("Action: Wheel Up");
      // Mouse.move(0, 0, 1);
      break;
    default:
      Serial.println("Unknown gesture");
      break;
  }
}