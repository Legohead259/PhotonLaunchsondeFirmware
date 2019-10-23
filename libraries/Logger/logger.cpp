#include "logger.h"
#include <Arduino.h>
#include <SPI.h>
#include <SD.h>

Logger::Logger() {
    Serial.print("Initializing SD card...");

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        while (1);
    }
    Serial.println("card initialized.");
}

File createNewFile() {
    
}