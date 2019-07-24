
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>   // ESP8266WebServer or ESP8266WebServerSecure
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <MQ135.h>              // 83.4 mA, RZERO = 230.00
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>   // 5 mA
#include <Adafruit_BME280.h>    // 0.007 mA
#include "FS.h"                 // set to SPIFFS to MAX before uploading the sketch, or you will lose all .csv files
#include "RTClib.h"             // 1.781 mA
#include <ArduinoJson.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server(80);    // ESP8266WebServer server (80) or ESP8266WebServerSecure server (443)
WebSocketsServer webSocket(81); // create a web socket server on port 81

//--- NTP
IPAddress timeServerIP;         // 129.6.15.27 The time.nist.gov NTP server's IP address

WiFiUDP UDP;                    // Create an instance of the WiFiUDP class to send and receive UDP messages
WiFiClient client;              // Use WiFiClient class to create TCP connections
const int httpPort = 5001;

//--- SPIFFS
String getContentType(String filename);
bool handleFileRead(String path);
File fsUploadFile;                        // a File variable to temporarily store the received file

//--- LCD
#define OLED_RESET D4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

//---
Adafruit_BME280 bme;

//---
#define MQ135_PIN A0
MQ135 gasSensor = MQ135(MQ135_PIN);

//---
RTC_DS3231 rtc;

//---
