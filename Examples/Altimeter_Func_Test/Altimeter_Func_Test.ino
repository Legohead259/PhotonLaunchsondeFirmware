#include <Wire.h>


#define PERCENT_THRESHOLD 0.10 //%
#define CHIRPPER_PIN 9

//int sea_pressure = 101200; //Pa

void chirp(int pulse=25) {
  digitalWrite(CHIRPPER_PIN, HIGH);
  delay(pulse);
  digitalWrite(CHIRPPER_PIN, LOW);
}

float last_altm = 0;
float start_altm = 0;

void setup() {
//  if (! baro.begin()) {
//    Serial.println("Couldnt find sensor");
//    return;
//  }

  Wire.begin();

//  baro.setSeaPressure(sea_pressure);
  
  pinMode(CHIRPPER_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Adafruit_MPL3115A2 test!");

  chirp(); //Zeroing altimeter
//  start_altm = get_altm(9, 333);
  chirp(125); //Altimeter zeroed and ready
  Serial.print("Altimeter starting at: "); Serial.println(start_altm);
}

void loop() {
//  if (! baro.begin()) {
//    Serial.println("Couldnt find sensor");
//    return;
//  }
  
//  float pascals = baro.getPressure();
  // Our weather page presents pressure in Inches (Hg)
  // Use http://www.onlineconversion.com/pressure.htm for other units
//  Serial.print(pascals); Serial.println(" Pascals");
  
//    get_altm();

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
