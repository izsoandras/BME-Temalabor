#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <DNSServer.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define WiFiTIMEOUT 10000
#define DEFAULT_CONNECT_SSID "AndroidAP"
#define DEFAULT_CONNECT_PWD "android00"
#define AP_SSID "ESP-8266"
#define AP_PWD "12345678"
#define OTA_HOST "ESP8266"
#define OTA_PWD "password"
#define SAMPLE_PERIOD_MS 30000
#define WIFI_SCANNING_PERIOD_MS 30000
#define SEALEVELPRESSURE_HPA (1013.25)

ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
WebSocketsServer sensorSocket(82);
WebSocketsServer rssiSocket(83);
Adafruit_BME280 sensor;
TwoWire twi;

const char* host = "esp8266";
unsigned int timestamp = 0;
unsigned int wifistamp = 0;

File fsUploadFile;

bool ledOn = false;
unsigned long prevMillis = 0, currMillis = 0, periodMillis = SAMPLE_PERIOD_MS, prevWifiMillis = 0, wifiPeriodMillis = WIFI_SCANNING_PERIOD_MS;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, !ledOn);

  Serial.begin(115200);
  delay(10);
  Serial.println();

  Serial.print("MAC: ");
  Serial.println(WiFi.macAddress());

  initOTA();

  initSPIFFS();

  setUpWiFi();

  if (!MDNS.begin(host))
    Serial.println("Error setting up mDNS service!");
  else
    Serial.println("Server available at: http://" + String(host) + ".local/");

  initWebServer();

  initWebSocket();

  initBME280();

  initRSSI();
}

void loop() {
  /*currMillis = millis();
  if (currMillis - prevMillis > periodMillis) {
    logData();
    prevMillis = currMillis;
  }
  if(currMillis - prevWifiMillis > wifiPeriodMillis){
    scanWifi();
    prevWifiMillis = currMillis;
  }*/
  ArduinoOTA.handle();
  server.handleClient();
  webSocket.loop();
  sensorSocket.loop();
  rssiSocket.loop();
}

