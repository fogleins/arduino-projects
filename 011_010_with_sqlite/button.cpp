#include "button.h"

/**
 * @brief Constructor.
 * @param pin The number of the digital pin on which the button is connected to the board.
 */
Button::Button(int pin): pin(pin), previousState(false) {
    pinMode(pin, INPUT);
}

/**
 * @brief Checks if the button is pressed.
 * @returns True, if the button is pressed (e.g. digitalRead() returns HIGH),
 *  False, if the button is not pressed.
 */
bool Button::isPressed() {
    return digitalRead(this->pin) == HIGH;
}

/**
 * @brief Checks whether the button's state changed 
 *  (the button was pressed, and now it's released, for example)
 * @returns True, if the state changed, else False.
 */
bool Button::stateChanged() {
    if (this->isPressed() != this->previousState) {
        this->previousState = !this->previousState;
        return true;
    }
    return false;
}

/**
 * @brief Returns the button's previous state.
 * @note The button's state is only updated in the stateChanged() method.
 * @returns True, if the button was pressed, else False.
 */
bool Button::getPreviousState() {
    return previousState;
}