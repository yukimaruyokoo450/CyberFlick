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
const int ClickPin = 7; // GPIO7
unsigned long ledPrevMillis = 0;
const unsigned long ledInterval = 200;
bool ledBlinking = false;
bool ledState = LOW;
bool Power_on = false;
bool Click_toggle = false;
unsigned long lastClickMillis = 0;
unsigned long fsrA_255_start = 0;
bool fsrA_255_flag = false;

void setup() {
  Serial.begin(9600);         // PCとの通信
  espSerial.begin(9600);      // ESP32との通信
  Serial.print("ready");
  pinMode(ledPin, OUTPUT);
  pinMode(PowerPin, OUTPUT);
  pinMode(ClickPin, OUTPUT);
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

    // デバッグ表示
    Serial.print("Power_on: ");
    Serial.print(Power_on);
    Serial.print(", ");
    Serial.print("Click_toggle: ");
    Serial.print(Click_toggle);
    Serial.print(", sensorB.az - sensorA.az: ");
    Serial.print(sensorB.az - sensorA.az);
    Serial.print(", ");
    Serial.print(sensorA.az);
    Serial.print(", ");
    Serial.println(sensorB.az);

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

    if (Click_toggle)
    {
      digitalWrite(ClickPin, HIGH); // クリックON
    }
    else
    {
      digitalWrite(ClickPin, LOW); // クリックOFF
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
      Mouse.move(moveX, moveY);
    }

    // クリック判定（0.2秒間は再判定しない）
    if ((sensorB.az - sensorA.az > click_judge) && (millis() - lastClickMillis >= 200)) {
      Mouse.click();
      Click_toggle = !Click_toggle;
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