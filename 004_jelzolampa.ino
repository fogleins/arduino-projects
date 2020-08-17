const int redPin = 4;
const int orangePin = 3;
const int greenPin = 2;

void setup() {
    pinMode(redPin, OUTPUT);
    pinMode(orangePin, OUTPUT);
    pinMode(greenPin, OUTPUT);
}

void loop() {
    digitalWrite(redPin, HIGH); // pirossal kezd
    delay(5000);
    digitalWrite(orangePin, HIGH); // 5 s után vált piros-sárgába
    delay(3000);
    digitalWrite(redPin, LOW); // piros-sárga elalszik...
    digitalWrite(orangePin, LOW);
    digitalWrite(greenPin, HIGH); // ...és a zöld kigyullad
    delay(15000);
    digitalWrite(greenPin, LOW); // a zöld elalszik...
    digitalWrite(orangePin, HIGH); // vissza sárgába
    // for (int i = 0; i <= 255; i += 5)
    // {
    //     analogWrite(orangePin, i);
    //     delay(50);
    // }
    delay(3000);
    digitalWrite(orangePin, LOW);
}