// This project is a music box

// Initialization and pinmappings

#include <Arduino.h>
#include <cstdint>
#include <cstdlib>
#include <sys/_intsup.h>
#include "api/Common.h"
#include "avr/pgmspace.h"
#include "rgbTCS3472x.hpp"

// For debug output on it's own lane using GPIO (Requires UART-USB bridge)
// --- FLAG: Comment out this line to use Serial (USB) instead ---
// Note that this flag controls where the raw sensor debug output goes
#define USE_HARDWARE_UART // For raw data output lane
// ----------------------------------------------------------------
// This one controls the port that Debug Terminal traffic is carried over
// Toggle the 'streaming' variable to declutter output over one port

#ifdef USE_HARDWARE_UART
#define DEBUG_SERIAL Serial1 // Tx and Rx 0 on giga
#else
#define DEBUG_SERIAL Serial // USB UART
#endif

#ifdef DTERM_HARDWARE_UART
#define DTERM_SERIAL Serial
#else
#define DTERM_SERIAL Serial1
#endif

#define BAUD_RATE 115200 // Serial UART baud rate to PC for debug output

const unsigned long DEBUG_INTERVAL = 100; // 100 ms
unsigned long lastOut = 0; // Marker for the time of previous output

bool streaming = true;
char cmdBuf[64]; // Serial input buffer
uint8_t cmdLen = 0; // Current length of buffer contents

const uint8_t illPin = D2; // Common pin to toggle illumination on the RGB color sensors
bool illPinState = false; // Track state of illumination pin

bool sensor_ok[NUM_SENSORS] = {}; // Sensors that respond to init are marked

// Debug function to can currently configured I2C busses for devices and report
void scanBus(TwoWire &bus, const char *name){
  DTERM_SERIAL.print("Scan on "); DTERM_SERIAL.println(name);
  for (uint8_t a = 1; a < 127; a++){
    bus.beginTransmission(a);
    if (bus.endTransmission() == 0){
      DTERM_SERIAL.print("  found 0x"); DTERM_SERIAL.println(a, HEX);
    }
  }
}


void setup(){
    delay(500); // Startup settle delay (So I don't miss debug output)
    DTERM_SERIAL.begin(BAUD_RATE);
    DTERM_SERIAL.print("\033[2J\033[H");
    DTERM_SERIAL.println("boot!");

    // Initialize pinout
    pinMode(illPin, OUTPUT);
    
    // Initialize I2C hardware
    for (int i = 0; i < NUM_SENSORS; i++){ // Increase this to tile sensors
        DTERM_SERIAL.print("Sensor "); Serial.print(sensor_names[i]);
        sensor_ok[i] = initSensor(*rgb_sensors[i]);
        DTERM_SERIAL.print(sensor_names[i]);
        DTERM_SERIAL.println(sensor_ok[i] ? " OK" : " NOT FOUND");
    }

    // Scan busses for device address diagnostics
      for (int i = 0; i < NUM_SENSORS; i++){
          scanBus(*rgb_sensors[i], sensor_names[i]);
          DTERM_SERIAL.print("ID reg read = 0x");
          DTERM_SERIAL.println(readReg(*rgb_sensors[0], REG_ID), HEX);
      }
    
    delay(5); // Wait

    
    DTERM_SERIAL.println("Initialization Complete!");
}

// For each of the connected sensors, print readings for debug to USB
void printRGBReadings(){
    for (int i = 0; i < 3; i++) {
    if (readReg(*rgb_sensors[i], REG_STATUS) & 0x01) {
      Color k = readColor(*rgb_sensors[i]);
      Serial.print(sensor_names[i]); // Name 
      DEBUG_SERIAL.print(" C:"); DEBUG_SERIAL.print(k.c); // Values

      DEBUG_SERIAL.print(" R:"); DEBUG_SERIAL.print(k.r);
      DEBUG_SERIAL.print(" G:"); DEBUG_SERIAL.print(k.g);
      DEBUG_SERIAL.print(" B:"); DEBUG_SERIAL.println(k.b);
    }
  }
}

// Take action on received debug commands
void handleCmd(char *cmd){
    // Accept and handle simple commands to toggle and view system state
    if (strcmp(cmd, "led") == 0){
      illPinState = !illPinState;
      digitalWrite(illPin, illPinState ? HIGH : LOW);
      DTERM_SERIAL.println("led toggled");
    }
    else if (strcmp(cmd, "stream") == 0) {
        streaming = !streaming;
        DTERM_SERIAL.print("sensor debug stream ");
        DTERM_SERIAL.println(streaming ? "on" : "off");
    }
    else if (strcmp(cmd, "status") == 0) { // output state variables on cmd
        DTERM_SERIAL.println("STATE");
        DTERM_SERIAL.print("streaming=");
        DTERM_SERIAL.println(streaming);
        DTERM_SERIAL.print(" led");
        DTERM_SERIAL.println(illPinState ? " on" : " off");
        DTERM_SERIAL.println("Sensor states:");
        for (int i = 0; i < NUM_SENSORS; i++){
            // For each of the configured sensors print status as of last boot
            DTERM_SERIAL.print(sensor_names[i]);
            DTERM_SERIAL.println(sensor_ok[i] ? " OK" : " NOT FOUND");
        }
    }
// Command template
//  else if (strcmp(cmd, "") == 0){}
    else {
        DTERM_SERIAL.print("ERR unknown: ");
        DTERM_SERIAL.println(cmd);
    }
}

// Format incoming debug commands
void readSerialCmd(){
    while (DTERM_SERIAL.available() > 0){
        char c = DTERM_SERIAL.read();

        if (c == '\n' || c == '\r'){
            if (cmdLen > 0){
                cmdBuf[cmdLen] = '\0';
                DTERM_SERIAL.println(); // Newline
                handleCmd(cmdBuf);
                cmdLen = 0;
            }
        } else if (cmdLen < sizeof(cmdBuf) - 1){ // Cmd still incomplete, continue until max limit
            cmdBuf[cmdLen++] = c; // Store next character
            DTERM_SERIAL.print(c); // Echo character
        }
    }
}


// Main program loop
void loop(){

    // Handles incoming debug commands
    readSerialCmd();
    
    // Functions within this statement execute periodically for debug output to
    //a PC terminal over USB.
    unsigned long now = millis(); // Current 'time'
    if (streaming && now - lastOut >= DEBUG_INTERVAL){
        lastOut = now;
        printRGBReadings();
    }
}
