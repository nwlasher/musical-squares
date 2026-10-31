/* Defines register addresses, interface functions and structures for interacting with
the TCS34725 RGB color/light sensors over I2C. */

#include <Wire.h>
#include <cstdint>
#include <stdint.h>

#define RGB_MAX_DATA_RATE 400 // Maximum data rate for the color sensors

// List of sensors mapped to their positions on the chassis and assoc. pinouts
enum SensorId : uint8_t {
    LEFT, // SDA/SCL 0 / Wire / pins 20 & 21
    CENTER, // SDA/SCL 1 / Wire1 / pins 16 & 17
    RIGHT, // SDA/SCL 2 / Wire2 / pins 8 & 9
    NUM_SENSORS
};

// Map enum to wire objects
TwoWire* rgb_sensors[NUM_SENSORS] = {&Wire, &Wire1, &Wire2};
// Map enum to string labels for debug output
const char* const sensor_names[NUM_SENSORS] = { "LEFT", "CENTER", "RIGHT" };

const uint8_t ADDR = 0x29; // I2C Address of TCS34725 chip 
const uint8_t CMD = 0x80; // Adresses the Command register 
const uint8_t CMD_AUTO = 0xA0; // Command register address with auto increment
// CMD_AUTO tells the sensor that each consecutive read should address the next register

/// Register addresses
const uint8_t REG_ENABLE = 0x00;
const uint8_t REG_ATIME = 0x01;
const uint8_t REG_CONTROL = 0x0F;
const uint8_t REG_ID = 0x12;
const uint8_t REG_STATUS = 0x13;
const uint8_t REG_CDATAL = 0x14;

// Meaningful register values
const uint8_t TCS34725_ID = 0x44;

/// Structure definitions
struct Color {uint16_t c, r, g, b; };

/// Interface function prototypes

// Initialize sensor
bool initSensor(TwoWire &bus);
// Write to a register on a given I2C bus
void writeReg(TwoWire &bus, uint8_t reg, uint8_t val);
// Read from a register off a given I2C bus
uint8_t readReg(TwoWire &bus, uint8_t reg);
// Read each of the 8, byte-sized registers from CDATAL to BDATAH to get color data
Color readColor(TwoWire &bus);

