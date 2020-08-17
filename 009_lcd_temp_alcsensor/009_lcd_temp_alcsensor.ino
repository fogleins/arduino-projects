/** 
 * This is an extended version of '008_lcd_tempsensor',
 * providing support for the MQ-3 alcohol sensor.
 * The MQ-3 is powered from an external power source.
 */

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <TimedAction.h>

// #define BMP280_I2C_ADDRESS  0x76
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

Adafruit_BMP280 bmp280;
char data[14]; // the data will be read into this array
const int alcsensorDigitalPin = 8;
const int alcsensorAnalogPin = A1; 
const int touchsensorPin = 2;
int touchsensorPrevState = LOW;
int touchsensorState = LOW; // the previous state of the touch sensor


enum State {
    showTemperature,
    showPressure,
    showAltitude,
    showAlcohol
};
State state = showTemperature;

void printTemperature();
void printPressure();
void printAltitude();
void printAlcohol();

TimedAction refreshTemperature = TimedAction(3000, printTemperature);
TimedAction refreshPressure = TimedAction(3000, printPressure);
TimedAction refreshAltitude = TimedAction(3000, printAltitude);
TimedAction refreshAlcohol = TimedAction(3000, printAlcohol);

void setup() {
    lcd.begin(16, 2);
    lcd.print("Hello");
    delay(2000);
    lcd.clear();
    // checks whether the sensor's address is right
    if (!bmp280.begin(0x76)) {  
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        lcd.print("Rossz cim");
        while (1) { }
    }
    pinMode(touchsensorPin, INPUT);
    pinMode(alcsensorDigitalPin, INPUT);
    // TimedActions don't have to be checked yet
    refreshTemperature.disable();
    refreshPressure.disable();
    refreshAltitude.disable();
    lcd.print("Ready.");
    lcd.setCursor(0, 1);
    lcd.print("Touch the sensor");
    // continues when the sensor is touched
    while (digitalRead(touchsensorPin) != HIGH) { }
}

void loop() {
    touchsensorPrevState = touchsensorState;
    touchsensorState = digitalRead(touchsensorPin);
    // if the touchsensor is touched
    if (touchsensorPrevState == LOW && touchsensorState == HIGH) {
        switch (state) {
            case showTemperature:
                printTemperature();
                state = showPressure;
                refreshAlcohol.disable();
                refreshTemperature.enable();
                break;
            case showPressure:
                printPressure();
                state = showAltitude;
                refreshTemperature.disable();
                refreshPressure.enable();
                break;
            case showAltitude:
                printAltitude();
                state = showAlcohol;
                refreshPressure.disable();
                refreshAltitude.enable();
                break;
            case showAlcohol:
                printAlcohol();
                state = showTemperature;
                refreshAltitude.disable();
                refreshAlcohol.enable();
                break;
            default: // control should not reach the default branch
                lcd.clear();
                lcd.print("error");
                break;
        }
    }
    refreshTemperature.check();
    refreshPressure.check();
    refreshAltitude.check();
    refreshAlcohol.check();
}

void printTemperature() {
    float temperature = bmp280.readTemperature();
    sprintf(data, "%d.%02u %cC\0", (unsigned int) temperature, (unsigned int) (temperature * 100) % 100, 223);
    lcd.clear();
    lcd.print("Temperature:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printPressure() {
    float pressure = bmp280.readPressure();
    sprintf(data, "%u.%02u hPa\0", (int) (pressure / 100), (int) ((uint32_t) pressure % 100));
    lcd.clear();
    lcd.print("Pressure:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printAltitude() {
    sprintf(data, "%d m\0", (int) bmp280.readAltitude(/* 1013.25 */));
    lcd.clear();
    lcd.print("Altitude:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printAlcohol() {
    int value = analogRead(alcsensorAnalogPin);
    lcd.clear();
    lcd.print("Alcohol level:");
    // lcd.setCursor(16 - strlen(data), 1);
    lcd.setCursor(12, 1);
    lcd.print(value);
}