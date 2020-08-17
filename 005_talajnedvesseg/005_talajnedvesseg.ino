// see attached png for circuit connections

const int greenPin = 2;
const int orangePin = 3;
const int redPin = 4;
const int sensorInPin = A0;

/* measured min and max values 
 * are used in the map() function to determine 
 * the soil moisture percentage instead of a 
 * number between 0 and 1023
*/
const int inWater = 305;
const int inAir = 580;
int moisturePercentage;

void setup() {
    //Serial.begin(9600);
    pinMode(redPin, OUTPUT);
    pinMode(orangePin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void loop() {
    int moistureLevel = analogRead(sensorInPin);
    if (moistureLevel > inAir)
        moisturePercentage = 0;
    else if (moistureLevel < inWater)
        moisturePercentage = 100;
    else
        moisturePercentage = map(moistureLevel, inAir, inWater, 0, 100);
    //Serial.println(moistureLevel);
    //Serial.println(moisturePercentage);

    if (moisturePercentage <= 15) {
        digitalWrite(redPin, HIGH);
        digitalWrite(orangePin, LOW);
        digitalWrite(greenPin, LOW);
    }
    else if (moisturePercentage > 15 && moisturePercentage < 50) {
        digitalWrite(redPin, LOW);
        digitalWrite(orangePin, HIGH);
        digitalWrite(greenPin, LOW);
    }
    else if (moisturePercentage >= 50) {
        digitalWrite(redPin, LOW);
        digitalWrite(orangePin, LOW);
        digitalWrite(greenPin, HIGH);
    }
    delay(200);
}
