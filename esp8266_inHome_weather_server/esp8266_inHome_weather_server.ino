#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h> // ESP8266WebServer or ESP8266WebServerSecure
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <WebSocketsServer.h>
#include <Wire.h>
#include <DHT.h>
#include <DHT_U.h>
#include <MQ135.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_BME280.h>
#include <FS.h>
#include "RTClib.h"

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server (80);   // ESP8266WebServer server (80) or ESP8266WebServerSecure server (443)
WebSocketsServer webSocket(81); // create a websocket server on port 81

IPAddress timeServerIP;   // 129.6.15.27 The time.nist.gov NTP server's IP address
unsigned int localPort = 123; // 123 or 2390
const char* ntpServerName = "time.nist.gov"; // time.nist.gov, time.windows.com, time.google.com
const int NTP_PACKET_SIZE = 48;           // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];       // A buffer to hold incoming and outgoing packets
WiFiUDP UDP;      // Create an instance of the WiFiUDP class to send and receive UDP messages
#define ONE_HOUR 3600000UL
const unsigned long intervalNTP = ONE_HOUR;       // Update the time every hour
unsigned long prevNTP = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;        // The most recent timestamp received from the time server

const unsigned long sensorsRequestPeriod = 60000; // Do a temperature measurement every minute
unsigned long sensorsRequest = 0;
bool tmpRequested = false;
const unsigned long DS_delay = 750;       // Reading the temperature from the DS18x20 can take up to 750ms

const char *ssid_1 = "suslik9282";
const char *password_1 = "3M0l4@09";
const char *ssid_2 = "suslik928";
const char *password_2 = "08022403";

String getContentType(String filename); // SPIFFS
bool handleFileRead(String path);       // SPIFFS
File fsUploadFile;      // a File variable to temporarily store the received file

const char *OTAName = "ESP8266";          // A name and a password for the OTA service
const char *OTAPassword = "espP0v0zdyxy";

const char* mdnsName = "esp8266";         // Domain name for the mDNS responder

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

Adafruit_BME280 bme; // I2C

#define DHTPIN 2
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

#define MQ135_PIN A0
MQ135 gasSensor = MQ135(MQ135_PIN);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

unsigned long sensorsUpdateMillis = 0;  // sensors udate
const int sensorsUpdatePeriod = 5000;  // sensors update
unsigned long timeUpdateMillis = 0; // rtc upd

float t1 = -1, h1 = -1, a = -1, t2 = -1, h2 = -1, p = -1;
bool show = true;

