#include "i2c.h"
#include <Arduino.h>

I2C::I2C()
{
    Wire.begin();
    Wire.setClock(400000);
}

void I2C::writeRegByte(uint8_t addr, uint8_t reg, uint8_t data)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void I2C::writeReg16Bytes(uint8_t addr, uint16_t reg, uint8_t* data, uint8_t length)
{
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    for (uint8_t i = 0; i < length; i++)
    {
        Wire.write(data[i]);
    }
    Wire.endTransmission();
}

void I2C::writeReg16Byte(uint8_t addr, uint16_t reg, uint8_t data)
{
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.write(data);
    Wire.endTransmission();
}

void I2C::writeReg16Words(uint8_t addr, uint16_t reg, uint16_t* data, uint8_t length)
{
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    for (uint8_t i = 0; i < length; i++)
    {
        Wire.write(data[i] >> 8);
        Wire.write(data[i] & 0xFF);
    }
    Wire.endTransmission();
}

uint8_t I2C::readRegByte(uint8_t addr, uint8_t reg)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, (uint8_t)1);
    uint8_t data = Wire.read();
    return data;
}

void I2C::readRegBytes(uint8_t addr, uint8_t reg, uint8_t* buffer, uint8_t length)
{
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, length);
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = Wire.read();
    }
}

uint8_t I2C::readReg16Byte(uint8_t addr, uint16_t reg)
{
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, (uint8_t)1);
    uint8_t data = Wire.read();
    return data;
}

void I2C::readReg16Bytes(uint8_t addr, uint16_t reg, uint8_t* buffer, uint8_t length)
{
    Wire.beginTransmission(addr);
    Wire.write(reg >> 8);
    Wire.write(reg & 0xFF);
    Wire.endTransmission(false);
    Wire.requestFrom(addr, length);
    for (uint8_t i = 0; i < length; i++)
    {
        buffer[i] = Wire.read();
    }
}