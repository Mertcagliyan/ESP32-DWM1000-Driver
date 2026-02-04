/*
 * Project: Calibration Reference Tag
 * Description: Acts as a static reference target for calibrating Anchors.
 * Place this Tag at a known fixed distance (e.g., 10.0m) from the Anchor
 * running the 'Anchor_Calibration' script.
 * Hardware: ESP32 + DWM1000
 */

#include <SPI.h>
#include "DW1000Ranging.h"

// --- Configuration ---
// Unique MAC Address for this Reference Tag
// Ensure this address matches what the Anchor expects if filtering is enabled.
char tag_addr[] = "7D:00:22:EA:82:60:3B:9C";

// --- SPI Pin Definitions (ESP32) ---
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 5     // Chip Select set to Pin 5

// --- Control Pins ---
const uint8_t PIN_RST = 27; // Reset
const uint8_t PIN_IRQ = 34; // Interrupt
const uint8_t PIN_SS = 5;   // Slave Select

void setup()
{
    Serial.begin(115200);
    delay(1000);
    
    // Initialize SPI
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    
    // Initialize DWM1000 Communication
    DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
    
    // Attach Callbacks to monitor connectivity
    DW1000Ranging.attachNewRange(newRange);
    DW1000Ranging.attachNewDevice(newDevice);
    DW1000Ranging.attachInactiveDevice(inactiveDevice);

    // Start as Tag
    // Mode: LONGDATA_RANGE_LOWPOWER gives best stability for calibration
    DW1000Ranging.startAsTag(tag_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
    
    Serial.println("[Ref Tag] Calibration Target Initialized.");
    Serial.print("Address: "); Serial.println(tag_addr);
}

void loop()
{
    DW1000Ranging.loop();
}

// --- Callback Functions ---

void newRange()
{
    // This function is called every time a range is computed.
    // We print it to monitor stability on the Tag side as well.
    Serial.print("From: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
    Serial.print("\t Range: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRange());
    Serial.print(" m\t RX Power: ");
    Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
    Serial.println(" dBm");
}

void newDevice(DW1000Device* device)
{
    Serial.print("[Event] New Device Detected: ");
    Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device* device)
{
    Serial.print("[Event] Device Inactive: ");
    Serial.println(device->getShortAddress(), HEX);
}