/**
 * This sketch is an extended version of 
 * '010_temp_alcsensor_serial.ino': it has
 * an extra motion sensor, and some input is 
 * handled with interrupts instead of polling.
 */

#include <Wire.h>
#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LiquidCrystal_I2C.h>
#include <TimedAction.h>

#include "button.h"
#include "MQ7.h" // https://github.com/swatish17/MQ7-Library

//LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, LCD_BACKLIGHT);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_BMP280 bmp280;
char data[14]; // the data will be read into this array
const int COSensorAnalogPin = A1; // the MQ7 sensor's pin
const int touchsensorPin = 2;
const int motionSensorPin = 3; // interrupt pins on the Arduino Uno are digital pins 2 and 3
const int backlightButtonPin = 8;
int backlightState = HIGH;
const int motionSensorButtonPin = 9;
const int motionSensorIndicatorLedPin = 10;
const int motionSensorActiveLedPin = 4;
bool motionSensorIsOn = true;
Button backlightButton(backlightButtonPin);
Button motionSensorButton(motionSensorButtonPin);
MQ7 mq7(COSensorAnalogPin, 5.0);

enum State {
    showTemperature,
    showCOSensorResistance,
    showCOSensorPPM
};
State state = showTemperature;

enum BlinkLedState {
    firstBlink,
    shortDelay,
    secondBlink,
    longDelay
};
BlinkLedState ledState = firstBlink;
unsigned long lastBlinkAt = 0;

// variables that may be changed while handling an interrupt
volatile unsigned long backlightStateChangedAt = 0;
volatile bool backlightStateChanged = false;
volatile bool stateChanged = false;
// forward declaration of ISRs
void touchsensorInterrupt();
void motionSensorInterrupt();

void motionSensorEnabledBlink();
void printTemperature();
void printCOSensorResistance();
void printCOSensorPPM();
void serialPrintTemperature();
void serialPrintCOSensorPPM();
void toggleBacklight();


// TimedAction objects are used to refresh the value printed on the LCD screen and print values to serial
TimedAction refreshTemperature = TimedAction(3000, printTemperature);
TimedAction refreshCOResistance = TimedAction(3000, printCOSensorResistance); // CO sensor's resistance
TimedAction refreshCOPPM = TimedAction(3000, printCOSensorPPM); // CO concentration in PPM
TimedAction refreshTemperatureOnSerial = TimedAction(1800000, serialPrintTemperature);
TimedAction refreshCOPPMOnSerial = TimedAction(1800000, serialPrintCOSensorPPM);

void setup() {
    // setting up interrupts
    pinMode(touchsensorPin, INPUT);
    pinMode(motionSensorIndicatorLedPin, OUTPUT);
    pinMode(motionSensorActiveLedPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(touchsensorPin), touchsensorInterrupt, FALLING);
    attachInterrupt(digitalPinToInterrupt(motionSensorPin), motionSensorInterrupt, RISING);
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();
    lcd.print("Hello");
    delay(2000);
    lcd.clear();

    // checks whether the sensor's address is right
    if (!bmp280.begin(0x76)) {  
        Serial.println("Could not find a valid BMP280 sensor, check wiring!");
        lcd.print("Rossz cim");
        while (true) { }
    }

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


    // Turn off the L led on the board
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
}

void loop() {
    if (stateChanged) {
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
        stateChanged = false;
    }
    // Checking whether the values need to be refreshed
    refreshTemperature.check();
    refreshCOResistance.check();
    refreshCOPPM.check();
    refreshTemperatureOnSerial.check();
    refreshCOPPMOnSerial.check();

    // checking if the button was pressed
    // if yes, we set the LCD's backlight accordingly
    if (backlightButton.stateChanged() && !backlightButton.getPreviousState()) {
        toggleBacklight();
    }

    // we turn off the backlight after 5 seconds
    if (backlightState == HIGH && millis() - backlightStateChangedAt >= 5000) {
        toggleBacklight();
    }
    else if (backlightState == LOW && backlightStateChanged) { // we turn on the backlight if an interrupt occurred
        toggleBacklight();
        backlightStateChanged = false;
    }

    // disable or enable the motion sensor
    if (motionSensorButton.stateChanged() && !motionSensorButton.getPreviousState()) {
        if (motionSensorIsOn) { // if it was turned on, we turn it off
            detachInterrupt(digitalPinToInterrupt(motionSensorPin));
            motionSensorIsOn = false;
            digitalWrite(motionSensorIndicatorLedPin, HIGH);
            delay(400);
            digitalWrite(motionSensorIndicatorLedPin, LOW);
        }
        else { // if it was turned off, we turn it on
            attachInterrupt(digitalPinToInterrupt(motionSensorPin), motionSensorInterrupt, RISING);
            motionSensorIsOn = true;
        }
    }

    // if the motion sensor is enabled, we blink an led to indicate this
    if (motionSensorIsOn) {
        motionSensorEnabledBlink();
        if (digitalRead(motionSensorPin) == HIGH) {
            digitalWrite(motionSensorActiveLedPin, HIGH);
            backlightStateChangedAt = millis();
        }
        else 
            digitalWrite(motionSensorActiveLedPin, LOW);
    }
    // if the motion sensor is inactive, but the "motionSensorActive" led is HIGH,
    // we turn the led off (set its pin to LOW)
    else if (digitalRead(motionSensorActiveLedPin) == HIGH)
        digitalWrite(motionSensorActiveLedPin, LOW);

}

/**
 * @brief Sets the LCD backlight to LOW if the current value is HIGH,
 *  and to HIGH if the current value is LOW.
 */
void toggleBacklight() {
    if (backlightState == HIGH) {
//        lcd.setBacklight(LOW);
        lcd.noBacklight();
        backlightState = LOW;
    }
    else if (backlightState == LOW) {
//        lcd.setBacklight(HIGH);
        lcd.backlight();
        backlightState = HIGH;
    }
    backlightStateChangedAt = millis();
}

/**
 * @brief Code that runs when the touch sensor's state
 *  changes from LOW to HIGH.
 *  Changes what's displayed on the LCD.
 */
void touchsensorInterrupt() {
    stateChanged = true;
}

/**
 * @brief This function runs when the motion sensor is HIGH.
 *  Turns on the LCD backlight.
 */
void motionSensorInterrupt() {
    backlightStateChangedAt = millis();
    backlightStateChanged = true;
}

/**
 * @brief We blink a led if the motion sensor is enabled.
 *  This is a non-blocking way to do that (no delay() is used).
 */ 
void motionSensorEnabledBlink() {
    switch (ledState) {
        case firstBlink:
            if (millis() - lastBlinkAt >= 2800) {
                digitalWrite(motionSensorIndicatorLedPin, HIGH);
                ledState = shortDelay;
                lastBlinkAt = millis();
            }
            break;
        case shortDelay:
            if (millis() - lastBlinkAt >= 50) {
                digitalWrite(motionSensorIndicatorLedPin, LOW);
                ledState = secondBlink;
                lastBlinkAt = millis();
            }
            break;
        case secondBlink:
            if (millis() - lastBlinkAt >= 100) {
                digitalWrite(motionSensorIndicatorLedPin, HIGH);
                ledState = longDelay;
                lastBlinkAt = millis();
            }
            break;
        case longDelay:
            if (millis() - lastBlinkAt >= 50) {
                digitalWrite(motionSensorIndicatorLedPin, LOW);
                ledState = firstBlink;
                lastBlinkAt = millis();
            }
            break;
        default:
            break;
    }
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
