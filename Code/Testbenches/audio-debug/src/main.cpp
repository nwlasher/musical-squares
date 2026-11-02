// This project is a music box

// Initialization and pinmappings

#include <Arduino.h>
#include <cstdint>
#include <cstdlib>
#include <sys/_intsup.h>
#include "api/Common.h"
#include "avr/pgmspace.h"
#include "TCS3472x.hpp"
#include "synth.hpp"

// This block steers where the output goes for debug functions
//   DEBUG = raw sensor stream
//   DTERM = boot and debug terminal info (typed commands and their replies)
//
//   1 = DEBUG on UART,  DTERM on UART   (both over D0/D1)
//   2 = DEBUG on USB,   DTERM on UART
//   3 = DEBUG on UART,  DTERM on USB
//   4 = DEBUG on USB,   DTERM on USB  (Both over usb)

#define OUTPUT_CONFIG 1

#if OUTPUT_CONFIG == 1
  #define DEBUG_SERIAL Serial1
  #define DTERM_SERIAL Serial1
  #define USES_UART 1
  #define USES_USB  0
#elif OUTPUT_CONFIG == 2
  #define DEBUG_SERIAL Serial
  #define DTERM_SERIAL Serial1
  #define USES_UART 1
  #define USES_USB  1
#elif OUTPUT_CONFIG == 3
  #define DEBUG_SERIAL Serial1
  #define DTERM_SERIAL Serial
  #define USES_UART 1
  #define USES_USB  1
#elif OUTPUT_CONFIG == 4
  #define DEBUG_SERIAL Serial
  #define DTERM_SERIAL Serial
  #define USES_UART 0
  #define USES_USB  1
  #error "OUTPUT_CONFIG must be 1, 2, 3 or 4"
#endif

#define BAUD_RATE 115200 // Serial UART baud rate to PC for debug output

const unsigned long DEBUG_INTERVAL = 100; // 100 ms
unsigned long lastOut = 0; // Marker for the time of previous output

bool streaming = false; // Off by default due to speaker interference
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
    if (USES_USB){
        Serial.begin(BAUD_RATE);        
    }
    if (USES_UART){
            Serial1.begin(BAUD_RATE);
    }

    // Clear DTERMinal and mark boot
    DTERM_SERIAL.print("\033[2J\033[H");
    DTERM_SERIAL.println("boot!");

    // Initialize pinout
    pinMode(illPin, OUTPUT);
    
    // Initialize I2C hardware
    for (int i = 0; i < NUM_SENSORS; i++){ // Increase this to tile sensors
        DTERM_SERIAL.print("Sensor "); Serial.print(SENSOR_NAMES[i]);
        sensor_ok[i] = initSensor(*RGB_SENSORS[i]);
        DTERM_SERIAL.print(SENSOR_NAMES[i]);
        DTERM_SERIAL.println(sensor_ok[i] ? " OK" : " NOT FOUND");
    }

    // Scan busses for device address diagnostics
      for (int i = 0; i < NUM_SENSORS; i++){
          scanBus(*RGB_SENSORS[i], SENSOR_NAMES[i]);
          DTERM_SERIAL.print("ID reg read = 0x");
          DTERM_SERIAL.println(readReg(*RGB_SENSORS[0], REG_ID), HEX);
      }
    
    delay(5); // Wait

    // Initialize DAC
    if (!synthInit()) DEBUG_SERIAL.println("DAC init FAILED");

    
    DTERM_SERIAL.println("Initialization Complete!");
}

// For each of the connected sensors, print readings for debug to USB
void printRGBReadings(){
    DTERM_SERIAL.print("\033[2J\033[H"); // Clear terminal each print to appear in-place
    for (int i = 0; i < NUM_SENSORS; i++) {
    if (readReg(*RGB_SENSORS[i], REG_STATUS) & 0x01) {
      Color k = readColor(*RGB_SENSORS[i]);
      DEBUG_SERIAL.print(SENSOR_NAMES[i]); // Name 
      DEBUG_SERIAL.print(" C:"); DEBUG_SERIAL.print(k.c); // Values
      DEBUG_SERIAL.print(" R:"); DEBUG_SERIAL.print(k.r);
      DEBUG_SERIAL.print(" G:"); DEBUG_SERIAL.print(k.g);
      DEBUG_SERIAL.print(" B:"); DEBUG_SERIAL.println(k.b);
    }
  }
}

// Take action on received debug commands
void handleCmd(char *cmd){
    char *arg = strchr(cmd, ' ');
    if (arg) { *arg = '\0'; arg++; }          // Null terminate argument
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
            DTERM_SERIAL.print(SENSOR_NAMES[i]);
            DTERM_SERIAL.println(sensor_ok[i] ? " OK" : " NOT FOUND");
        }
    }
// Command template
//  else if (strcmp(cmd, "") == 0){}
    else if (strcmp(cmd, "play") == 0){ // Plays tones as commamded. glys' between
        uint8_t n = atoi(arg); // Int convertion on argument
        float nf = atof(arg); // Float convertion
        if (n <= NUM_TONES){ // raw hz will be > NUM_TONES anyway
            if (TRACKS[0].active){
                synthGlideTo(0, (Tone)n, 1000);
            }else {
                synthPlay(0, (Tone)n);   
            }
            DTERM_SERIAL.print("Now playing "); DTERM_SERIAL.println(TONE_NAMES[n]);
        }else if (nf >= MIN_FREQ && nf <= MAX_FREQ){
            if (TRACKS[0].active){
                synthGlideToHz(0, nf, 1000);                
            }else {
                synthPlayHz(0, nf);   
            }
            DTERM_SERIAL.print("Now playing at frequency: "); DTERM_SERIAL.println(nf);
        }
    }
    else if (strcmp(cmd, "stop") == 0){
        synthStop(0);
    }
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
    // Update DMA buffers for DAC imediately!
    synthUpdate();
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
