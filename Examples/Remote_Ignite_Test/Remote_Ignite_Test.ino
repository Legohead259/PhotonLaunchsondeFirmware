#include "chirpper.h"
#include <RH_RF95.h>

#define CHIRPPER_PIN 9
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7
#define RFM95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

char LAUNCH_KWD[6] = "LAUNCH"; 

void blink();

//=====ARDUINO SETUP=====
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);

    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(9600);
    while (!Serial); //Wait for serial terminal to be available

    Serial.println("Launchsonde LoRa TX Test!");

    //maunual reset
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!rf95.init()) {
        Serial.println("LoRa radio init failed!");
        while (1); //do nothing else!
    }
    Serial.println("LoRa radio init OK!");

    if (!rf95.setFrequency(RFM95_FREQ)) {
        Serial.println("setFrequency failed");
        while (1);
    }
    Serial.print("Set Freq to: "); Serial.println(RFM95_FREQ);
    Serial.println(LAUNCH_KWD); //DEBUG

    rf95.setTxPower(23, false); //Sets max transmitter power. Range is 5-23 dbm; default is 13 dbm
}

void loop() {
    Chirpper chirpper(CHIRPPER_PIN);

    while (1) {
        if (rf95.available()) {
            // Should be a message for us now   
            uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
            uint8_t len = sizeof(buf);
            
            if (rf95.recv(buf, &len)) {
                RH_RF95::printBuffer("Received: ", buf, len);
                Serial.print("Got: "); Serial.println((char)buf); //DEBUG
                // Serial.print("RSSI: "); Serial.println(rf95.lastRssi(), DEC); //DEBUG
                
                // Send a reply
                Serial.println(strncmp((char *)buf, LAUNCH_KWD, 6) == 0); //DEBUG
                if (strncmp((char *)buf, LAUNCH_KWD, 6) == 0) {
                    blink(); //IGNITE ROCKET MOTOR!
                    Serial.println("LAUNCHING!"); //DEBUG
                }
            }
            else {
                Serial.println("Receive failed");
            }
        }
    }
}

void blink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(500);
    digitalWrite(LED_BUILTIN, LOW);
}