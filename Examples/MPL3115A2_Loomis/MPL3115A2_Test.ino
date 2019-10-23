#include "i2c.h"
#include "barometer.h"
#include "chirpper.h"
// #include <Arduino.h>

#define CHIRPPER_PIN 9

// =====FUNCTION PROTOTYPES=====
// void chirp(int pulse=25);
// void readyChrip(int intv=125, int iter=3);
void flash(int intv=1000);

// =====ARDUINO EXECUTION=====
/**
 * Arduino configuration!
 */
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(CHIRPPER_PIN, OUTPUT);
    
    flash();
}

void loop() {
  // =====OBJECT INSTANTIATION AND CONFIGURATION=====
  I2C i2c;
  Barometer baro(i2c);
  Chirpper chirpper(CHIRPPER_PIN);

  //Calibrate Altimeter starting height w/ feedback
  chirpper.chirp(250);
  baro.calibrateStartingHeight();
  chirpper.readyChirp();

  while (1) {
    flash();

    Serial.print("Delta-tude: "); Serial.print(baro.getAltitude()); Serial.println(" m");
  }
}

//=====UTILITY FUNCTIONS=====

void flash(int intv=1000) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(intv);
    digitalWrite(LED_BUILTIN, LOW);
}