// The certificate is stored in PMEM
static const uint8_t x509[] PROGMEM = {
  0x30, 0x82, 0x01, 0x4b, 0x30, 0x81, 0xf6, 0x02, 0x09, 0x00, 0xee, 0x2b,
  0x15, 0xcc, 0x46, 0xbc, 0xc7, 0xc8, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86,
  0x48, 0x86, 0xf7, 0x0d, 0x01, 0x01, 0x0b, 0x05, 0x00, 0x30, 0x2d, 0x31,
  0x17, 0x30, 0x15, 0x06, 0x03, 0x55, 0x04, 0x0a, 0x0c, 0x0e, 0x79, 0x6f,
  0x75, 0x72, 0x2d, 0x6e, 0x61, 0x6d, 0x65, 0x2d, 0x68, 0x65, 0x72, 0x65,
  0x31, 0x12, 0x30, 0x10, 0x06, 0x03, 0x55, 0x04, 0x03, 0x0c, 0x09, 0x31,
  0x32, 0x37, 0x2e, 0x30, 0x2e, 0x30, 0x2e, 0x31, 0x30, 0x1e, 0x17, 0x0d,
  0x31, 0x38, 0x30, 0x36, 0x30, 0x33, 0x31, 0x31, 0x34, 0x32, 0x35, 0x34,
  0x5a, 0x17, 0x0d, 0x33, 0x32, 0x30, 0x32, 0x31, 0x30, 0x31, 0x31, 0x34,
  0x32, 0x35, 0x34, 0x5a, 0x30, 0x2d, 0x31, 0x17, 0x30, 0x15, 0x06, 0x03,
  0x55, 0x04, 0x0a, 0x0c, 0x0e, 0x79, 0x6f, 0x75, 0x72, 0x2d, 0x6e, 0x61,
  0x6d, 0x65, 0x2d, 0x68, 0x65, 0x72, 0x65, 0x31, 0x12, 0x30, 0x10, 0x06,
  0x03, 0x55, 0x04, 0x03, 0x0c, 0x09, 0x31, 0x32, 0x37, 0x2e, 0x30, 0x2e,
  0x30, 0x2e, 0x31, 0x30, 0x5c, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48,
  0x86, 0xf7, 0x0d, 0x01, 0x01, 0x01, 0x05, 0x00, 0x03, 0x4b, 0x00, 0x30,
  0x48, 0x02, 0x41, 0x00, 0xc6, 0xb4, 0x2a, 0x42, 0xbf, 0xba, 0x2b, 0xf6,
  0xf6, 0xf0, 0xa3, 0xa7, 0x2d, 0xe6, 0xaf, 0xd8, 0x88, 0x74, 0x78, 0xd0,
  0x6e, 0x01, 0xf4, 0x24, 0xda, 0x96, 0x1d, 0x43, 0xc6, 0x1b, 0x45, 0xd5,
  0xd2, 0xda, 0xe6, 0xec, 0xd4, 0xb5, 0x9e, 0xea, 0x0f, 0x9a, 0xc1, 0x89,
  0x17, 0x96, 0xb9, 0x0f, 0x8b, 0x2a, 0x8f, 0x4b, 0x40, 0x95, 0xae, 0xdc,
  0x33, 0x10, 0x3d, 0xca, 0x7a, 0x5b, 0x9d, 0xc3, 0x02, 0x03, 0x01, 0x00,
  0x01, 0x30, 0x0d, 0x06, 0x09, 0x2a, 0x86, 0x48, 0x86, 0xf7, 0x0d, 0x01,
  0x01, 0x0b, 0x05, 0x00, 0x03, 0x41, 0x00, 0x6c, 0x7e, 0x21, 0xd7, 0x31,
  0xe7, 0x34, 0xf2, 0x61, 0xd6, 0x0b, 0x6f, 0x1a, 0xae, 0x36, 0x07, 0x96,
  0xf2, 0x67, 0x44, 0x8e, 0xcb, 0x5a, 0x6c, 0xb0, 0x3d, 0x4b, 0x8d, 0xdd,
  0x4c, 0x06, 0xdb, 0x02, 0xa7, 0x16, 0x4a, 0x1f, 0x0e, 0x43, 0xa8, 0x3b,
  0xb4, 0xce, 0x29, 0x4f, 0x3c, 0x46, 0x62, 0x0b, 0x0f, 0xc3, 0xb7, 0x69,
  0x69, 0x80, 0x5e, 0x7e, 0xb6, 0x77, 0x4c, 0x5a, 0x98, 0xed, 0xd4
};

