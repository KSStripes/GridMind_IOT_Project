#ifndef GRIDMIND_WIFI_CONNECTION_H
#define GRIDMIND_WIFI_CONNECTION_H

#include <ESP8266WiFi.h>

class WiFiConnection {
 public:
  WiFiConnection(const char* ssid, const char* password);

  bool begin();
  bool isConnected() const;
  IPAddress address() const;

 private:
  const char* ssid_;
  const char* password_;
};

#endif
