/*
 * Module: ESP-NOW Network Node A1
 * Description: Dedicated ESP-NOW mesh node.
 * Role: Reads serial input, broadcasts to mesh, and logs received peer data.
 * Peers: Anchor 2, Anchor 3, Tag.
 */

#include <esp_now.h>
#include <WiFi.h>

// --- Network Peers (MAC Addresses) ---
// Update these if hardware changes!
uint8_t mac_A2[] = {0xc8, 0xf0, 0x9e, 0x52, 0x50, 0x94};
uint8_t mac_A3[] = {0xec, 0x62, 0x60, 0xe3, 0x87, 0x9c};
uint8_t mac_T[]  = {0xec, 0x62, 0x60, 0xe4, 0x13, 0x48};

// --- Data Structures ---
typedef struct struct_message {
  double Data1;
  double Data2;
  char Data3[32];
} struct_message;

struct_message tx_packet; // Data to transmit
struct_message rx_packet; // Data received

String incomingSerialLine = "";

// --- Callbacks ---

// Callback: Transmission Status
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Optional debug: Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback: Data Reception
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rx_packet, incomingData, sizeof(rx_packet));
  // Print received data in CSV format: "ESP:d1,d2,msg;"
  Serial.printf("ESP:%f,%f,%s;\n", rx_packet.Data1, rx_packet.Data2, rx_packet.Data3);
}

// Helper: Add a peer to the ESP-NOW list
void addPeer(const uint8_t *mac_addr) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_addr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("[Error] Failed to add peer");
  }
}

// Helper: Parse Serial Input (Format: "d1,d2,msg;")
void parseSerialData(String line) {
  line.trim();
  if (!line.endsWith(";")) return; 
  line.remove(line.length() - 1); // Remove semicolon

  int firstComma = line.indexOf(',');
  int secondComma = line.indexOf(',', firstComma + 1);

  if (firstComma == -1 || secondComma == -1) return;

  String d1 = line.substring(0, firstComma);
  String d2 = line.substring(firstComma + 1, secondComma);
  String d3 = line.substring(secondComma + 1);

  tx_packet.Data1 = d1.toFloat();
  tx_packet.Data2 = d2.toFloat();
  d3.toCharArray(tx_packet.Data3, sizeof(tx_packet.Data3));
  
  // Broadcast the parsed data to all peers
  esp_now_send(0, (uint8_t *) &tx_packet, sizeof(tx_packet));
}

void setup() {
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  
  // Initialize ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("[Error] ESP-NOW Init Failed");
    return;
  }

  // Register Callbacks
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv)); // Cast required for some ESP32 cores

  // Register Peers
  addPeer(mac_A2);
  addPeer(mac_A3);
  addPeer(mac_T);
  
  Serial.println("[Node A1] System Ready.");
}

void loop() {
  // Non-blocking Serial Read
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      parseSerialData(incomingSerialLine);
      incomingSerialLine = "";
    } else {
      incomingSerialLine += c;
    }
  }
}