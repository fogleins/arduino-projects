#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <TimedAction.h>

#include "button.h"
#include "MQ7.h" // https://github.com/swatish17/MQ7-Library

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
Adafruit_BMP280 bmp280;
char data[14]; // the data will be read into this array
const int COSensorAnalogPin = A1; // the MQ7 sensor's pin
const int touchsensorPin = 2;
const int buttonPin = 7;
int touchsensorPrevState = LOW; // the previous state of the touch sensor
int touchsensorState = LOW;
int backlightState = HIGH;
Button button(buttonPin);
MQ7 mq7(COSensorAnalogPin, 5.0);

enum State {
    showTemperature,
    showCOSensorResistance,
    showCOSensorPPM
};
State state = showTemperature;

void printTemperature();
void printCOSensorResistance();
void printCOSensorPPM();
void serialPrintTemperature();
void serialPrintCOSensorPPM();

// TimedAction objects are used to refresh the value printed on the LCD screen and print values to serial
TimedAction refreshTemperature = TimedAction(3000, printTemperature);
TimedAction refreshCOResistance = TimedAction(3000, printCOSensorResistance); // CO sensor's resistance
TimedAction refreshCOPPM = TimedAction(3000, printCOSensorPPM); // CO concentration in PPM
TimedAction refreshTemperatureOnSerial = TimedAction(60000, serialPrintTemperature);
TimedAction refreshCOPPMOnSerial = TimedAction(60000, serialPrintCOSensorPPM);

void setup() {
    Serial.begin(9600);
    lcd.begin(16, 2);
    lcd.print("Hello");
    delay(2000);
    lcd.clear();
    // checks whether the sensor's address is right
    if (!bmp280.begin(0x76)) {  
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        lcd.print("Rossz cim");
        while (true) { }
    }
    pinMode(touchsensorPin, INPUT);
    // TimedActions don't have to be checked yet
    refreshTemperature.disable();
    refreshCOResistance.disable();
    refreshCOPPM.disable();
    lcd.print("Ready.");
    lcd.setCursor(0, 1);
    lcd.print("Touch the sensor");
    // continues when the sensor is touched
    while (digitalRead(touchsensorPin) != HIGH) { }
    refreshTemperatureOnSerial.enable();
    refreshCOPPMOnSerial.enable();
}

void loop() {
    touchsensorPrevState = touchsensorState;
    touchsensorState = digitalRead(touchsensorPin);
    // if the touchsensor is touched
    if (touchsensorPrevState == LOW && touchsensorState == HIGH) {
        switch (state) {
            case showTemperature:
                printTemperature();
                state = showCOSensorResistance;
                refreshCOPPM.disable();
                refreshTemperature.enable();
                break;
            case showCOSensorResistance:
                printCOSensorResistance();
                state = showCOSensorPPM;
                refreshTemperature.disable();
                refreshCOResistance.enable();
                break;
            case showCOSensorPPM:
                printCOSensorPPM();
                state = showTemperature;
                refreshCOResistance.disable();
                refreshCOPPM.enable();
                break;
            default: // this branch should not be reached
                lcd.clear();
                lcd.print("error");
                break;
        }
    }

    // checking if the button was pressed
    // if yes, we set the LCD's backlight accordingly
    if (button.stateChanged() && !button.getPreviousState()) {
        if (backlightState == HIGH) {
            lcd.setBacklight(LOW);
            backlightState = LOW;
        }
        else if (backlightState == LOW) {
            lcd.setBacklight(HIGH);
            backlightState = HIGH;
        }
    }

    // Checking whether the values need to be refreshed
    refreshTemperature.check();
    refreshCOResistance.check();
    refreshCOPPM.check();
    refreshTemperatureOnSerial.check();
    refreshCOPPMOnSerial.check();
}

/**
 * @brief Prints the temperature onto the LCD screen
 */
void printTemperature() {
    float temperature = bmp280.readTemperature();
    sprintf(data, "%d.%02u %cC\0", (unsigned int) temperature, (unsigned int) (temperature * 100) % 100, 223);
    lcd.clear();
    lcd.print("Temperature:");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

/**
 * @brief Prints the temperature to the serial port
 */
void serialPrintTemperature() {
    float temperature = bmp280.readTemperature();
    Serial.println(temperature);
}

/**
 * @brief Prints the value read from the MQ7 sensor's analog input
 */
void printCOSensorResistance() {
    int value = analogRead(COSensorAnalogPin);
    lcd.clear();
    lcd.print("CO level:");
    // lcd.setCursor(16 - strlen(data), 1);
    lcd.setCursor(12, 1);
    lcd.print(value);
}

/**
 * @brief Prints the MQ7's value in PPM to the lcd
 */
void printCOSensorPPM() {
    float ppm = mq7.getPPM();
    sprintf(data, "%d.%02u PPM\0", (unsigned int) ppm, (unsigned int) (ppm * 100) % 100, 223);
    lcd.clear();
    lcd.print("CO concentration");
    lcd.setCursor(16 - strlen(data), 1);
    lcd.print(data);
}

/**
 * @brief Prints the MQ7's value in PPM to serial
 */
void serialPrintCOSensorPPM() {
    float ppm = mq7.getPPM();
    Serial.println(ppm);
}
