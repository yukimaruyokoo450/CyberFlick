#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>
#include <BLERemoteService.h>
#include <BLERemoteCharacteristic.h>

#define UART_TX_PIN 17  // Leonardoへ送信するTXピン

// UUID設定
static BLEUUID serviceUUID("19B10000-E8F2-537E-4F6C-D104768A1214");
static BLEUUID charUUID("19B10003-E8F2-537E-4F6C-D104768A1214");

// XIAOのMACアドレス（自分の機器のものに置き換えてください）
BLEAddress addressA("0d:27:fb:2d:f1:46");
BLEAddress addressB("dd:1e:83:b0:44:8e");

// BLEクライアント・キャラクタリスティック
BLEClient* clientA = nullptr;
BLEClient* clientB = nullptr;
BLERemoteCharacteristic* charA = nullptr;
BLERemoteCharacteristic* charB = nullptr;

// 受信データ格納構造体
struct SensorData {
  bool ready = false;
  uint8_t fsr;
  int16_t ax, ay, az, gx, gy, gz;
};

SensorData dataA, dataB;

// 通知受信時の処理
void processData(uint8_t* data, size_t len, SensorData& target) {
  if (len != 16) return;

  target.fsr = data[0];
  target.ax  = (int16_t)(data[1]  | (data[2]  << 8));
  target.ay  = (int16_t)(data[3]  | (data[4]  << 8));
  target.az  = (int16_t)(data[5]  | (data[6]  << 8));
  target.gx  = (int16_t)(data[7]  | (data[8]  << 8));
  target.gy  = (int16_t)(data[9]  | (data[10] << 8));
  target.gz  = (int16_t)(data[11] | (data[12] << 8));
  target.ready = true;
}

// A/B両方揃ったらCSV送信
void checkAndSend() {
  if (dataA.ready && dataB.ready) {
    unsigned long t = millis();
    char buf[160];

    snprintf(buf, sizeof(buf),
      "%lu,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
      t, dataA.fsr,
      dataA.ax, dataA.ay, dataA.az, dataA.gx, dataA.gy, dataA.gz,
      dataB.ax, dataB.ay, dataB.az, dataB.gx, dataB.gy, dataB.gz
    );

    Serial2.print(buf);
    Serial.print("CSV: "); Serial.print(buf);

    dataA.ready = false;
    dataB.ready = false;
  }
}

// 通知コールバック
void notifyCB_A(BLERemoteCharacteristic*, uint8_t* data, size_t len, bool) {
  processData(data, len, dataA);
  checkAndSend();
}

void notifyCB_B(BLERemoteCharacteristic*, uint8_t* data, size_t len, bool) {
  processData(data, len, dataB);
  checkAndSend();
}

// XIAOとの接続
bool connectToXIAO(BLEAddress addr, BLEClient*& client, BLERemoteCharacteristic*& characteristic,
                   void (*cb)(BLERemoteCharacteristic*, uint8_t*, size_t, bool)) {
  client = BLEDevice::createClient();
  if (!client->connect(addr)) return false;

  BLERemoteService* svc = client->getService(serviceUUID);
  if (!svc) return false;

  characteristic = svc->getCharacteristic(charUUID);
  if (!characteristic || !characteristic->canNotify()) return false;

  characteristic->registerForNotify(cb);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600, SERIAL_8N1, -1, UART_TX_PIN);  // TX専用 UART

  BLEDevice::init("ESP32_MultiCentral");

  bool okA = connectToXIAO(addressA, clientA, charA, notifyCB_A);
  bool okB = connectToXIAO(addressB, clientB, charB, notifyCB_B);

  if (!okA || !okB) {
    Serial.println("❌ 接続失敗");
  } else {
    Serial.println("✅ 両方接続完了");
    Serial2.println("t,fsr,axA,ayA,azA,gxA,gyA,gzA,axB,ayB,azB,gxB,gyB,gzB");
  }
}

void loop() {
  delay(5);
}
