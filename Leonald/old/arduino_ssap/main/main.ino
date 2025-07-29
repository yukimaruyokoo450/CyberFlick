#include <Mouse.h>
#include <Wire.h>
#include <SparkFun_KX13X.h>
#include <VL6180X.h>
#include <SoftwareSerial.h>

VL6180X sensor;
SparkFun_KX132 kxAccel;
outputData myData;
const int switchPin = 7;
bool isSwitchOn = false;      // トグル状態を保持する変数
bool lastSwitchState = HIGH;  // 前回のスイッチ状態（初期はHIGH）

const int sens_x = 5;
const int sens_y = 5;
const int threshold_x = 5;
const int threshold_y = 5;

const int max_x = 30;
const int max_y = 50;

//SoftwareSerial bluetooth(10, 11); // RX, TXピンを10, 11に設定

void setup() {
  pinMode(switchPin, INPUT_PULLUP); // 内部プルアップ抵抗を有効にする
  Wire.begin();
  Serial.begin(9600);
  //bluetooth.begin(9600);
  Serial.println("Welcome");
  Mouse.begin();

  if (!kxAccel.begin()) {
      Serial.println("Could not communicate with the the KX13X. Freezing.");
      while (1); // 通信できない場合、停止
  }
  Serial.println("Ready.");

  if (kxAccel.softwareReset())
      Serial.println("Reset.");
  delay(5);
  kxAccel.enableAccel(false);
  kxAccel.setRange(SFE_KX132_RANGE16G);
  kxAccel.enableDataEngine();
  kxAccel.enableAccel();

  sensor.init();
  sensor.configureDefault();
  sensor.setTimeout(500);
}

void loop() {
  // トグルスイッチの状態を更新
  isSwitchOn = updateSwitchState(switchPin);

  // スイッチがONの場合にのみ処理を実行
  if (isSwitchOn) {
      if (kxAccel.dataReady()) {
          kxAccel.getAccelData(&myData);
          // Serial.print(" X: "); Serial.print(myData.xData);
          // Serial.print(" Y: "); Serial.print(myData.yData);
          // Serial.print(" Z: "); Serial.println(myData.zData);
          
          int move_y = (myData.xData - 0.0113343) * 10 * sens_y;
          int move_x = (myData.yData - 0.0277381) * 10 * sens_x;
          
          Serial.print(" move_x: "); Serial.print(move_x);
          Serial.print(" move_y: "); Serial.println(move_y);
          Serial.println(sensor.readRangeSingleMillimeters());

          if(sensor.readRangeSingleMillimeters()>26){
            Mouse.click();
            Serial.println(" Ckicked! ");
            delay(20);
          } else {
            Mouse.move(abs(move_x) > threshold_x ? move_x : 0, abs(move_y) > threshold_y ? move_y : 0);
          }
          
          delay(20);
      }
  } else {
    Serial.println("switch OFF");
  }
  delay(20);
}

// スイッチのトグル動作を実現する関数
bool updateSwitchState(int pin) {
    bool currentSwitchState = digitalRead(pin);

    // スイッチが押された瞬間を検出してトグル状態を反転
    if (currentSwitchState == LOW && lastSwitchState == HIGH) {
        isSwitchOn = !isSwitchOn;
    }
    
    // 現在のスイッチ状態を次回に備えて保存
    lastSwitchState = currentSwitchState;
    
    // 現在のトグル状態を返す
    return isSwitchOn;
}

