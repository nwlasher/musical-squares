// This project is a music box

// Initialization and pinmappings
// T

#include <Arduino.h>
#include "rgbTCS34725.hpp"

#define BAUD_RATE 115200 // Serial UART baud rate to PC for debug output

void setup(){
    delay(500); // Startup settle delay (So I don't miss debug output)
    Serial.begin(BAUD_RATE);
    for (int i = 0; i < 1; i++){ // Increase this to tile sensors
        Serial.print("Sensor "); Serial.print(sensor_names[i]);
        Serial.println(initSensor(rgb_sensors[i]) ? " OK" : " NOT FOUND");
    }
    
    delay(5); // Wait
    Serial.println("Initialization Complete!");
}


// Main program loop
void loop(){

    // Simple polling loop to debug hardware connections to UART
    for (int i = 0; i < 3; i++) {
    if (readReg(*rgb_sensors[i], REG_STATUS) & 0x01) {
      Color k = readColors(*rgb_sensors[i]);
      Serial.print(sensor_names[i]); // Name
      Serial.print(" C:"); Serial.print(k.c); // Values
      Serial.print(" R:"); Serial.print(k.r);
      Serial.print(" G:"); Serial.print(k.g);
      Serial.print(" B:"); Serial.println(k.b);
    }
  }
  delay(60);
}
