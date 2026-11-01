/* This file contains the source code for interface functions used in interactions with
   I2C and the TCS34725 RGB color/light sensors */

#include "TCS3472x.hpp"
#include <cstdint>

// Initialize sensor
bool initSensor(TwoWire &bus){
    bus.begin();
    bus.setClock(RGB_MAX_DATA_RATE);
    // Shout to sensor on bus, ensure it responds with a valid ID for the RGB sensor I'm using
    uint8_t id = readReg(bus, REG_ID);
    if (!(id == TCS3472_1_5_ID || id == TCS3472_3_7_ID)) return false;
    // Write configuration values to register
    writeReg(bus, REG_ATIME, 0xEB); // Set RGBC integration time (resolution)
    writeReg(bus, REG_CONTROL, 0x00); // Set bit 0 in ctrl for 4x gain, it is set to 1 with 0x00
    writeReg(bus, REG_ENABLE, 0x01); // Clock sensor itself
    delay(3); // Short delay to allow sensor time to wake up
    writeReg(bus, REG_ENABLE, 0x03); // Activate sensor ADC for readings
    return true;
}

// Write to a register on a given I2C bus
void writeReg(TwoWire &bus, uint8_t reg, uint8_t val){
    bus.beginTransmission(ADDR);
    bus.write(CMD | reg); // Command static address for write
    bus.write(val);
    bus.endTransmission();
}

// Read from a register off a given I2C bus
uint8_t readReg(TwoWire &bus, uint8_t reg){
    bus.beginTransmission(ADDR);
    bus.write(CMD | reg); // Command static address for read
    bus.endTransmission();
    bus.requestFrom(ADDR, (uint8_t)1); // Request byte from sensor
    return bus.read();
}

// Read each of the 8, byte-sized registers from CDATAL to BDATAH to get color data
Color readColor(TwoWire &bus){
    bus.beginTransmission(ADDR);
    bus.write(CMD_AUTO | REG_CDATAL); // Command sequentially incrementing reads from start reg
    bus.endTransmission();
    bus.requestFrom(ADDR, (uint8_t)8); // Request 8 bytes from sensor
    uint8_t v[8];
    for (int i = 0; i < 8; i++) v[i] = bus.read(); // Read 8 registers
    // Format Low and High bytes from each color reading into full halfword values
    return {(uint16_t)(v[1] << 8 | v[0]), (uint16_t)(v[3] << 8 | v[2]),
            (uint16_t)(v[5] << 8 | v[4]), (uint16_t)(v[7] << 8 | v[6])};
}
