/*
 * Project: UWB Anchor Unit 1
 * Description: Fixed anchor point for UWB Localization System using Decawave DWM1000.
 * Hardware: ESP32 + DWM1000
 */

#include <SPI.h>
#include "DW1000Ranging.h"

// --- Configuration ---
// Unique MAC Address for this Anchor (Change for Unit 2/3)
#define ANCHOR_ADD "72:11:5B:D3:B9:2A:H2:9C" 

// Calibrated Antenna Delay (Specific to this hardware unit)
// Unit 1: 16555 | Unit 2: 16560 | Unit 3: 16577
uint16_t Adelay = 16555; 

// --- SPI Pin Definitions (ESP32) ---
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 4     // Chip Select (Check hardware!)

// --- Control Pins ---
const uint8_t PIN_RST = 27; // Reset
const uint8_t PIN_IRQ = 34; // Interrupt
const uint8_t PIN_SS = 4;   // Slave Select

// --- Timing Variables ---
unsigned long lastRangeTime = 0;
const unsigned long TIMEOUT_DURATION = 1000; // 1 second timeout

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    // Initialize SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Initialize DWM1000 Communication
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    
    // Attach Callback
    DW1000Ranging.attachNewRange(newRange);
    delay(7);

    // Configure as Anchor
    // Mode: LONGDATA_RANGE_LOWPOWER provides good range/power balance
    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    
    // Apply Antenna Delay Calibration
    DW1000.setAntennaDelay(Adelay);
    DW1000.commitConfiguration();
    
    Serial.println("[Anchor 1] Setup Complete. Waiting for Tags...");
}

void loop()
{
    DW1000Ranging.loop();

    // Watchdog: Reset module if no activity for too long
    if (millis() - lastRangeTime > TIMEOUT_DURATION) {
        resetDW1000();
    }
}

void newRange()
{
    // Update heartbeat timer when a valid range is received
    lastRangeTime = millis();
    
    // Optional: Print details for debugging
    // Serial.print("Tag Connected: ");
    // Serial.println(DW1000Ranging.getDistantDevice()->getShortAddress());
}

void resetDW1000() {
    // Re-initialize the module in case of freeze
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    DW1000Ranging.attachNewRange(newRange);
    delay(7);

    DW1000Ranging.startAsAnchor(ANCHOR_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    DW1000.setAntennaDelay(Adelay);
    DW1000.commitConfiguration();

    lastRangeTime = millis();
    Serial.println("[System] Watchdog Reset Triggered");
}
📄 src/Tag_Main.ino (Eski Tag.ino)
Burada "range1, range2" gibi hardcoded kısımları temizledim ve CSV formatında çıktı verecek şekilde (Python kodunla uyumlu olması için) düzenledim.

C++
/*
 * Project: UWB Tag Unit (Mobile Drone Node)
 * Description: Mobile tag that ranges with fixed anchors.
 * Output: Serial CSV format (d1, d2, d3) for external processing.
 */

#include <SPI.h>
#include "DW1000Ranging.h"

// --- Configuration ---
#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"

// --- SPI Pins ---
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 5     // Note: Tag uses Pin 5 for CS in your setup

const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 5;

// --- Ranging Variables ---
// Map Short Addresses of Anchors to Array Indices
// Example Short Addresses (Decimal): Anchor1=4466, Anchor2=6022, Anchor3=5221
// Note: You must verify these Short Addresses from the Anchor serial output!
double ranges[3] = {0.0, 0.0, 0.0}; 

unsigned long lastRangeTime = 0;
const unsigned long TIMEOUT_DURATION = 1000; 

void setup()
{
    Serial.begin(115200);
    delay(1000);

    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    
    DW1000Ranging.attachNewRange(newRange);
    
    // Enable filter for smoother data
    DW1000Ranging.useRangeFilter(true);

    // Start as Tag
    DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    Serial.println("[Tag] Initialized.");
}

void loop()
{
    DW1000Ranging.loop();

    if (millis() - lastRangeTime > TIMEOUT_DURATION) {
        resetDW1000();
    }
}

void newRange()
{
    lastRangeTime = millis();
    
    // Get the short address of the anchor we just ranged with
    int anchor_addr = DW1000Ranging.getDistantDevice()->getShortAddress();
    float dist = DW1000Ranging.getDistantDevice()->getRange();

    // Map specific Anchor IDs to array slots
    // UPDATE THESE IDs based on your actual Anchor Short Addresses
    if(anchor_addr == 4466) {      // Anchor 1
        ranges[0] = dist;
    } else if(anchor_addr == 6022) { // Anchor 2
        ranges[1] = dist;
    } else if(anchor_addr == 5221) { // Anchor 3
        ranges[2] = dist;
    }

    // Output formatted CSV for Python script: "d1, d2, d3"
    Serial.printf("%.3f, %.3f, %.3f\n", ranges[0], ranges[1], ranges[2]);
}

void resetDW1000() {
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.useRangeFilter(true);
    DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    lastRangeTime = millis();
}