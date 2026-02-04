/*
 * Project: UWB Anchor Unit 3
 * Description: Fixed anchor point for UWB Localization System.
 * Hardware: ESP32 + DWM1000
 * Specifics: MAC Address ending in E2:9C (Different Prefix), Custom Antenna Delay.
 */

#include <SPI.h>
#include "DW1000Ranging.h"

// --- Configuration ---
// Unique MAC Address for Anchor 3
#define ANCHOR_ADD "65:14:5B:D2:A5:9B:E2:9C"

// Calibrated Antenna Delay (Specific to this hardware unit)
// Original Value from Anchor3.ino: 16577
uint16_t Adelay = 16577;

// --- SPI Pin Definitions (ESP32) ---
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4     // WARNING: Anchor 3 uses Pin 4 for CS!

// --- Control Pins ---
const uint8_t PIN_RST = 27; 
const uint8_t PIN_IRQ = 34; 
const uint8_t PIN_SS = 4;   // Slave Select must match DW_CS

// --- Timing Variables ---
unsigned long lastRangeTime = 0;
const unsigned long TIMEOUT_DURATION = 1000;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    // Initialize SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Initialize DWM1000 Communication
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    
    DW1000Ranging.attachNewRange(newRange);
    delay(97); // Specific delay from original code

    // Configure as Anchor
    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    
    // Apply Antenna Delay Calibration
    DW1000.setAntennaDelay(Adelay);
    DW1000.commitConfiguration();
    
    Serial.println("[Anchor 3] Setup Complete.");
}

void loop()
{
    DW1000Ranging.loop();

    // Watchdog
    if (millis() - lastRangeTime > TIMEOUT_DURATION) {
        resetDW1000();
    }
}

void newRange()
{
    lastRangeTime = millis();
}

void resetDW1000() {
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    delay(97);
    DW1000Ranging.attachNewRange(newRange);

    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    DW1000.setAntennaDelay(Adelay);
    DW1000.commitConfiguration();

    lastRangeTime = millis();
    Serial.println("[Anchor 3] Reset Triggered");
}