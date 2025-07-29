#include <Mouse.h>
#include <Wire.h>
#include <SparkFun_KX13X.h>

SparkFun_KX132 kxAccel;
outputData myData;

void setup() {
    Wire.begin();
    Serial.begin(9600);
    Serial.println("Welcome");
    Mouse.begin();
    delay(100);

    if (!kxAccel.begin()) {
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
    Serial.println("Welcomeaa");
    if (kxAccel.dataReady()) {
        kxAccel.getAccelData(&myData);
        Serial.print("X: "); Serial.print(myData.xData);
        Serial.print(" Y: "); Serial.print(myData.yData);
        Serial.print(" Z: "); Serial.println(myData.zData);
        
        int move_x = (myData.xData - 0.0113343) * 1000;
        int move_y = (myData.yData - 0.0277381) * 1000;
        
        Serial.print("move_x: "); Serial.print(move_x);
        Serial.print(" move_y: "); Serial.println(move_y);
        
        Mouse.move(move_x, move_y);
        delay(20);
    }
    Serial.println("Welcomess");
    delay(20);
}