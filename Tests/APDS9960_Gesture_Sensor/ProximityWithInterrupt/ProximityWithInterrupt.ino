/****************************************************************
ProximityInterrupt.ino
APDS-9960 RGB and Gesture Sensor
Shawn Hymel @ SparkFun Electronics
October 24, 2014
https://github.com/sparkfun/APDS-9960_RGB_and_Gesture_Sensor

Tests the proximity interrupt abilities of the APDS-9960.
Configures the APDS-9960 over I2C and waits for an external
interrupt based on high or low proximity conditions. Move your
hand near the sensor and watch the LED on pin 13.

Hardware Connections:

IMPORTANT: The APDS-9960 can only accept 3.3V!
 
Wemos Pin  APDS-9960 Board  Function
 
 3.3V         VCC              Power
 GND          GND              Ground
 D3           SDA              I2C Data
 D1           SCL              I2C Clock
 D6           INT              Interrupt
 D7           -                LED

Resources:
Include Wire.h and SparkFun_APDS-9960.h

Development environment specifics:
Written in Arduino 1.0.5
Tested with SparkFun Arduino Pro Mini 3.3V

This code is beerware; if you see me (or any other SparkFun 
employee) at the local, and you've found our code helpful, please
buy us a round!

Distributed as-is; no warranty is given.

Modified for ESP8266 by Jon Ulmer Nov 2016
****************************************************************/

#include <Wire.h>
#include "SparkFun_APDS9960_ESP8266.h"

// Pins on wemos D1 mini
#define APDS9960_INT    D6  //AKA GPIO12 -- Interupt pin
#define APDS9960_SDA    D2  //AKA GPIO0
#define APDS9960_SCL    D1  //AKA GPIO5
#define LED_PIN         D4  //AKA GPIO13 -- LED for showing interrupt

// Constants
#define PROX_INT_HIGH   50 // Proximity level for interrupt
#define PROX_INT_LOW    0  // No far interrupt

// Global variables
SparkFun_APDS9960 apds = SparkFun_APDS9960();
uint8_t proximity_data = 0;
volatile bool isr_flag = 0;

void setup() {

  //Start I2C with pins defined above
  Wire.begin(APDS9960_SDA,APDS9960_SCL);
  
  // Set LED as output
  pinMode(LED_PIN, OUTPUT);
  pinMode(APDS9960_INT, INPUT);
  
  // Initialize Serial port
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("---------------------------------------"));
  Serial.println(F("SparkFun APDS-9960 - ProximityInterrupt"));
  Serial.println(F("---------------------------------------"));
  
  // Initialize interrupt service routine
  attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
  
  // Initialize APDS-9960 (configure I2C and initial values)
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }
  
  // Adjust the Proximity sensor gain
  if ( !apds.setProximityGain(PGAIN_2X) ) {
    Serial.println(F("Something went wrong trying to set PGAIN"));
  }
  
  // Set proximity interrupt thresholds
  if ( !apds.setProximityIntLowThreshold(PROX_INT_LOW) ) {
    Serial.println(F("Error writing low threshold"));
  }
  if ( !apds.setProximityIntHighThreshold(PROX_INT_HIGH) ) {
    Serial.println(F("Error writing high threshold"));
  }
  
  // Start running the APDS-9960 proximity sensor (interrupts)
  if ( apds.enableProximitySensor(true) ) {
    Serial.println(F("Proximity sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during sensor init!"));
  }
}

void loop() {
  
  // If interrupt occurs, print out the proximity level
  if ( isr_flag == 1 ) {
  
    // Read proximity level and print it out
    if ( !apds.readProximity(proximity_data) ) {
      Serial.println("Error reading proximity value");
    } else {
      Serial.print("Proximity detected! Level: ");
      Serial.println(proximity_data);
    }
    
    // Turn on LED for a half a second
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    
    // Reset flag and clear APDS-9960 interrupt (IMPORTANT!)
    isr_flag = 0;
    if ( !apds.clearProximityInt() ) {
      Serial.println("Error clearing interrupt");
    }
    
  }
}

void interruptRoutine() {
  isr_flag = 1;
}
