#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

/**
 * @class Button
 * @brief A class that defines methods to work with a pushbutton on Arduino
 */
class Button {
    const int pin; // the pin, on which the button connects to the board
    bool previousState; // true if HIGH, false if LOW

    public:
        Button(int pin);
        bool isPressed();
        bool stateChanged();
        bool getPreviousState();

};

#endif