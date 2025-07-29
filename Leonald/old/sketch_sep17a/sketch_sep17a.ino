#include <Wire.h>
#include <SparkFun_KX13X.h>

SparkFun_KX132 kxAccel;
outputData myData;

void setup() {
  Wire.begin();
  Serial.begin(115200);
  Serial.println("Welcome");

  if (!kxAccel.begin())
  {
    Serial.println("Could not communicate with the the KX13X. Freezing.");
    while (1);
  }
  Serial.println("Ready.");

  if (kxAccel.softwareReset())
    Serial.println("Reset.");
  delay(5);
  kxAccel.enableAccel(false);
  kxAccel.setRange(SFE_KX132_RANGE16G);
  kxAccel.enableDataEngine();
  kxAccel.enableAccel();
}

void loop() {
  if (kxAccel.dataReady())
  {
    kxAccel.getAccelData(&myData);
    Serial.print("X: ");
    Serial.print(myData.xData, 4);
    Serial.print(" Y: ");
    Serial.print(myData.yData, 4);
    Serial.print(" Z: ");
    Serial.print(myData.zData, 4);
    Serial.println();
  }
  delay(20);
}
