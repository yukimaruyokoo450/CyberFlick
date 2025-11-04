#include <Mouse.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(10, 11);  // RX = 10 (ESP32 TX接続)

// MLライブラリを使わない簡易ジェスチャー認識
bool simpleGestureRecognition = true;

int time;
int fsrA;

// ジェスチャークラス名
const char* gestureClasses[] = {
  "MouseDown",
  "MouseUp", 
  "WheelDown",
  "WheelUp"
};

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

// LED関連の変数
const int LED_PIN = LED_BUILTIN;  // 内蔵LED使用
bool ledState = LOW;

// 電源関連の変数
bool Power_on = false;
unsigned long lastClickMillis = 0;
unsigned long fsrA_press_start = 0;
bool fsrA_press_flag = false;
bool longPressFlag = false;
bool powerToggled = false;

// ML推論関連の変数
unsigned long lastGestureTime = 0;
const unsigned long gestureInterval = 200; // 0.2秒間隔でジェスチャー判定

// ホイール制御用の変数
unsigned long lastWheelTime = 0;
const unsigned long wheelInterval = 500; // 0.5秒間隔でホイール操作
int wheelSensitivity = 2000; // ホイール認識の閾値
bool wheelActionExecuted = false;

void setup() {
  Serial.begin(9600);         // PCとの通信
  espSerial.begin(9600);      // ESP32との通信
  Serial.print("ready");
  
  // LED初期化
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // マウス機能の初期化
  Mouse.begin();
  
}

void loop() {
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

    // fsrA制御: 90以上で0.1-1秒→電源切替、1秒以上→マウス移動モード
    if (fsrA >= 50) {
      if (!fsrA_press_flag) {
        fsrA_press_flag = true;
        fsrA_press_start = millis();
        powerToggled = false;
        longPressFlag = false;
      } else {
        unsigned long pressDuration = millis() - fsrA_press_start;
        
        // 0.1秒以上0.5秒以内で電源切替（1回だけ）
        if (pressDuration >= 100 && pressDuration < 500 && !powerToggled) {
          Serial.println("Power Toggle");
          Power_on = !Power_on;
          powerToggled = true;
        }
        // 1秒以上で長押しフラグ立てて、マウス移動モード
        else if (pressDuration >= 1000 && !longPressFlag) {
          Serial.println("Long Press - Mouse Movement Mode");
          longPressFlag = true;
          Power_on = true; // マウス移動モードでは電源ON
        }
      }
    } else {
      fsrA_press_flag = false;
      powerToggled = false;
      longPressFlag = false;
    }

    // マウス移動モード（長押しフラグが立っている時）
    if (longPressFlag && Power_on) {
      int moveX = (sensorA.ay) / 10; // 感度調整
      int moveY = (sensorA.ax) / 10; // 感度調整
      Mouse.move(moveX, moveY);
    }

    performGestureRecognition();
  }
  
  // LED制御: Power_onの間点滅
  if (Power_on) {
    unsigned long currentMillis = millis();
    digitalWrite(LED_PIN, ledState);
  } else {
    // Power_offの時はLED消灯
    digitalWrite(LED_PIN, LOW);
    ledState = LOW;
  }
}

void performGestureRecognition() {
  
  // 最新のセンサデータを使用
  float ax_diff = sensorB.ax - sensorA.ax;
  float ay_diff = sensorB.ay - sensorA.ay;
  float az_diff = sensorB.az - sensorA.az;
  float gx_diff = sensorB.gx - sensorA.gx;
  float gy_diff = sensorB.gy - sensorA.gy;
  float gz_diff = sensorB.gz - sensorA.gz;
  
  // クリック判定（0.2秒間は再判定しない）
  if (Power_on && !longPressFlag && (sensorB.az - sensorA.az > click_judge) && (millis() - lastClickMillis >= 400)) {
    Mouse.click();
    Serial.println("Click!!");
    lastClickMillis = millis();
  }

  // 簡易ジェスチャー判定ロジック
  String recognizedGesture = "None";
  float confidence = 0.0;
  unsigned long currentTime = millis();
  
  // ホイール操作の判定（タイミング制御付き）
  bool canProcessWheel = (currentTime - lastWheelTime >= wheelInterval);
  
  // マウスダウン: Z軸加速度の大きな変化
  if (az_diff > 2000) {
    recognizedGesture = "MouseDown";
    confidence = min(100.0, (az_diff / 2000.0) * 70 + 30);
  }
  // マウスアップ: Z軸加速度の負の変化  
  else if (az_diff < -2000) {
    recognizedGesture = "MouseUp";
    confidence = min(100.0, (abs(az_diff) / 2000.0) * 70 + 30);
  }
  // ホイール:
  else if (canProcessWheel && abs(sensorA.gy) > wheelSensitivity && abs(sensorB.gy) > wheelSensitivity) {
    if(sensorA.gy * sensorB.gy > 0){
      if(sensorA.gy > 0){
        recognizedGesture = "WheelDown";
      }else{
        recognizedGesture = "WheelUp";
      }
      confidence = 100;
      lastWheelTime = currentTime;
    }
  }
  
  // 信頼度が閾値を超えた場合のみジェスチャーを認識
  if (Power_on && confidence > 60.0) {  // 60%以上の信頼度
    Serial.print("Gesture Recognized: ");
    Serial.print(recognizedGesture);
    Serial.print(" (Confidence: ");
    Serial.print(confidence);
    Serial.println("%)");
    
    // ジェスチャーに応じたアクション実行
    if (recognizedGesture == "MouseDown") executeGestureAction(0);
    else if (recognizedGesture == "MouseUp") executeGestureAction(1);
    else if (recognizedGesture == "WheelDown") executeGestureAction(2);
    else if (recognizedGesture == "WheelUp") executeGestureAction(3);
  }
  
  // デバッグ情報
  if(Power_on){
    Serial.print("Debug - ax_diff: "); Serial.print(ax_diff);
    Serial.print(", ay_diff: "); Serial.print(ay_diff);
    Serial.print(", az_diff: "); Serial.print(az_diff);
    Serial.print(", gx_diff: "); Serial.print(gx_diff);
    Serial.print(", gy_diff: "); Serial.print(gy_diff);
    Serial.print(", gz_diff: "); Serial.print(gz_diff);
    
    // ホイール制御のデバッグ情報
    if (abs(gx_diff) > wheelSensitivity/2) {
      Serial.print(" [Wheel Threshold Check: gx_diff=");
      Serial.print(gx_diff);
      Serial.print(", threshold=±");
      Serial.print(wheelSensitivity);
      Serial.print("]");
    }

    Serial.println();
  }
}

void executeGestureAction(int gestureType) {
  // スクロール量の設定（調整可能）
  const int scrollAmount = 2; // スクロール量（1-5推奨）
  
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
      Mouse.move(0, 0, -scrollAmount);
      break;
    case 3:  // WheelUp
      Serial.println("Action: Wheel Up");
      Mouse.move(0, 0, scrollAmount);
      break;
    default:
      Serial.println("Unknown gesture");
      break;
  }
}