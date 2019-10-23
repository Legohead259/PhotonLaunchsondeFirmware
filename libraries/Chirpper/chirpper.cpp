#include "chirpper.h"
#include <Arduino.h>

/**
 * Uses an arduino-compatible buzzer to generate noise for various functions.
 * The class includes multiple functions for patterns and call outs that are completely customizable
 */
Chirpper::Chirpper(int p) {
    pin = p;
    pinMode(pin, OUTPUT);
}

/**
 * Generates a chrip of some length.
 * NOTE: There is no delay after the chirp built into this function
 * @param pulse - the length of the chirp
 */
void Chirpper::chirp(int pulse=25) {
  digitalWrite(pin, HIGH);
  delay(pulse);
  digitalWrite(pin, LOW);
}

/**
 * Generates a series of chirps at some interval
 * APPLICATION NOTE: for Project Photon, a series of three quick chirps is a ready/acknowledgement signal
 * @param intv - the interval between chirps
 * @param iter - the number of chirps
 */
void Chirpper::readyChirp(int intv=125, int iter=3) {
  for (int x=0; x<iter; x++) {
    chirp(intv);
    delay(intv);
  }
}