// And so is the key.  These could also be in DRAM
static const uint8_t rsakey[] PROGMEM = {
  0x30, 0x82, 0x01, 0x39, 0x02, 0x01, 0x00, 0x02, 0x41, 0x00, 0xc6, 0xb4,
  0x2a, 0x42, 0xbf, 0xba, 0x2b, 0xf6, 0xf6, 0xf0, 0xa3, 0xa7, 0x2d, 0xe6,
  0xaf, 0xd8, 0x88, 0x74, 0x78, 0xd0, 0x6e, 0x01, 0xf4, 0x24, 0xda, 0x96,
  0x1d, 0x43, 0xc6, 0x1b, 0x45, 0xd5, 0xd2, 0xda, 0xe6, 0xec, 0xd4, 0xb5,
  0x9e, 0xea, 0x0f, 0x9a, 0xc1, 0x89, 0x17, 0x96, 0xb9, 0x0f, 0x8b, 0x2a,
  0x8f, 0x4b, 0x40, 0x95, 0xae, 0xdc, 0x33, 0x10, 0x3d, 0xca, 0x7a, 0x5b,
  0x9d, 0xc3, 0x02, 0x03, 0x01, 0x00, 0x01, 0x02, 0x40, 0x2e, 0x2f, 0x78,
  0x57, 0x35, 0x7b, 0x4f, 0x76, 0xc2, 0xe5, 0x22, 0x14, 0xdc, 0xbd, 0x6f,
  0x08, 0x14, 0x87, 0x86, 0x01, 0x64, 0x60, 0x7d, 0xe0, 0x07, 0x12, 0xb7,
  0x50, 0xa6, 0x7e, 0x76, 0x57, 0x99, 0x23, 0x62, 0xf8, 0x18, 0x0a, 0x54,
  0x10, 0x13, 0xbe, 0x7f, 0x58, 0xdd, 0xc0, 0x76, 0xab, 0xcf, 0x01, 0xb3,
  0xf6, 0xa9, 0x34, 0xbb, 0xd2, 0xe1, 0x6b, 0xf0, 0xba, 0x2f, 0xb7, 0x5d,
  0x79, 0x02, 0x21, 0x00, 0xe5, 0x54, 0x83, 0x3d, 0x46, 0x55, 0xb3, 0x40,
  0xd0, 0xec, 0x30, 0x7b, 0xae, 0xd9, 0x71, 0xc2, 0x99, 0x88, 0x64, 0xc5,
  0x9b, 0xb8, 0x4c, 0xec, 0x7d, 0x4d, 0x00, 0x10, 0xc1, 0x69, 0x9a, 0xb5,
  0x02, 0x21, 0x00, 0xdd, 0xcf, 0xdc, 0x9c, 0x65, 0xc8, 0x32, 0x9b, 0x8d,
  0x30, 0x27, 0x38, 0xa5, 0xf9, 0x37, 0x90, 0xe0, 0xab, 0x01, 0xa0, 0xbb,
  0x58, 0x14, 0x73, 0xbf, 0x65, 0x31, 0xc1, 0xfb, 0xbd, 0x09, 0x97, 0x02,
  0x20, 0x0f, 0x22, 0x5d, 0xaa, 0xdc, 0x8a, 0x69, 0x52, 0xfc, 0xb6, 0xa9,
  0x98, 0x6f, 0xf8, 0x8f, 0xa6, 0x5d, 0x05, 0x80, 0x48, 0xfe, 0x36, 0xd2,
  0x74, 0xc4, 0xec, 0x20, 0xf5, 0xfa, 0x1d, 0xa3, 0x05, 0x02, 0x20, 0x24,
  0x59, 0x90, 0x19, 0xc1, 0x44, 0x51, 0xa6, 0xd4, 0x38, 0x73, 0xf5, 0x00,
  0x9e, 0x1a, 0xd1, 0x0f, 0x40, 0x76, 0xff, 0xd4, 0x33, 0xd7, 0x06, 0x30,
  0xf3, 0x70, 0x78, 0x0d, 0xbd, 0x28, 0xf9, 0x02, 0x20, 0x64, 0x20, 0x1a,
  0x9f, 0xe5, 0xd1, 0x75, 0x3c, 0xdb, 0xf9, 0x76, 0x22, 0x73, 0xcc, 0x6f,
  0xed, 0x3f, 0xef, 0x31, 0xd0, 0xe4, 0xf4, 0xbb, 0x4b, 0x44, 0xaf, 0x8c,
  0x98, 0x84, 0xad, 0x16, 0x3f
};

