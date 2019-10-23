/**
 * PROJECT PHOTON
 * FLORIDA INSTITUTE OF TECHNOLOGY, OEMS DEPARTMENT
 * UNDERWATER TECHNOLOGY LABORATORY
 * Supervising Professor: Dr. Stephen Wood, Ph.D, PE
 * 
 * Launchsonde Firmware Version 0.1.2 Created 10/21/2019 By Braidan Duffy
 * 
 * Theory of Operation:
 * This firmware is intended to relay instrumentation data from the launchsonde to a ground station using the LoRa RFM95 chipset
 * On startup, the firmware initializes the chipsets and prepares the file format for the SD Card logging
 * To create a file for logging, the firmware captures the current date from the onboard RTC and uses that as the name for the .txt file
 * If there has already been a file for that date created, the appropriate log number is appended to the name
 * The firmware then opens the file and if it cannot, it blocks the code from continuing further
 * Note: This block is non-resettable besides a full hardware reset
 * 
 * In loop(), the firmware begins by instantiating the barometer/altimeter
 * Note: due to a bug in the current barometer library (as of 10/21/2019), the barometer MUST be instantiated in the loop or the bootloader corrupts
 * The firmware then calibrates the barometer's starting atitude using the library's function and begins formatting the data packet
 * The data packet is formed by capturing the time from the onboard RTC and the change-in-alitude (delta-tude) from the barometer
 * The data is then put into a comma-delimitted format for data packet transmission and logging on the SD card
 * 
 * Last Revision: 10/21/2019 By Braidan Duffy
 */

#include "barometer.h"
#include "i2c.h"
#include <RH_RF95.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RFM95_FREQ 915.0

RH_RF95 rf95(RFM95_CS, RFM95_INT);

RTC_PCF8523 rtc;
const int chipSelect = 10;
File dataFile;
char filename[30];

//=====ARDUINO SETUP=====
void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);

    digitalWrite(RFM95_RST, HIGH);

    Serial.begin(115200);

    //Maunual reset
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

    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        while (1); //do nothing else!
    }
    Serial.println("card initialized.");

    DateTime now = rtc.now();
    for (int x=0; x<100; x++) {
        sprintf(filename, "%02d%02d_%02d.txt", now.month(), now.day(), x); //ATM the filename can only be 8 chars long
        if (!SD.exists(filename)) {
            break;
        }
        Serial.println(filename); //DEBUG
    }

    //Open the SD card file
    //NOTE: Only ONE file can be open at a time, you must close before opening a new one
    dataFile = SD.open(filename, FILE_WRITE);

    if(!dataFile) {
        Serial.print("Couldnt create ");
        Serial.println(filename);
        while(1);
    }

    Serial.print("Writing to "); Serial.println(filename); //DEBUG
}

void loop() {
    //Object instantiation placed down here to prevent bootloader corruption issues on Adafruit RFM95X 32u4 Feather
    I2C i2c;
    Barometer baro(i2c);

    baro.calibrateStartingHeight();

    float altitude = 0;

    while (1) {
        //Timestamp Recording
        char timeStamp[20];
        DateTime now = rtc.now();
        sprintf(timeStamp, "%02d:%02d:%02d,",  now.hour(), now.minute(), now.second());
        Serial.println(timeStamp); //DEBUG

        //Altitude Gathering
        altitude = baro.getAltitude();
        Serial.print("Delta-tude: "); Serial.print(altitude); Serial.println(" m"); //DEBUG
        char altitudePkt[6+1];
        dtostrf(altitude, 6, 2, altitudePkt);

        //Packet Formation
        char packet[sizeof(altitudePkt)+sizeof(timeStamp)] = "";
        strcat(packet, timeStamp); //Add the timestamp into the packet
        strcat(packet, altitudePkt); //Add the altitude data
        Serial.print("Sent: "); Serial.println(packet); //DEBUGP
        rf95.send((uint8_t*)packet, 20); //Send data to LoRa module

        dataFile = SD.open(filename, FILE_WRITE);
        dataFile.println(packet);
        dataFile.close();

        rf95.waitPacketSent(); //Wait for packet to complete
        delay(50);
    }
}
