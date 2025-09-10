#include <ArduinoBLE.h>
#include <LSM6DS3.h>
#include <Wire.h>

#define DEVICE_ID 0x02  // B側の識別子

LSM6DS3 myIMU(I2C_MODE, 0x6A);

// BLEサービスとキャラクタリスティック
BLEService imuService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic imuCharacteristic(
  "19B10003-E8F2-537E-4F6C-D104768A1214",
  BLERead | BLENotify,
  16  // 1ダミー + IMU(12) + seq(2) + ID(1)
);

uint16_t sequenceNumber = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("BLE init failed!");
    while (1);
  }

  Serial.print("BLE MAC address: ");
  Serial.println(BLE.address());

  if (myIMU.begin() != 0) {
    Serial.println("IMU init failed!");
    while (1);
  }

  BLE.setLocalName("XIAO-IMU-B");
  BLE.setAdvertisedService(imuService);
  imuService.addCharacteristic(imuCharacteristic);
  BLE.addService(imuService);
  BLE.advertise();

  Serial.println("BLE advertising...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      // IMUデータ取得
      int16_t ax = myIMU.readFloatAccelX() * 1000;
      int16_t ay = myIMU.readFloatAccelY() * 1000;
      int16_t az = myIMU.readFloatAccelZ() * 1000;
      int16_t gx = myIMU.readFloatGyroX() * 10;
      int16_t gy = myIMU.readFloatGyroY() * 10;
      int16_t gz = myIMU.readFloatGyroZ() * 10;
      uint16_t seq = sequenceNumber++;

      // BLE送信用ペイロード
      uint8_t payload[16];
      payload[0] = 0x00;  // ダミーバイト
      memcpy(payload + 1, &ax, 2);
      memcpy(payload + 3, &ay, 2);
      memcpy(payload + 5, &az, 2);
      memcpy(payload + 7, &gx, 2);
      memcpy(payload + 9, &gy, 2);
      memcpy(payload + 11, &gz, 2);
      memcpy(payload + 13, &seq, 2);
      payload[15] = DEVICE_ID;

      imuCharacteristic.setValue(payload, 16);

      // デバッグ表示
      Serial.print("ID: "); Serial.print(DEVICE_ID, HEX);
      Serial.print(" SEQ: "); Serial.print(seq);
      Serial.print(" | Acc: "); Serial.print(ax); Serial.print(", "); Serial.print(ay); Serial.print(", "); Serial.print(az);
      Serial.print(" | Gyro: "); Serial.print(gx); Serial.print(", "); Serial.print(gy); Serial.print(", "); Serial.println(gz);

      delay(100); // 10Hz
    }

    Serial.println("Disconnected");
    BLE.advertise();
  }
}
