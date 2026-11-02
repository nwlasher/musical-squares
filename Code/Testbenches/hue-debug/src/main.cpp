// This project is a music box
// Initialization and pinmappings

#include <Arduino.h>
#include <cstdint>
#include <cstdlib>
#include <sys/_intsup.h>
#include "Wire.h"
#include "api/Common.h"
#include "avr/pgmspace.h"
#include "TCS3472x.hpp"
#include "pins_arduino.h"
#include "synth.hpp"
#include "hue.hpp"

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

// Constants
 
const unsigned long DEBUG_INTERVAL = 100; // 100 ms

const uint8_t illPin = D2; // Common pin to toggle illumination on the RGB color sensors

const uint16_t CLEAR_TO_IDLE   = 150;   // PLACEHOLDERS: calibrate against real tiles
const uint16_t CLEAR_TO_ACTIVE = 300;   // must be > CLEAR_TO_IDLE (this gap is the hysteresis)
const float    SAT_MIN         = 0.25f; // below this the hue is noise (gray/white)
const unsigned long SENSOR_POLL_MS = 200;

// Globals

bool illPinState = false; // Track state of illumination pin

bool sensor_ok[NUM_SENSORS] = {}; // Sensors that respond to init are marked

bool   idle[NUM_SENSORS];               // set all true in setup()
int8_t lastTone[NUM_SENSORS];           // -1 = nothing playing; set all -1 in setup()
unsigned long lastPoll = 0;
unsigned long lastOut = 0; // Marker for the time of previous output

bool streaming = false; // Off by default due to speaker interference
char cmdBuf[64]; // Serial input buffer
uint8_t cmdLen = 0; // Current length of buffer contents

// Storage for debug output; Keeps the sensors from being contacted twice per pass
Color lastColor[NUM_SENSORS] = {};   // most recent reading per sensor
bool  haveColor[NUM_SENSORS] = {};   // false until the first reading arrives
bool  sensorsEnabled = true;         // lets you test audio alone via the "sensors" command



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

    // Interfereance pin, for debugging the DAC ripple 
    pinMode(D3, OUTPUT);

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

    // Silence all tracks
    for (uint8_t i = 0; i < NUM_SENSORS; i++){
    idle[i]     = true;    // start silent until a tile is seen
    lastTone[i] = -1;      // nothing playing
}
    
    DTERM_SERIAL.println("Initialization Complete!");
}

// Track playback functions

void silence(uint8_t i){
    if (lastTone[i] >= 0) synthStop(i);
    lastTone[i] = -1;
}

// Update the tracks based on I2C sensor data
void updateTracks(){
    for (uint8_t i = 0; i < NUM_SENSORS; i++){
        if (!sensor_ok[i]) continue;
        if (!(readReg(*RGB_SENSORS[i], REG_STATUS) & 0x01)) continue;  // no new data yet
        Color k = readColor(*RGB_SENSORS[i]);

        // Store for debug output later
        lastColor[i] = k;
        haveColor[i] = true;

        // Brightness gate with hysteresis
        if (idle[i] && k.c > CLEAR_TO_ACTIVE) idle[i] = false;
        if (!idle[i] && k.c < CLEAR_TO_IDLE)  idle[i] = true;
        if (idle[i]) { silence(i); continue; }

        // Saturation gate: bright but colorless (gray/white) is also silence
        float sat;
        float hue = hueFromColor(k, sat);
        if (sat < SAT_MIN) { silence(i); continue; }

        // Only touch the synth when the note changes
        int8_t tone = (int8_t)hueToTone(hue);
        if (tone == lastTone[i]) continue;
        if (lastTone[i] < 0) synthPlay(i, (Tone)tone);
        else                 synthGlideTo(i, (Tone)tone, 150);
        lastTone[i] = tone;
    }
}

// For each of the connected sensors, print readings for debug to USB
// Watch C on black, no tile, and colors
// Watch S on white and grays
void printRGBReadings(){
    DTERM_SERIAL.print("\033[2J\033[H");
    for (int i = 0; i < NUM_SENSORS; i++){
        if (!haveColor[i]) continue;
        const Color &k = lastColor[i];
        float sat;
        float hue = hueFromColor(k, sat);

        DEBUG_SERIAL.print(SENSOR_NAMES[i]);
        DEBUG_SERIAL.print(" C:"); DEBUG_SERIAL.print(k.c);
        DEBUG_SERIAL.print(" R:"); DEBUG_SERIAL.print(k.r);
        DEBUG_SERIAL.print(" G:"); DEBUG_SERIAL.print(k.g);
        DEBUG_SERIAL.print(" B:"); DEBUG_SERIAL.print(k.b);
        DEBUG_SERIAL.print(" H:"); DEBUG_SERIAL.print(hue, 1);
        DEBUG_SERIAL.print(" S:"); DEBUG_SERIAL.print(sat, 2);
        DEBUG_SERIAL.print(" -> ");
        if (idle[i])               DEBUG_SERIAL.println("IDLE (dark)");
        else if (sat < SAT_MIN)    DEBUG_SERIAL.println("SILENT (low sat)");
        else if (lastTone[i] >= 0) DEBUG_SERIAL.println(TONE_NAMES[lastTone[i]]);
        else                       DEBUG_SERIAL.println("--");
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
    else if (strcmp(cmd, "sensors") == 0){
    sensorsEnabled = !sensorsEnabled;
    if (!sensorsEnabled){
        for (uint8_t i = 0; i < NUM_SENSORS; i++) silence(i);
    }
    DTERM_SERIAL.print("sensor-driven tones ");
    DTERM_SERIAL.println(sensorsEnabled ? "on" : "off");
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

    
    
    // Update tracks according to RGB sensor input
    // On a timer as to not bang I2C every pass of loop
    unsigned long now = millis();
    if (sensorsEnabled && (now - lastPoll) >= SENSOR_POLL_MS){
        lastPoll = now;
        updateTracks();
    }
    
    // Handles incoming debug commands
    readSerialCmd();
    
    // Functions within this statement execute periodically for debug output to
    //a PC terminal over USB.
    now = millis(); // Current 'time'
    if (streaming && now - lastOut >= DEBUG_INTERVAL){
        lastOut = now;
        printRGBReadings();
    }
}
