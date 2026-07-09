#include "WiFiConnection.h"

WiFiConnection::WiFiConnection(
    const char* ssid,
    const char* password)
    : ssid_(ssid),
      password_(password) {
}

bool WiFiConnection::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname("gridmind-node-b");
  WiFi.begin(ssid_, password_);
  WiFi.setAutoReconnect(true);

  const unsigned long connectionStarted = millis();
  const unsigned long timeoutMs = 15000;

  while (WiFi.status() != WL_CONNECTED &&
         millis() - connectionStarted < timeoutMs) {
    delay(500);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool WiFiConnection::isConnected() const {
  return WiFi.status() == WL_CONNECTED;
}

IPAddress WiFiConnection::address() const {
  return WiFi.localIP();
}
