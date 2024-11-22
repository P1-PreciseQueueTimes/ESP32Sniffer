#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>

// Function to print MAC address
void printMacAddress(uint8_t *mac) {
  for (int i = 0; i < 6; i++) {
    if (i != 0) Serial.print(":");
    Serial.printf("%02X", mac[i]);
  }
}

// Callback function to handle incoming packets
void promiscuousRxCallback(void *buf, wifi_promiscuous_pkt_type_t type) {
  // Ensure it's a Wi-Fi management frame (e.g., probe request)
  if (type == WIFI_PKT_MGMT) {
    wifi_promiscuous_pkt_t *pkt = (wifi_promiscuous_pkt_t *)buf;
    uint8_t *data = pkt->payload;

    // Get current time in milliseconds
    uint64_t timestamp = esp_timer_get_time(); // Microseconds since boot

    // Check the frame type and subtype
    uint8_t frameType = data[0] & 0x0C; // Type is bits 2-3 of the frame control field
    uint8_t frameSubtype = data[0] & 0xF0; // Subtype is bits 4-7 of the frame control field

    if (frameType == 0x00 && frameSubtype == 0x40) { // 0x00 = Management, 0x40 = Probe Request
      Serial.println("Probe Request Detected!");

      // Print timestamp in microseconds
      Serial.print("Timestamp (us): ");
      Serial.println(timestamp);

      // Extract MAC address (source address is at offset 10)
      Serial.print("Source MAC: ");
      printMacAddress(&data[10]);
      Serial.println();

      // Extract SSID (starts after fixed header, typically at offset 24)
      uint8_t ssidLength = data[25]; // SSID length field
      if (ssidLength > 0 && ssidLength <= 32) {
        Serial.print("SSID: ");
        for (int i = 0; i < ssidLength; i++) {
          Serial.print((char)data[26 + i]); // SSID starts at offset 26
        }
        Serial.println();
      } else {
        Serial.println("SSID: Hidden or Invalid");
      }
      Serial.println();
    }
  }
}

void setup() {
  Serial.begin(115200);

  // Initialize Wi-Fi in station mode (no connection)
  WiFi.mode(WIFI_STA);
  esp_wifi_set_promiscuous(true); // Enable promiscuous mode
  esp_wifi_set_promiscuous_rx_cb(promiscuousRxCallback); // Set callback for packet reception
  esp_wifi_set_channel(13, WIFI_SECOND_CHAN_NONE); // Set Wi-Fi channel

  // Initialize ESP timer
  esp_timer_create_args_t timerConfig = {};
  esp_timer_init();
}

void loop() {
  // Nothing to do here; the callback handles incoming packets.
}
