#include "barometer.h"
#include "i2c.h"
#include "chirpper.h"
#include <RH_RF95.h>

#define CHIRPPER_PIN 9
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RFM95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

//=====ARDUINO SETUP=====
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);

    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(9600);
    // while (!Serial); //Wait for serial terminal to be available

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

    rf95.setTxPower(23, false); //Sets max transmitter power. Range is 5-23 dbm; default is 13 dbm
}

void loop() {
    //Object instantiation placed down here to prevent bootloader corruption issues on Adafruit RFM95X 32u4 Feather
    I2C i2c;
    Barometer baro(i2c);
    Chirpper chirpper(CHIRPPER_PIN);

    // chirpper.chirp(250); //Starting calibration
    baro.calibrateStartingHeight();
    // chirpper.readyChirp(); //Altitude calibrated

    float altitude = 0;
    long time = 0;

    while (1) {
        //Timestamp Recording
        time = millis(); //ms
        char timeStamp[sizeof(time)*1+1];
        sprintf(timeStamp,"%lu,", time); //Converts the timestamp into a string

        //Altitude Gathering
        altitude = baro.getAltitude();
        Serial.print("Delta-tude: "); Serial.print(altitude); Serial.println(" m"); //DEBUG
        char altitudePkt[6+1];
        dtostrf(altitude, 6, 2, altitudePkt);

        //Packet Formation
        char packet[sizeof(altitudePkt)+sizeof(timeStamp)] = "";
        strcat(packet, timeStamp); //Copy the timestamp into the packet
        // strcat(packet, ","); //Add a comma
        strcat(packet, altitudePkt); //Add the altitude data
        // Serial.println("Sending to rf95_server..."); delay(10); //DEBUG
        Serial.print("Sent: "); Serial.println(packet); //DEBUGP
        rf95.send((uint8_t*)packet, 20); //Send data to LoRa module

        // Serial.println("Waiting for packet to complete..."); delay(10); //DEBUG
        rf95.waitPacketSent(); //Wait for packet to complete

        // // Wait for reply
        // uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
        // uint8_t len = sizeof(buf);
        // Serial.println("Waiting for replay..."); delay(10); //DEBUG
        // if (rf95.waitAvailableTimeout(1000)) {
        //     if (rf95.recv(buf, &len)) {
        //         Serial.print("Got reply: "); Serial.println((char*)buf); //DEBUG
        //         Serial.print("RSSI: "); Serial.println(rf95.lastRssi(), DEC); //DEBUG - Tells strength of last signal recieved
        //     }
        //     else {
        //         Serial.println("Recieve failed!"); //DEBUG
        //     }
        // }
        // else {
        //     Serial.println("No reply, is there a listener around?");
        // }
        delay(50);
    }
}