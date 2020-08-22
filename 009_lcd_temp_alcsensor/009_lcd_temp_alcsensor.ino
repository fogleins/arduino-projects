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
    showAlcohol,
    showBAC
};
State state = showTemperature;

void printTemperature();
void printPressure();
void printAltitude();
void printAlcohol();
void printBAC(); // blood alcohol content

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max);

// TimedAction objects are used to refresh the value printed on the LCD screen
TimedAction refreshTemperature = TimedAction(3000, printTemperature);
TimedAction refreshPressure = TimedAction(3000, printPressure);
TimedAction refreshAltitude = TimedAction(3000, printAltitude);
TimedAction refreshAlcohol = TimedAction(3000, printAlcohol);
TimedAction refreshBAC = TimedAction(3000, printBAC);

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
    refreshAlcohol.disable();
    refreshBAC.disable();
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
                refreshBAC.disable();
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
                state = showBAC;
                refreshAltitude.disable();
                refreshAlcohol.enable();
                break;
            case showBAC:
                printBAC();
                state = showTemperature;
                refreshAlcohol.disable();
                refreshBAC.enable();
                break;
            default: // control should not reach the default branch
                lcd.clear();
                lcd.print("error");
                break;
        }
    }
    // Checking whether the values need to be refreshed
    refreshTemperature.check();
    refreshPressure.check();
    refreshAltitude.check();
    refreshAlcohol.check();
    refreshBAC.check();
}

void printTemperature() {
    /**
     * @brief Prints the temperature onto the LCD screen
     */
    float temperature = bmp280.readTemperature();
    sprintf(data, "%d.%02u %cC\0", (unsigned int) temperature, (unsigned int) (temperature * 100) % 100, 223);
    lcd.clear();
    lcd.print("Temperature:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printPressure() {
    /**
     * @brief Prints the pressure onto the LCD screen.
     */
    float pressure = bmp280.readPressure();
    sprintf(data, "%u.%02u hPa\0", (int) (pressure / 100), (int) ((uint32_t) pressure % 100));
    lcd.clear();
    lcd.print("Pressure:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printAltitude() {
    /**
     * @brief Calculates the altitude based on pressure.
     * You might need to change the readAltitude function's parameter
     * based on your geographic location.
     */
    sprintf(data, "%d m\0", (int) bmp280.readAltitude(/* 1013.25 */));
    lcd.clear();
    lcd.print("Altitude:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

void printAlcohol() {
    /**
     * @brief Reads the voltage from an analog input pin and prints
     * the value onto the LCD screen.
     */
    int value = analogRead(alcsensorAnalogPin);
    lcd.clear();
    lcd.print("Alcohol level:");
    // lcd.setCursor(16 - strlen(data), 1);
    lcd.setCursor(12, 1);
    lcd.print(value);
}

void printBAC() {
    /**
     * @brief Calculates and prints the approximate blood alcohol content
     * The conversion is not accurate, it's reliability heavily depends on
     * the environment, such as temperature and humidity. Therefore this
     * calculation can't be trusted to accurately determine one's BAC.
     * DO NOT use this sensor or calculation to determine one's ability to drive.
     */
    int value = analogRead(alcsensorAnalogPin);
    if (value < 200)
        sprintf(data, "0.00%%\0");
    else if (value > 1023)
        sprintf(data, "max value: >0.2\0");
    else {
        // float bac = mapFloat(value, 750, 1023, 0.01, 0.08);
        float bac = mapFloat(value, (float) 200, (float) 1023, (float) 0.01, (float) 0.2);
        sprintf(data, "%u.%02u%%\0", (unsigned int) bac, (unsigned int) (bac * 100) % 100);
    }
    lcd.clear();
    lcd.print("Approx. BAC:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

float mapFloat(float x, float in_min, float in_max, float out_min, float out_max) {
    /**
     * @brief The Arduino library's map function implemented to work with floats
     * See the Arduino reference here: https://www.arduino.cc/reference/en/language/functions/math/map/
     * @param x: the number to map
     * @param in_min: the lower bound of the value’s current range
     * @param in_max: the upper bound of the value’s current range
     * @param out_min: the lower bound of the value’s target range
     * @param out_max: the upper bound of the value’s target range
     * @returns The mapped value
     */
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
