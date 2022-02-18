#ifndef EspConnectionConfig_H
#define EspConnectionConfig_H

#include "EepromEsp.h"
#include <ESP8266WebServer.h>
//#include "JsonConfig.h"


class EspConnectionConfig
{
  private:
    int i = 0;
    int statusCode;
    const char* ssid;
    const char* passphrase;
    EepromEsp estore;
    String st;
    String content;
    ESP8266WebServer server;
    void updateNetworkList();
    void setupAP();
    void launchWeb();
    void createWebServer();
    
  public:
    EspConnectionConfig(ESP8266WebServer* server);
    bool testWifi();
    void setup();
};

#endif