/*
 * Module: ESP-NOW Network Node A3
 * Description: Dedicated ESP-NOW mesh node.
 * Peers: Anchor 1, Anchor 2, Tag.
 */

#include <esp_now.h>
#include <WiFi.h>

// --- Network Peers (MAC Addresses) ---
uint8_t mac_T[]  = {0xec, 0x62, 0x60, 0xe4, 0x13, 0x48};
uint8_t mac_A2[] = {0xc8, 0xf0, 0x9e, 0x52, 0x50, 0x94};
uint8_t mac_A1[] = {0xec, 0x62, 0x60, 0xe4, 0x1d, 0xfc};

typedef struct struct_message {
  double Data1;
  double Data2;
  char Data3[32];
} struct_message;

struct_message tx_packet;
struct_message rx_packet;
String incomingSerialLine = "";

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  // Transmission status
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&rx_packet, incomingData, sizeof(rx_packet));
  Serial.printf("ESP:%f,%f,%s;\n", rx_packet.Data1, rx_packet.Data2, rx_packet.Data3);
}

void addPeer(const uint8_t *mac_addr) {
  esp_now_peer_info_t peerInfo = {};
  memcpy(peerInfo.peer_addr, mac_addr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  if (esp_now_add_peer(&peerInfo) != ESP_OK) Serial.println("[Error] Failed to add peer");
}

void parseSerialData(String line) {
  line.trim();
  if (!line.endsWith(";")) return;
  line.remove(line.length() - 1);

  int firstComma = line.indexOf(',');
  int secondComma = line.indexOf(',', firstComma + 1);
  if (firstComma == -1 || secondComma == -1) return;

  tx_packet.Data1 = line.substring(0, firstComma).toFloat();
  tx_packet.Data2 = line.substring(firstComma + 1, secondComma).toFloat();
  line.substring(secondComma + 1).toCharArray(tx_packet.Data3, sizeof(tx_packet.Data3));

  esp_now_send(0, (uint8_t *) &tx_packet, sizeof(tx_packet));
}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  
  if (esp_now_init() != ESP_OK) return;

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(esp_now_recv_cb_t(OnDataRecv));

  addPeer(mac_A1);
  addPeer(mac_A2);
  addPeer(mac_T);
  
  Serial.println("[Node A3] System Ready.");
}

void loop() {
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