#include "EspConnectionConfig.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WebServer.h>
#include "EepromEsp.h"


EspConnectionConfig::EspConnectionConfig(ESP8266WebServer* server){
  this->estore = EepromEsp();
  this->ssid = "";
  this->passphrase = "";
}



void EspConnectionConfig::updateNetworkList() {
  delay(400);
  int n = WiFi.scanNetworks();
  this->st = "<label for='ssid'>Select network</label><select id='ssid' name='ssid'>";
  for (int i = 0; i < n; ++i) {
    // Print SSID and RSSI for each network found
    this->st += "<option value='"+WiFi.SSID(i)+"'>";
    this->st += WiFi.SSID(i);
    this->st += " (";
    this->st += WiFi.RSSI(i);

    this->st += ")";
    this->st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    this->st += "</option>";
  }
  this->st += "</select>";
  delay(300);
}

void EspConnectionConfig::setupAP() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  WiFi.softAP("EspColdBootAP", "");
  this->launchWeb();
};


bool EspConnectionConfig::testWifi() {
  int c = 0;
  while (c < 20) {
    if (WiFi.status() == WL_CONNECTED) {
      return true;
    }
    delay(500);
    c++;
  }
  return false;
};

void EspConnectionConfig::launchWeb() {
  this->createWebServer();
  this->server.begin();
}

void EspConnectionConfig::createWebServer() {
  this->server.on("/", [this]() {
   this->updateNetworkList();
    String content = "<!DOCTYPE html>\r\n<html><head><meta charset='UTF-8'><meta http-equiv='X-UA-Compatible' content='IE=edge'><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>Настройка подключения</title></head>";
    content += "<style>.container{max-width: 600px;margin:0 auto;} body{font-family: Arial;background-color:#f7f7f7}label{display: block;margin-bottom: 0.5rem;} select,input{ width:100%;margin-bottom: 1.5rem;}</style>";
    content += "<body><div class='container'>";
    content += "<h1 style='margin-bottom: 3rem;font-size: 1.7rem; text-align:center;'>Настройка подключения контроллера к wi-fi</h1>";
    content += "<form style='margin-bottom: 2rem;' action=\"/\" method=\"POST\"><input type=\"submit\" value=\"Сканировать\"></form>";
    content += "<form method='get' action='setting'>";
    content += st;
    content += "<label for='password'>Password for WiFi</label><input id='password' name='pass' placeholder='Input password' length=64><input type='submit'></form>";
    content += "</div></body>";
    content += "</html>";
    server.send(200, "text/html", content);
  });
  this->server.on("/setting", [this]() {
    String qsid =  this->server.arg("ssid");
    String qpass = this->server.arg("pass");
    
    this->estore.saveSettings(qsid,qpass);
    String content = "{\"Success\":\"saved to eeprom... reset to boot into new wifi\"}";
    this->server.send(200, "application/json", content);
    delay(500);
    ESP.restart();

  });
};

void EspConnectionConfig::setup() {
  WiFi.disconnect();
  delay(10);
  this->estore.readSettings();
  WiFi.begin(this->estore.esid, this->estore.epass);
  delay(100);

  bool isConnected = this->testWifi();
  if (isConnected) {
   WiFi.mode(WIFI_STA);
  }
  else{

  this->setupAP();  // Setup HotSpot

  while ((WiFi.status() != WL_CONNECTED)) {
    delay(100);
    this->server.handleClient();
  }
  }
};
