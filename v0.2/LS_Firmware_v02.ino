/**
 * PROJECT PHOTON
 * FLORIDA INSTITUTE OF TECHNOLOGY, OEMS DEPARTMENT
 * UNDERWATER TECHNOLOGY LABORATORY
 * Supervising Professor: Dr. Stephen Wood, Ph.D, PE
 * 
 * Launchsonde Firmware Version 0.2 Created 10/09/2019 By Braidan Duffy
 * 
 * Theory of Operation:
 * This firmware is intended to relay instrumentation data from the launchsonde to a ground station using the LoRa RFM95 chipset
 * This version of the firmware expects the ground station to braodcast out a "sign"
 * This sign is defined by the SIGN char array and should be the same between the ground station and the launchsonde
 * During this phase, the firmware is in the BOOTING mode
 * When this firmware recieves the proper sign, it will broadcast a countersign and go into a STANDBY mode
 * The ground station is simultaneousy listening for the countersign similar to as stated above
 * NOTE: BOTH GROUND STATION AND LAUNCHSONDE FIRMWARE SHOULD HAVE THE SAME SIGN/COUNTERSIGN TO ENSURE THEY LINK!
 * ...
 * 
 * Last Revision: 10/21/2019 By Braidan Duffy
 */

#include "barometer.h"
#include "i2c.h"
#include "chirpper.h"
#include <RH_RF95.h>

#define CHIRPPER_PIN 9
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 7

#define RFM95_FREQ 915.0

#define MAX_CALIBRATION_TRIES 3

RH_RF95 rf95(RFM95_CS, RFM95_INT);

const char SIGN[28] = "ground control to major tom";
const char COUNTERSIGN[28] = "major tom to ground control";

int readyTime = 0;
int launchTime = 0;

enum Mode {
    FAILSAFE = -1,      //Launchsonde detects major failure state and safes itself
    BOOTING = 0,        //Launchsonde is booting and initializing
    STANDBY = 1,        //Launchsonde has established communication with the ground station and is awaiting commands
    INITIALIZING = 2,   //Launchsonde has recived the INITIATE command and is zeroing sensors
    READY = 3,          //Launchsonde has successfully zeroed and calibrated sensors and is ready for flight
    LAUNCHING = 4,      //Launchsonde is launching and actively transmitting telemetry
    RECOVERY = 5        //Launchsonde has either landed or a certain amount of time, turning on the locating beepers and beacon
};
Mode curMode = BOOTING;

enum Commands {
    ABORT,      //Launchsonde aborts operations
    INITIATE    //Launchsonde initiates operations
};

void blink();
bool listenFor(char msg[]);

//=============================
//=====EXECUTION FUNCTIONS=====
//=============================

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(RFM95_RST, OUTPUT);

    digitalWrite(RFM95_RST, HIGH); //Pull RST high on RFM95 to enable the radio

    Serial.begin(115200);

    //-------------------------------
    //---LORA RADIO INITIALIZATION---
    //-------------------------------

    //Maunually reset LoRa radio
    digitalWrite(RFM95_RST, LOW);
    delay(10);
    digitalWrite(RFM95_RST, HIGH);
    delay(10);

    while (!rf95.init()) { //Initialize LoRa radio
        Serial.println("LoRa radio init failed!");
        //TODO: Incorporate feedback for LoRa init failure
        while (1); //do nothing else!
    }
    Serial.println("LoRa radio init OK!"); //Debug

    if (!rf95.setFrequency(RFM95_FREQ)) {
        Serial.println("setFrequency failed");
        //TODO: Incorporate feedback for LoRa frequency failure
        while (1);
    }

    rf95.setTxPower(23, false); //Sets max transmitter power. Range is 5-23 dbm; default is 13 dbm
    //TODO: Incorporate feedback for LoRa initialization success

    //-----------------------------------------
    //---INITIAL COMMUNICATION ESTABLISHMENT---
    //-----------------------------------------

    while (curMode == BOOTING) {
        Serial.println("Waiting for sign..."); //DEBUG
        rf95.waitAvailable(); //Block until a broadcast is recieved
        if (listenFor(SIGN)) {
            for (int x=0; x<2; x++) { //Send countersign twice to ensure GS receives confirmation
                rf95.send((uint8_t*)COUNTERSIGN, 28); //Send counter sign
                rf95.waitPacketSent();
            }
            if (!rf95.waitAvailableTimeout(1500)) { //Wait for 1.5 seconds to make sure the ground station has stopped transmitting a sign
                curMode = STANDBY;
            }
        }
    }

    //-----------------------------------
    //---STANDBY FOR OPERATOR COMMANDS---
    //-----------------------------------

    rf95.send((uint8_t*)"Setting STANDBY mode...", 24); //Lets the ground station know that the launchsonde is entering STANDBY (mode 1)
}


