/**************************************************************************/
/*!
    @file     Adafruit_MPL3115A2.cpp
    @author   K.Townsend (Adafruit Industries)
    @license  BSD (see license.txt)

    Example for the MPL3115A2 barometric pressure sensor

    This is a library for the Adafruit MPL3115A2 breakout
    ----> https://www.adafruit.com/products/1893

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0 - First release
*/
/**************************************************************************/

#include <Wire.h>
#include <Adafruit_MPL3115A2.h>

#define PERCENT_THRESHOLD 0.10 //%

// Power by connecting Vin to 3-5V, GND to GND
// Uses I2C - connect SCL to the SCL pin, SDA to SDA pin
// See the Wire tutorial for pinouts for each Arduino
// http://arduino.cc/en/reference/wire
Adafruit_MPL3115A2 baro = Adafruit_MPL3115A2();

void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit_MPL3115A2 test!");
}

float last_altm = 0;

void loop() {
  if (! baro.begin()) {
    Serial.println("Couldnt find sensor");
    return;
  }
  
//  float pascals = baro.getPressure();
  // Our weather page presents pressure in Inches (Hg)
  // Use http://www.onlineconversion.com/pressure.htm for other units
//  Serial.print(pascals); Serial.println(" Pascals");
  
  float avg_altm = 0;
  for (int x=0;x<3;x++) {
    float temp = 0;
    float altm_tmp = baro.getAltitude();
    delay(2.5);
    temp += altm_tmp;
    avg_altm = temp/3;
  }
  Serial.print(avg_altm); Serial.println(" meters");

//  float pdiff = abs(last_altm - avg_altm)/((last_altm + avg_altm)/2); //Percent difference
//  if (pdiff > PERCENT_THRESHOLD) {
//    Serial.print(avg_altm); Serial.println(" meters");
//    Serial.print("Percent Change: "); Serial.print(pdiff); Serial.println("%");
//    last_altm = avg_altm;
//  }

//  float tempC = baro.getTemperature();
//  Serial.print(tempC); Serial.println("*C");

//  delay(250);
}
