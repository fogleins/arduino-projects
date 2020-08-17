#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
const int touchInputPin = 2;
int prevSensorState = LOW; // the sensor is not touched on startup

void setup() {
    pinMode(touchInputPin, INPUT);
    lcd.begin(16, 2);
    lcd.home();
    lcd.print("Starting...");
    lcd.setCursor(0, 1); // second row, first character
    lcd.print("Touch the sensor");
}

void loop() {
    int sensorState = digitalRead(touchInputPin);
    if (prevSensorState == LOW && sensorState == HIGH) {
        lcd.clear();
        lcd.print("Sensor touched");
    }
    if (prevSensorState == HIGH && sensorState == LOW) {
        lcd.clear();
        lcd.print("Touch the sensor");
    }
    prevSensorState = sensorState;
}