void loop() {
    I2C i2c;
    Barometer baro(i2c);

    float altitude = 0;
    long time = 0;

    while(1) {
        while (!listenFor((char*)INITIATE)); //Blocks code while listening for the INITIATE command
        curMode = READY;
        rf95.send((uint8_t*)"Setting READY mode...", 22); //Lets the ground station know that the launchsonde is entering READY (mode 2)

        //Zeroing launch timers
        readyTime = millis(); //Records the time the ready was state was started

        //---------------------------
        //---CALIBRATING ALTIMETER---
        //---------------------------

        int tries = 0;
        while(!baro.calibrateStartingHeight()) { //Try to calibrate the altimeter
            tries++;
            if (tries >= MAX_CALIBRATION_TRIES) {
                curMode = FAILSAFE;
                rf95.send("[FATAL] ALTIMETER CALIBRATION FAILED!", 38); //Broadcast that the altimeter calibration failed
            }
        }
        rf95.send("[NOTIFICATION] READY!", 22); //Broadcast launchsonde is ready for flight

        //-------------------------
        //---RECORDING TIMESTAMP---
        //-------------------------

        time = millis(); //ms
        char timeStamp[sizeof(time)*1+1];
        sprintf(timeStamp,"%lu,", time); //Converts the timestamp into a string

        //------------------------
        //---GATHERING ALTITUDE---
        //------------------------

        altitude = baro.getAltitude();
        Serial.print("Delta-tude: "); Serial.print(altitude); Serial.println(" m"); //DEBUG
        char altitudePkt[6+1];
        dtostrf(altitude, 6, 2, altitudePkt);

        //-----------------------
        //---FORMATTING PACKET---
        //-----------------------

        char packet[sizeof(altitudePkt)+sizeof(timeStamp)] = "";
        strcat(packet, timeStamp); //Copy the timestamp into the packet
        strcat(packet, altitudePkt); //Add the altitude data
        Serial.print("Sent: "); Serial.println(packet); //DEBUGP
        rf95.send((uint8_t*)packet, 20); //Send data to LoRa module

        rf95.waitPacketSent(); //Wait for packet to complete
        delay(50);
    }
}


//==========================
//=====DEVICE FUNCTIONS=====
//==========================


bool listenFor(char msg[]) {
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    Serial.println("Waiting for replay..."); delay(10); //DEBUG
    if (rf95.waitAvailableTimeout(1000)) {
        if (rf95.recv(buf, &len)) {
            Serial.print("Recived: "); Serial.println((char*)buf); //DEBUG
            Serial.print("Compare to: "); Serial.println(msg); //DEBUG
            if (strcmp(msg, (char*)buf) == 0) { //Compare received message to target message
                return true;
            }
        }
        else {
            Serial.println("Recieve failed!"); //DEBUG
        }
    }
    else {
        Serial.println("No reply, is there a listener around?"); //DEBUG
    }
    return false;
}


//===========================
//=====UTILITY FUNCTIONS=====
//===========================


void blink() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
}