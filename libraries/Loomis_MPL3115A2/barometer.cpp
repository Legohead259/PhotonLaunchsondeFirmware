#include "barometer.h"
#include <Arduino.h>

Barometer::Barometer(I2C& i2c)
    : i2c(i2c)
{
    uint8_t whoami = i2c.readRegByte(addr, 0x0C);
    Serial.print("Baro ID: ");
    Serial.println(whoami, HEX);
    i2c.writeRegByte(addr, 0x13, 0x07); //Setting some register in the data config
    i2c.writeRegByte(addr, MPL3115A2_CTRL_REG1, 0x00); //reset the CTRL_REG1 byte
    i2c.writeRegByte(addr, MPL3115A2_CTRL_REG1, 1); //enable the MPL3115A2
    setOverSamplingRatio();
    Serial.print("Enabled: "); Serial.println(getMode()); //DEBUG
    delay(minSampleInterval); //Wait to make sure altimeter is ready for reading
}

// boolean Barometer::begin() {
//   uint8_t whoami = i2c.readRegByte(MPL3115A2_WHOAMI);
//   if (whoami != 0xC4) {
//     return false;
//   }

//   write8(MPL3115A2_CTRL_REG1, MPL3115A2_CTRL_REG1_RST); //reset MPL3115A2
//   delay(10);

//   while (read8(MPL3115A2_CTRL_REG1) & MPL3115A2_CTRL_REG1_RST) //read if device is still resetting
//     delay(10);

//   _ctrl_reg1.reg = MPL3115A2_CTRL_REG1_OS128 | MPL3115A2_CTRL_REG1_ALT; //0b10111000 //NEEDS to change to 0b10100000

//   write8(MPL3115A2_CTRL_REG1, _ctrl_reg1.reg);

//   write8(MPL3115A2_PT_DATA_CFG, MPL3115A2_PT_DATA_CFG_TDEFE |
//                                     MPL3115A2_PT_DATA_CFG_PDEFE |
//                                     MPL3115A2_PT_DATA_CFG_DREM);
//   return true;
// }


// =====SETUP FUNCTIONS=====


/**
 * Sets the oversampling ratio. Configurable according to Table 58 and 59 of MPL3115A2 datasheet
 * @param ratio - Defualts to 0x10100000 or x16
*/
void Barometer::setOverSamplingRatio(int ratio=0b10100000)
{
    i2c.writeRegByte(addr, 0x26, ratio);
}

// NOTE: USE ABSOLUTELY BREAKS THE SENSOR READINGS! DO NOT USE!
// /**
//  * Sets the barometric input for Mean Sea Level altitude calculations. Default value is 101,326 Pa. Check local airport for pressure!
//  * @param p - Defaults to 101326 Pa
//  */
// void Barometer::setPressure(uint16_t p=101326) {
//     // uint16_t pressure = p/2; //Pressure in 2 Pa
//     i2c.writeRegByte(addr, 0x14, p);
// }

/**
 * Calibrates the starting height of the altimeter. Used for offset calculations during flight
*/
bool Barometer::calibrateStartingHeight()
{
    int failzone = 1;
    Meters total = 0;
    const uint8_t reps = 30;
    for (uint8_t i = 0; i < reps; i++)
    {
        total += getAltitude();
        delay(minSampleInterval);
        // Serial.print(i); Serial.print(": "); Serial.println(total);  //DEBUG
    }
    startingHeight = total / reps;

    if (startingHeight < failzone && startingHeight > -failzone) { //Check if the altitude is truely zeroed
        return true;
    }
    else {
        return false;
    }
    // Serial.println(startingHeight); //DEBUG
}

/**
 * Activates the altimeter
 * NOTE: HAS ISSUE WITH TOGGLING ACTIVE STATE. NEEDS LOGIC!
*/
void Barometer::setActive()
{
    activated = true;
    uint8_t data = i2c.readRegByte(addr, 0x26) + 1;
    // Serial.print("Active - 0x26 Reg: "); Serial.println(data, BIN); //DEBUG
    i2c.writeRegByte(addr, 0x26, data);

    // uint8_t ctrlData = i2c.readRegByte(addr, MPL3115A2_CTRL_REG1); //Reads the current CTRL_REG1 status
    // if (!(ctrlData & 1)) { //Check if the LSB of CTRL_REG1 is 0 (set to standby)
    //     i2c.writeRegByte(addr, MPL3115A2_CTRL_REG1, ctrlData+1); //Enables the MPL3115A2
        // Serial.print("Activate - CTRL_REG1 byte: "); Serial.println(ctrlData+1); //DEBUG
    // }
}

uint8_t Barometer::getMode() {
    return i2c.readRegByte(addr, 0x11);
}


// =====SAMPLING FUNCTIONS=====


/**
 * Checks if there is data available for reading
 * @return if data is available
*/
bool Barometer::isDataReady()
{
    // Serial.print("Status byte: "); Serial.println(i2c.readRegByte(addr, 0x00)); //DEBUG
    return (i2c.readRegByte(addr, 0x00) & 0x04);
}

/**
 * Clears the OST bit in the 0x26 control register to begin taking another sample. See documentation in Table 58 of MPL3115A2 datasheet 
*/
void Barometer::initiateSample()
{
    uint8_t currentSetting = i2c.readRegByte(addr, 0x26); //reads the 0x26 register
    // uint8_t clearOST = currentSetting & ~(1 << 1);
    // i2c.writeRegByte(addr, 0x26, clearOST);
    uint8_t setOST = currentSetting | (1 << 1); //clears the OST bit for a new sample //0b00000010
    // Serial.print("Initiate - 0x26 Reg: "); Serial.println(currentSetting, BIN); //DEBUG
    i2c.writeRegByte(addr, 0x26, setOST);
}

/**
 * Gets the change in altitude relative to the starting altitude as reported by the sensor
 * @return the change in altitude relative to the starting altitude
 */
Meters Barometer::getAltitude()
{
    if (isDataReady())
    {
        uint8_t data[3];
        i2c.readRegBytes(addr, 0x01, data, 3);

        Meters altitude = (float)((data[0] << 8) | data[1]) + 
            ((float)(data[2] >> 4) / 16.0f);
        initiateSample(); //Resets sample interval for next read cycle
        delay(minSampleInterval);
        altitude = altitude - startingHeight;
        return altitude;
    }
    else return -5000;
}


// =====UTILITY FUNCTIONS=====


float Barometer::findRange(float* numbers, uint8_t length)
{
    float min, max, diff;
    min = numbers[0];
    max = numbers[0];
    for (uint8_t i = 1; i < length; i++)
    {
        if (numbers[i] < min) min = numbers[i];
        if (max < numbers[i]) max = numbers[i];
    }

    diff = max - min;
    return diff;
}

float Barometer::findAvg(float* numbers, uint8_t length)
{
    float total = 0;
    for (uint8_t i = 0; i < length; i++)
    {
        total += numbers[i];
    }
    return total / length;
}

void Barometer::waitForLaunch(float* output, uint8_t length)
{
    const uint8_t maxBufferSize = 20;
    if (length <= maxBufferSize)
    {
        float buffer[maxBufferSize] = {0};
        while (findRange(buffer, length) < 4.5f && findAvg(buffer, length) < 5.0f)
        {
            for (uint8_t i = 0; i < length; i++)
            {
                buffer[i] = getAltitude();
                delay(minSampleInterval);
            }
        }

        memcpy(output, buffer, length*sizeof(float));
    }
}

// void reset() {
//     for ()
// }