void setup ( void ) {

  Serial.begin(115200);
  delay(10);
  Serial.println('\r\n');
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite (LED_BUILTIN, LOW);

  // actions with display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  display.display();
  //delay(10);
  display.clearDisplay(); // Clear the buffer.
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("Hello, world!");
  display.setTextColor(BLACK, WHITE); // 'inverted' text
  display.println(3.141592);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print("0x");
  display.println(0xDEADBEEF, HEX);
  display.display();

  bme.begin(0x76);
  dht.begin();
  rtc.begin();  // start clock
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  startWiFi();      // Start a Wi-Fi access point, and try to connect to some given access points. Then wait for either an AP or STA connection
  startOTA();       // Start the OTA service
  startSPIFFS();      // Start the SPIFFS and list all contents
  startMDNS();      // Start the mDNS responder
  startServer();      // Start a HTTP server with a file read handler and an upload handler
  startUDP();       // Start listening for UDP messages to port 123

  WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);
  sendNTPpacket(timeServerIP);
  delay(500);

  server.begin();
  Serial.println("HTTPS server started");
}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop (void) {

  DateTime now = rtc.now();
  unsigned long currentMillis = millis();

  if (currentMillis - prevNTP > intervalNTP) {    // Request the time from the time server every hour
    prevNTP = currentMillis;
    WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
    Serial.println("Time server IP:\t" + (String)timeServerIP);
    sendNTPpacket(timeServerIP);

    uint32_t time = getTime();      // Check if the time server has responded, if so, get the UNIX time
    if (time) {
      timeUNIX = time;
      //rtsTime = rtc.adjust(DateTime(....));   // TODO !!!
      //Serial.println("RTC Update");
      Serial.println("NTP response:\t" + (String)timeUNIX);
      lastNTPResponse = millis();
    } else if ((millis() - lastNTPResponse) > 24UL * ONE_HOUR) {
      Serial.println("More than 24 hours since last NTP response. Rebooting.");
      Serial.flush();
      ESP.reset();
    }

    if ((millis() - lastNTPResponse) > 24UL * ONE_HOUR) {
      Serial.println("More than 24 hours since last NTP response. Rebooting.");
      Serial.flush();
      ESP.reset();
    }
  }

  if (currentMillis - sensorsRequest > sensorsRequestPeriod) {
    sensorsRequest = currentMillis;
    uint32_t timeNow = now.unixtime() - 3*60*60; // minus 3h
    if (timeNow != 0) {
      showTimeNow();

      String t2_t = (String)t2; // Compare to "nan", to avoid holes in graphics
      if (!t2_t.equals("nan")) {
        Serial.printf("Appending temperature to file: %lu,", timeNow);
        float temp1 = t2;      // Get the temperature from the sensor
        Serial.println(temp1);

        File tempLog = SPIFFS.open("/tempr.csv", "a");  // Write the time and the temperature to the csv file
        tempLog.println((String)timeNow + ',' + (String)temp1);
        tempLog.close();
      }

      String h2_t = (String)h2;   // Compare to "nan", to avoid holes in graphics
      if (!h2_t.equals("nan")) {
        Serial.printf("Appending humidity to file: %lu,", timeNow);
        float hum1 = h2;
        Serial.println(hum1);

        File humLog = SPIFFS.open("/hum.csv", "a");     // Write the time and the temperature to the csv file
        humLog.println((String)timeNow + ',' + (String)hum1);
        humLog.close();
      }

      String air_t = (String)a;   // Compare to "nan", to avoid holes in graphics
      if (!air_t.equals("nan")) {
        Serial.printf("Appending humidity to file: %lu,", timeNow);
        float air = a;
        Serial.println(air);

        File airLog = SPIFFS.open("/air.csv", "a");     // Write the time and the temperature to the csv file
        airLog.println((String)timeNow + ',' + (String)air);
        airLog.close();
      }

      String press_t = (String)p;   // Compare to "nan", to avoid holes in graphics
      if (!press_t.equals("nan")) {
        Serial.printf("Appending humidity to file: %lu,", timeNow);
        float pre = p;
        Serial.println(pre);

        File preLog = SPIFFS.open("/pre.csv", "a");     // Write the time and the temperature to the csv file
        preLog.println((String)timeNow + ',' + (String)pre);
        preLog.close();
      }
    } else {      // If we didn't receive an NTP response yet, send another request
      sendNTPpacket(timeServerIP);
      delay(500);
    }
  }

  if (currentMillis - sensorsUpdateMillis >= sensorsUpdatePeriod) {     // update sensers every 'period'
    sensorsUpdateMillis = currentMillis;

    t1 = dht.readTemperature();
    //Serial.println("temp1: " + (String)t1);
    h1 = dht.readHumidity();
    //Serial.println("hum2: " + (String)h1);
    a = gasSensor.getPPM();
    //float zero = gasSensor.getRZero();
    //Serial.println("gas: " + (String)a + " zero: " + (String)zero);
    t2 = bme.readTemperature();
    //Serial.println("temp2: " + (String)t2);
    h2 = bme.readHumidity();
    //Serial.println("hum2: " + (String)h2);
    p = bme.readPressure() * 0.0075006;   // to 'mmHg'
    //Serial.println("press: " + (String)p);
    //Serial.println("-----------------------------" );

    if (show) {   // display sensors: Time + t2 + h2 for 5 sec, then Wifi IP + Air + pressure for 5 sec
      RTC();    // display Time
      display.println(""); // offset

      display.setTextSize(2);
      display.println((String)t2 + " *C");
      display.println((String)h2 + " %");
      display.display();

      show = false;
    } else {
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 0);
      display.print("IP: ");
      display.println(WiFi.localIP());  // show IP
      display.println("");

      display.setTextSize(2);
      display.print(a, 1); // round to '.0'
      display.println(" Air");
      display.print(p, 1); // round from '738.92' to '738.9'
      display.println(" mmHg");
      display.display();

      show = true;
    }
  }

  ArduinoOTA.handle();      // listen for OTA events
  server.handleClient();    // run the server
}

/*__________________________________________________________SETUP_FUNCTIONS__________________________________________________________*/

void startWiFi() {      // Try to connect to some given access points. Then wait for a connection
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  wifiMulti.addAP(ssid_1, password_1);      // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid_2, password_2);

  Serial.println("Connecting");     // Wait for the Wi-Fi to connect
  while (wifiMulti.run() != WL_CONNECTED) {     // wifiMulti.run() or WiFi.status()
    delay(250);
    Serial.print('.');
  }
  Serial.println("\r\n");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());      // Tell us what network we're connected to
  Serial.print("IP address:\t");
  Serial.print(WiFi.localIP());     // Send the IP address of the ESP8266 to the computer
  Serial.println("\r\n");
}

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(localPort);     // Start listening for UDP messages to port 123
  Serial.print("Local port:\t");
  Serial.println(UDP.localPort());
}

