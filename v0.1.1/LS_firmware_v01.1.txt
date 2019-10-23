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
//    while (!Serial); //Block until Serial port opened

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

    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        while (1);
    }
    Serial.println("card initialized.");

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    DateTime now = rtc.now();
    for (int x=0; x<100; x++) {
        sprintf(filename, "%02d%02d_%02d.txt", now.month(), now.day(), x); //ATM the filename can only be 8 chars long
        if (!SD.exists(filename)) {
            break;
        }
        Serial.println(filename); //DEBUG
    }

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

    // chirpper.chirp(250); //Starting calibration
    baro.calibrateStartingHeight();
    // chirpper.readyChirp(); //Altitude calibrated

    float altitude = 0;

    while (1) {
        //Timestamp Recording
        // time = millis(); //ms
        char timeStamp[20];
        DateTime now = rtc.now();
        sprintf(timeStamp, "%02d:%02d:%02d,",  now.hour(), now.minute(), now.second());
        Serial.println(timeStamp); //DEBUG

        //Altitude Gathering
        altitude = baro.getAltitude();
        Serial.print("Delta-tude: "); Serial.print(altitude); Serial.println(" m"); //DEBUG
        char altitudePkt[6+1];
        dtostrf(altitude, 6, 2, altitudePkt);

        //TODO: Write packet to Adalogger!
        //Packet Formation
        char packet[sizeof(altitudePkt)+sizeof(timeStamp)] = "";
        strcat(packet, timeStamp); //Add the timestamp into the packet
        strcat(packet, altitudePkt); //Add the altitude data
        Serial.print("Sent: "); Serial.println(packet); //DEBUGP
        rf95.send((uint8_t*)packet, 20); //Send data to LoRa module

        // if the file is available, write to it:
        // if (dataFile) {
        dataFile = SD.open(filename, FILE_WRITE);
        dataFile.println(packet);
        dataFile.close();
            // print to the serial port too:
            // Serial.println(dataString);
        // }
        // if the file isn't open, pop up an error:
        // else {
            // Serial.println("error opening datalog.txt");
        // }

        rf95.waitPacketSent(); //Wait for packet to complete
        delay(50);
    }
}
