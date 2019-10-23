#include "i2c.h"
#include "barometer.h"
#include <Arduino.h>

#define CHIRPPER_PIN 9

// =====FUNCTION PROTOTYPES=====
void chirp(int pulse=25);
void flash();

// =====ARDUINO EXECUTION=====
void setup() {
    // =====OBJECT INSTANTIATION=====
    I2C i2c;
    Barometer baro(i2c);
    
    // chirp();
    pinMode(LED_BUILTIN, OUTPUT);

    flash();
}

void loop() {

}

//=====UTILITY FUNCTIONS=====
void chirp(int pulse=25) {
  digitalWrite(CHIRPPER_PIN, HIGH);
  delay(pulse);
  digitalWrite(CHIRPPER_PIN, LOW);
}

void flash() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
