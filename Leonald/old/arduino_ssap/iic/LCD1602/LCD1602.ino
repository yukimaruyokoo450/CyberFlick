#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);
float count = 1.0;

void setup() {
  lcd.init(); 
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Ready...");
  delay(3000);
}

void loop(){
  char line1[15] = "Hello, world!";
  char line2[15];
  char floatString[4];    
  dtostrf(count, 3, 1, floatString);
  snprintf(line2, 15, "Count: %s", floatString);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(line1);
  lcd.setCursor(0, 1);
  lcd.print(line2);

  count = count + 0.1;
  delay(100);
}
