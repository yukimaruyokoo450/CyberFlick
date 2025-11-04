#include <Mouse.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(10, 11);  // RX = 10 (ESP32 TX接続)
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
const int switchPin = 3; // スイッチ1用ピン
const int switch2Pin = 5; // スイッチ2用ピン

// スイッチ関連の変数
bool switchState = false;
bool lastSwitchState = false;
bool switch2State = false;
bool lastSwitch2State = false;

unsigned long lastDebounceTime = 0;
unsigned long lastDebounceTime2 = 0;
const unsigned long debounceDelay = 50; // デバウンス時間（ms）

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

// recordingフラグ
bool recordingOn = false;

void setup() {
  Serial.begin(9600);         // PCとの通信
  espSerial.begin(9600);      // ESP32との通信
  Serial.print("ready");
  
  // ピンの初期化
  pinMode(ledPin, OUTPUT);
  pinMode(PowerPin, OUTPUT);
  pinMode(switchPin, INPUT_PULLUP); // 内部プルアップ抵抗を使用
  pinMode(switch2Pin, INPUT_PULLUP); // 内部プルアップ抵抗を使用
  
  // マウス機能の初期化
  Mouse.begin();
}

void loop() {
  delay(5);
  
  // スイッチの状態を読み取り（デバウンス処理付き）
  readSwitches();
  
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

    // 
    if(recordingOn) {
      Serial.print(sensorA.ax);
      Serial.print(",");
      Serial.print(sensorA.ay);
      Serial.print(",");
      Serial.print(sensorA.az);
      Serial.print(",");
      Serial.print(sensorA.gx);
      Serial.print(",");
      Serial.print(sensorA.gy);
      Serial.print(",");
      Serial.print(sensorA.gz);
      Serial.print(",");
      Serial.print(sensorB.ax);
      Serial.print(",");
      Serial.print(sensorB.ay);
      Serial.print(",");
      Serial.print(sensorB.az);
      Serial.print(",");
      Serial.print(sensorB.gx);
      Serial.print(",");
      Serial.print(sensorB.gy);
      Serial.print(",");
      Serial.println(sensorB.gz);
    }

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

    // マウス移動（感度調整）
    if (Power_on && (abs(sensorA.ax) >= threshold_y || abs(sensorA.ay) >= threshold_x)) {
      int moveX = (sensorA.ay - threshold_x) / 10; // 感度調整
      int moveY = -(sensorA.ax - threshold_y) / 10; // 感度調整
    //   Mouse.move(moveX, moveY);
    }

    // クリック判定（0.2秒間は再判定しない）
    if (Power_on && (sensorB.az - sensorA.az > click_judge) && (millis() - lastClickMillis >= 200)) {
    //   Mouse.click();
      // Serial.println("Click!!");
      lastClickMillis = millis();
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

// スイッチの状態を読み取る関数（デバウンス処理付き）
void readSwitches() {
  // スイッチ1の処理
  bool reading = digitalRead(switchPin);
  if (reading != lastSwitchState) {
    lastDebounceTime = millis();
  }
  
  if ((millis() - lastDebounceTime) > debounceDelay) {
    if (reading != switchState) {
      switchState = reading;
      
      // スイッチが押された時（LOW → HIGH、プルアップなのでLOWが押下状態）
      if (switchState == LOW) {
        // スイッチ1の機能：マウス機能のON/OFF切り替え
        recordingOn = !recordingOn;
        Serial.print("Mouse function: ");
        Serial.println(recordingOn ? "ON" : "OFF");
      }
    }
  }
  lastSwitchState = reading;
  
  // スイッチ2の処理
  bool reading2 = digitalRead(switch2Pin);
  if (reading2 != lastSwitch2State) {
    lastDebounceTime2 = millis();
  }
  
  if ((millis() - lastDebounceTime2) > debounceDelay) {
    if (reading2 != switch2State) {
      switch2State = reading2;
      
      // スイッチ2が押された時
      if (switch2State == LOW) {
        // スイッチ2の機能：強制的に電源ON/OFF切り替え
        Power_on = !Power_on;
        Serial.print("Power forced: ");
        Serial.println("now!");
      }
    }
  }
  lastSwitch2State = reading2;
}