void startOTA() {     // Start the OTA service
  ArduinoOTA.setHostname(OTAName);
  ArduinoOTA.setPassword(OTAPassword);

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\r\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready\r\n");
}

void startSPIFFS() {      // Start the SPI Flash File System (SPIFFS)
  SPIFFS.begin();
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {      // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }
}

void startMDNS() {      // Start the mDNS
  MDNS.begin(mdnsName);
  Serial.print("mDNS responder started: http://");
  Serial.print(mdnsName);
  Serial.println(".local");
}

void startServer() {      // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {      // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);     // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);      // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists

  server.begin();     // start the HTTP server
  Serial.println("HTTP server started.");
}

/*__________________________________________________________SERVER_HANDLERS__________________________________________________________*/

void handleNotFound() {     // if the requested file or page doesn't exist, return a 404 not found error
  digitalWrite(LED_BUILTIN, HIGH);
  if (!handleFileRead(server.uri())) {      // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

bool handleFileRead(String path) { // send the right file to the client (if it exists)
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";          // If a folder is requested, send the index file
  String contentType = getContentType(path);             // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) { // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                         // If there's a compressed version available
      path += ".gz";                                         // Use the compressed verion
    File file = SPIFFS.open(path, "r");                    // Open the file
    size_t sent = server.streamFile(file, contentType);    // Send it to the client
    file.close();                                          // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);   // If the file doesn't exist, return false
  digitalWrite(LED_BUILTIN, LOW);
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  HTTPUpload& upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz = path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.print("handleFileUpload Name: "); Serial.println(path);
    fsUploadFile = SPIFFS.open(path, "w");               // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.print("handleFileUpload Size: "); Serial.println(upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
}

/*__________________________________________________________HELPER_FUNCTIONS__________________________________________________________*/

String formatBytes(size_t bytes) { // convert sizes in bytes to KB and MB
  if (bytes < 1024) {
    return String(bytes) + "B";
  } else if (bytes < (1024 * 1024)) {
    return String(bytes / 1024.0) + "KB";
  } else if (bytes < (1024 * 1024 * 1024)) {
    return String(bytes / 1024.0 / 1024.0) + "MB";
  }
}

String getContentType(String filename) {      // convert the file extension to the MIME type
  if (filename.endsWith(".htm")) return "text/html";
  else if (filename.endsWith(".html")) return "text/html";
  else if (filename.endsWith(".css")) return "text/css";
  else if (filename.endsWith(".js")) return "application/javascript";
  else if (filename.endsWith(".png")) return "image/png";
  else if (filename.endsWith(".gif")) return "image/gif";
  else if (filename.endsWith(".jpg")) return "image/jpeg";
  else if (filename.endsWith(".ico")) return "image/x-icon";
  else if (filename.endsWith(".xml")) return "text/xml";
  else if (filename.endsWith(".pdf")) return "application/x-pdf";
  else if (filename.endsWith(".zip")) return "application/x-zip";
  else if (filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

unsigned long getTime() { // Check if the time server has responded, if so, get the UNIX time, otherwise, return 0

  int cb = UDP.parsePacket();
  if (!cb) { // If there's no response (yet)
    //Serial.println("no packet yet");
    return 0;
  }
  Serial.print("packet received, length=");
  Serial.println(cb);
  UDP.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  // subtract seventy years:
  uint32_t UNIXTime = NTPTime - seventyYears;
  Serial.print("UDP NTP UNIXTime: " + (String)UNIXTime);
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address) {
  Serial.println("Sending NTP request");
  memset(packetBuffer, 0, NTP_PACKET_SIZE);  // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123); // NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

void RTC() {
  DateTime now = rtc.now();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(now.day(), DEC);
  display.print('/');
  display.print(now.month(), DEC);
  display.print('/');
  display.print(now.year() - 2000, DEC); // round from '2018' to '18'
  display.print(" ");
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(" ");
  display.print(now.hour(), DEC);
  display.print(':');
  display.print(now.minute(), DEC);
  display.print(':');
  display.print(now.second(), DEC);
  display.println();
  //display.display();
}

void showTimeNow() {
  DateTime now = rtc.now();

  Serial.print(" since 1970 = ");
  Serial.println(now.unixtime());

  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();
}
