#ifndef I2C_H
#define I2C_H

#include <Wire.h>

class I2C
{
public:
    I2C();
    void writeRegByte(uint8_t addr, uint8_t reg, uint8_t data);
    void writeReg16Bytes(uint8_t addr, uint16_t reg, uint8_t* data, uint8_t length);
    void writeReg16Byte(uint8_t addr, uint16_t reg, uint8_t data);
    void writeReg16Words(uint8_t addr, uint16_t reg, uint16_t* data, uint8_t length);

    uint8_t readRegByte(uint8_t addr, uint8_t reg);
    void readRegBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length);
    uint8_t readReg16Byte(uint8_t addr, uint16_t reg);
    void readReg16Bytes(uint8_t addr, uint16_t reg, uint8_t* buffer, uint8_t length);
};

#endif