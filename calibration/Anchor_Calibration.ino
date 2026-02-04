/*
 * Tool: Antenna Delay Calibration (Auto-Tuner)
 * Description: Automatically iterates through antenna delay values to find 
 * the one that matches a known fixed distance (Reference Distance).
 */

#include <SPI.h>
#include "DW1000Ranging.h"
#include "DW1000.h"

// --- Hardware Pins ---
#define SPI_SCK 18
#define SPI_MISO 19
#define SPI_MOSI 23
#define DW_CS 5
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;
const uint8_t PIN_SS = 5;

// --- Calibration Settings ---
char this_anchor_addr[] = "84:00:22:EA:82:60:3B:80"; // Temp Address
float target_distance = 10.0;     // TRUE distance to tag (meters)
float tolerance = 0.01;           // Acceptable error (meters)
float stability_threshold = 0.02; // Max standard deviation

// Scan Range
const uint16_t ADELAY_START = 16300;
const uint16_t ADELAY_END = 17000;
const uint16_t ADELAY_STEP = 10;   // Increased step for speed, refine manually later

uint16_t current_adelay = ADELAY_START;
bool testing_stability = false;
unsigned long stability_test_start = 0;
const unsigned long STABILITY_DURATION = 5000; 

// Statistical Storage
const int MAX_SAMPLES = 100;
float stability_samples[MAX_SAMPLES];
int stability_index = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial);
  
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  
  DW1000Ranging.attachNewRange(newRange);
  
  // Start as Anchor
  DW1000Ranging.startAsAnchor(this_anchor_addr, DW1000.MODE_LONGDATA_RANGE_LOWPOWER, false);
  
  // Set initial delay
  DW1000.setAntennaDelay(current_adelay);
  DW1000.commitConfiguration();

  Serial.println("Calibration Sequence Started...");
  Serial.print("Target Distance: "); Serial.println(target_distance);
}

void loop() {
  DW1000Ranging.loop();
}

void newRange() {
  float measured_dist = DW1000Ranging.getDistantDevice()->getRange();
  float error = abs(measured_dist - target_distance);

  // Phase 2: Stability Test (If accurate enough, check precision)
  if (testing_stability) {
    if (stability_index < MAX_SAMPLES) {
      stability_samples[stability_index++] = measured_dist;
    }

    if (millis() - stability_test_start >= STABILITY_DURATION) {
      // Calculate Mean and StdDev
      float sum = 0, mean, stddev = 0;
      for (int i = 0; i < stability_index; i++) sum += stability_samples[i];
      mean = sum / stability_index;
      
      for (int i = 0; i < stability_index; i++) stddev += pow(stability_samples[i] - mean, 2);
      stddev = sqrt(stddev / stability_index);

      Serial.print("Stability Result -> Mean: "); Serial.print(mean);
      Serial.print(" StdDev: "); Serial.println(stddev);

      if (abs(mean - target_distance) < tolerance && stddev < stability_threshold) {
        Serial.println(">>> CALIBRATION SUCCESSFUL <<<");
        Serial.print("Optimal Adelay: "); Serial.println(current_adelay);
        while (1); // Stop
      } else {
        // Failed stability, keep searching
        testing_stability = false;
        increment_adelay();
      }
    }
    return;
  }

  // Phase 1: Accuracy Check
  if (error < tolerance) {
    Serial.print("Potential Adelay found: "); Serial.println(current_adelay);
    Serial.println("Starting Stability Test...");
    testing_stability = true;
    stability_test_start = millis();
    stability_index = 0;
  } else {
    increment_adelay();
  }
}

void increment_adelay() {
    current_adelay += ADELAY_STEP;
    if (current_adelay > ADELAY_END) {
      Serial.println("Calibration Failed: Range exceeded.");
      while (1);
    }
    DW1000.setAntennaDelay(current_adelay);
    DW1000.commitConfiguration();
    Serial.print("Testing Adelay: "); Serial.println(current_adelay);
}