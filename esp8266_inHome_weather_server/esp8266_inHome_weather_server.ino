// Sketch uses 349988 bytes (33%) of program storage space. Maximum is 1044464 bytes.
// Global variables use 38808 bytes (47%) of dynamic memory, leaving 43112 bytes for local variables. Maximum is 81920 bytes.
// "data" holder size = 21,407 bytes
//
// Based on https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html

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
#include <FS.h>                 // set to SPIFFS to MAX before uploading the sketch, or you will lose all .csv files
#include "RTClib.h"             // 1.781 mA
//#include <spiffs/spiffs.h>

ESP8266WiFiMulti wifiMulti;     // Create an instance of the ESP8266WiFiMulti class, called 'wifiMulti'
ESP8266WebServer server (80);   // ESP8266WebServer server (80) or ESP8266WebServerSecure server (443)
WebSocketsServer webSocket(81); // create a websocket server on port 81

IPAddress timeServerIP;         // 129.6.15.27 The time.nist.gov NTP server's IP address
unsigned int localPort = 123;   // 123 or 2390
const char* ntpServerName = "time.nist.gov"; // time.nist.gov, time.windows.com, time.google.com
const int NTP_PACKET_SIZE = 48;           // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];       // A buffer to hold incoming and outgoing packets
WiFiUDP UDP;                    // Create an instance of the WiFiUDP class to send and receive UDP messages
#define ONE_HOUR 3600000UL
const unsigned long intervalNTP = ONE_HOUR;       // Update the time every hour
unsigned long prevNTPMillis = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;          // The most recent timestamp received from the time server

const unsigned long sensorsRequestPeriod = 600000; // Do a temperature measurement every 10 min
unsigned long sensorsRequestMillis = 0;

const unsigned long wifiReconnectPeriod = 60000; // Do a temperature measurement every 1 min
unsigned long wifiReconnectMillis = 0;

const char *ssid_1 = "suslik9282";        // set up your own wifi config !
const char *password_1 = "3M0l4@09";
const char *ssid_2 = "suslik928";
const char *password_2 = "08022403";
const char *ssid_3 = "AndroidN5";
const char *password_3 = "valik2403quite";

const char *OTAName = "ESP8266";          // A name and a password for the OTA service
const char *OTAPassword = "espP0v0zdyxy"; // set up your own OTA pass!

const char* mdnsName = "esp8266";         // Domain name for the mDNS responder

String getContentType(String filename);   // SPIFFS
bool handleFileRead(String path);         // SPIFFS
File fsUploadFile;                        // a File variable to temporarily store the received file

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

Adafruit_BME280 bme;

#define MQ135_PIN A0
MQ135 gasSensor = MQ135(MQ135_PIN);

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

unsigned long sensorsUpdateMillis = 0;    // sensors udate
const int sensorsUpdatePeriod = 5000;     // sensors update 5 sec

float a = -1, t = -1, h = -1, p = -1;
bool show = true;
bool bootUp = false;

bool correction_t = true;           // temperature correction after 10 min work, because of self heating !
bool correction_delta = true;
float t_zero;
float delta_t = 0;

static const uint8_t x509[] PROGMEM = {   // The certificate is stored in PMEM
  0x30, 0xd4                              // set up your own security certificate!
};

static const uint8_t rsakey[] PROGMEM = {   // And so is the key.  These could also be in DRAM
  0x30, 0x3f                                // set up your own security key!
};

void setup ( void ) {

  Serial.begin(115200);
  delay(10);
  Serial.println('\r\n');
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite (LED_BUILTIN, LOW);    // tern on

  // actions with display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  display.display();
  delay(10);
  display.clearDisplay();                 // Clear the buffer.
  display.setTextColor(WHITE);            // 'inverted' text
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.println(" inHome\n  weather\n   server\nLoading...");
  display.display();

  delay(10);

  bme.begin(0x76);  // I2C adr.
  rtc.begin();
  /*if (rtc.lostPower()) {
    Serial.println(" --- RTC lost power, lets set the time! --- ");
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }*/

  startWiFi();          // Start a Wi-Fi access point, and try to connect to some given access points.
  startOTA();           // Start the OTA service
  startSPIFFS();        // Start the SPIFFS and list all contents
  startMDNS();          // Start the mDNS responder
  startServer();        // Start a HTTP server with a file read handler and an upload handler
  startUDP();           // Start listening for UDP messages to port 123
  tryGetNTPresponse();  // connect to a NTP server

  digitalWrite (LED_BUILTIN, HIGH); // tern off
}

/*________________________________________LOOP________________________________________*/

void loop (void) {

  ArduinoOTA.handle();      // listen for OTA events
  server.handleClient();    // run the server

  unsigned long currentMillis = millis();

  if (currentMillis - prevNTPMillis > intervalNTP) {  // Request the time from the time server every hour
    prevNTPMillis = currentMillis;

    tryGetNTPresponse();                        // work with NTP and try to synchronize RTC with time Server
  }

  if (currentMillis - sensorsUpdateMillis >= sensorsUpdatePeriod) {     // update sensors every 'period'
    sensorsUpdateMillis = currentMillis;
    bootUp = true;

    sensorData();                           // get sensors data

    displayYourStaff();                     // show collected data

    if (correction_delta) {                 // temperature correction: get delta
      if (currentMillis >= 600000) {        // temer = 10 min
        delta_t = t - t_zero;
        correction_delta = false;
        Serial.println("temperature correction done");
      }
    }
  }

  if (currentMillis - sensorsRequestMillis > sensorsRequestPeriod) {
    sensorsRequestMillis = currentMillis;

    writeSensorsDataTotheFiles();               // write data to .csv files

  } else if (bootUp) {
    bootUp = false;
    writeSensorsDataTotheFiles();               // write on boot up (power On)
  }

  if (currentMillis - wifiReconnectMillis >= wifiReconnectPeriod) {     // update sensors every 'period'
    wifiReconnectMillis = currentMillis;

    if (wifiMulti.run() == WL_DISCONNECTED ||
        wifiMulti.run() == WL_IDLE_STATUS ||
        wifiMulti.run() == WL_CONNECT_FAILED) { // reconnect to a new Wifi point
      startWiFi();
    }
  }
}

/*________________________________________SETUP_FUNCTIONS________________________________________*/

void startWiFi() {      // Try to connect to some given access points. Then wait for a connection
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  wifiMulti.addAP(ssid_1, password_1);      // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid_2, password_2);
  wifiMulti.addAP(ssid_3, password_3);

  Serial.println("Connecting...");                // Wait for the Wi-Fi to connect
  wifiMulti.run();                                // wifiMulti.run() or WiFi.status()

  delay(10);

  if (wifiMulti.run() == WL_CONNECTED) {          // Tell us what network we're connected to
    Serial.println("\r\n");
    Serial.println("Connected to:\t" + (String) WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());
    Serial.println("\r\n");
  }
}

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(localPort);                         // Start listening for UDP messages to port 123
  Serial.println("Local port:\t" + (String) UDP.localPort());
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
    digitalWrite (LED_BUILTIN, LOW);            // tern on
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite (LED_BUILTIN, HIGH);           // tern off
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

void startSPIFFS() {                            // Start the SPI Flash File System (SPIFFS)
  SPIFFS.begin();
  Serial.println("SPIFFS started. Contents:");
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {                        // List the file system contents
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("\tFS File: %s, size: %s\r\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    Serial.printf("\n");
  }

  //SPIFFS.remove("/tempr.csv");                // remove data file
  //SPIFFS.remove("/hum.csv");
  //SPIFFS.remove("/air.csv");
  //SPIFFS.remove("/pre.csv");
}

void startMDNS() {                              // Start the mDNS
  MDNS.begin(mdnsName);
  Serial.println("mDNS responder started: http://" + (String) mdnsName + ".local");
}

void startServer() {                            // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html",  HTTP_POST, []() {    // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                         // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);            // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists
  server.begin();                               // start the HTTP server
  Serial.println("HTTP server started.");
}

/*________________________________________SERVER_HANDLERS________________________________________*/

void handleNotFound() {     // if the requested file or page doesn't exist, return a 404 not found error
  digitalWrite(LED_BUILTIN, LOW);
  if (!handleFileRead(server.uri())) {      // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
  digitalWrite(LED_BUILTIN, HIGH);
}

bool handleFileRead(String path) {                          // send the right file to the client (if it exists)
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";             // If a folder is requested, send the index file
  String contentType = getContentType(path);                // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {   // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                        // Use the compressed verion
    File file = SPIFFS.open(path, "r");                     // Open the file
    size_t sent = server.streamFile(file, contentType);     // Send it to the client
    file.close();                                           // Close the file again
    Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);      // If the file doesn't exist, return false
  digitalWrite(LED_BUILTIN, HIGH);
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  digitalWrite (LED_BUILTIN, LOW);                        // tern on
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
    Serial.println("handleFileUpload Name: " + (String) path);
    fsUploadFile = SPIFFS.open(path, "w");               // Open the file for writing in SPIFFS (create if it doesn't exist)
    path = String();
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize); // Write the received bytes to the file
  } else if (upload.status == UPLOAD_FILE_END) {
    if (fsUploadFile) {                                   // If the file was successfully created
      fsUploadFile.close();                               // Close the file again
      Serial.println("handleFileUpload Size: " + (String) upload.totalSize);
      server.sendHeader("Location", "/success.html");     // Redirect the client to the success page
      server.send(303);
    } else {
      server.send(500, "text/plain", "500: couldn't create file");
    }
  }
  digitalWrite (LED_BUILTIN, HIGH);           // tern off
}

/*________________________________________HELPER_FUNCTIONS________________________________________*/

String formatBytes(size_t bytes) {            // convert sizes in bytes to KB and MB
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

void tryGetNTPresponse() {
  digitalWrite (LED_BUILTIN, LOW);          // tern on
  WiFi.hostByName(ntpServerName, timeServerIP); // Get the IP address of the NTP server
  Serial.print("Time server IP:\t");
  Serial.println(timeServerIP);

  sendNTPpacket(timeServerIP);

  uint32_t time = getTime();                // Check if the time server has responded, if so, get the UNIX time
  if (time) {
    timeUNIX = time;

    DateTime now = rtc.now();
    uint32_t rtcTime = now.unixtime();      // minus 3h
    if (timeUNIX < rtcTime) {
      Serial.println("RTC ready to Update, Check you Time Zone");
    } else {
      rtc.adjust(DateTime(timeUNIX));       // synchronize RTC with time Server
      Serial.println("RTC Updated from\t" + (String) ntpServerName);
      Serial.println("NTP response:\t" + (String) timeUNIX);
      lastNTPResponse = millis();
    }
  }

  if ((millis() - lastNTPResponse) > 24UL * ONE_HOUR) {
    Serial.println("More than 24 hours since last NTP response. Rebooting.");
    //Serial.flush();
    //ESP.reset();
  }
  digitalWrite (LED_BUILTIN, HIGH);         // tern off
}

unsigned long getTime() {   // Check if the time server has responded, if so, get the UNIX time, otherwise, return 0
  int cb = UDP.parsePacket();
  if (!cb) {                               // If there's no response (yet)
    Serial.println("no packet yet");
    return 0;
  }
  Serial.println("packet received, length=" + (String) cb);
  UDP.read(packetBuffer, NTP_PACKET_SIZE);      // read the packet into the buffer

  // Combine the 4 timestamp bytes into one 32-bit number
  uint32_t NTPTime = (packetBuffer[40] << 24) | (packetBuffer[41] << 16) | (packetBuffer[42] << 8) | packetBuffer[43];
  // Convert NTP time to a UNIX timestamp:
  // Unix time starts on Jan 1 1970. That's 2208988800 seconds in NTP time:
  const uint32_t seventyYears = 2208988800UL;
  uint32_t UNIXTime = NTPTime - seventyYears;   // subtract seventy years:
  Serial.print("UDP NTP UNIXTime: " + (String) UNIXTime);
  return UNIXTime;
}

void sendNTPpacket(IPAddress& address) {
  Serial.println("Sending NTP request");
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;             // LI, Version, Mode
  packetBuffer[1] = 0;                      // Stratum, or type of clock
  packetBuffer[2] = 6;                      // Polling Interval
  packetBuffer[3] = 0xEC;                   // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123);            // NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

void sensorData() {
  t = bme.readTemperature();
  h = bme.readHumidity();
  //a = gasSensor.getPPM();               // normal
  a = gasSensor.getCorrectedPPM(t, h);    // corrected
  //float zero = gasSensor.getRZero();    // check 'zero' and set you personal number in the MQ135 library
  //Serial.println("gas: " + (String)a + " zero: " + (String)zero);
  p = bme.readPressure() * 0.0075006;     // to 'mmHg'

  t -= delta_t;

  if (correction_t) {                     // temperature correction: get start number (room/PCB temperature)
    t_zero = t;
    correction_t = false;
  }
}

void displayYourStaff() {

  sensorDataError();                        // handle 'nan' data at first boot

  if (show) {   // display: time + temperature + humidity for 5 sec, then Wifi IP + air + pressure for 5 sec
    RTC();                                  // display Time

    display.setTextSize(2);
    display.println((String) t + " *C");
    display.setTextSize(1);                 // line spacing
    display.println("");
    display.setTextSize(2);
    display.println((String) h + " %");
    display.display();

    show = false;
  } else {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.print("IP: ");
    display.println(WiFi.localIP());        // display IP
    display.println("");

    display.setTextSize(2);
    display.print(a, 1);                    // round to '.0'
    display.println(" Air");
    display.setTextSize(1);                 // line spacing
    display.println("");
    display.setTextSize(2);
    display.print(p, 1);                    // round from '738.92' to '738.9'
    display.println(" mmHg");
    display.display();

    show = true;
  }
}

void sensorDataError() {                    // handle 'nan' data at first boot -
  String ts = (String)t;                    // some data initialization bug
  String hs = (String)h;
  String ps = (String)p;

  if (ts.equals(hs) && hs.equals(ps) && ps.equals(ts)) {     // only 1 chance to be - when "nan"
    ESP.restart();
  }
}

void writeSensorsDataTotheFiles() {
  DateTime now = rtc.now();
  uint32_t timeNow = now.unixtime() - 3 * 60 * 60; // minus 3h, TODO: Time Zone Problem !!!
  if (timeNow != 0) {
    //showTimeNow();  // check time from RTC3231

    String t_t = (String)t;                 // Compare to "nan", to avoid holes in the graphic
    if (!t_t.equals("nan")) {
      File tempLog = SPIFFS.open("/tempr.csv", "a");  // Write the time and the temperature to the csv file
      tempLog.println((String)timeNow + ',' + (String)t);
      tempLog.close();
    }

    String h_t = (String)h;                 // Compare to "nan", to avoid holes in the graphic
    if (!h_t.equals("nan")) {
      File humLog = SPIFFS.open("/hum.csv", "a");
      humLog.println((String)timeNow + ',' + (String)h);
      humLog.close();
    }

    String air_t = (String)a;               // Compare to "nan", to avoid holes in the graphic
    if (!air_t.equals("nan")) {
      File airLog = SPIFFS.open("/air.csv", "a");
      airLog.println((String)timeNow + ',' + (String)a);
      airLog.close();
    }

    String press_t = (String)p;             // Compare to "nan", to avoid holes in the graphic
    if (!press_t.equals("nan")) {
      File preLog = SPIFFS.open("/pre.csv", "a");
      preLog.println((String)timeNow + ',' + (String)p);
      preLog.close();
    }
  } else {                                  // If we didn't receive an NTP response yet, send another request
    sendNTPpacket(timeServerIP);
    delay(10);
  }
}

void RTC() {
  DateTime now = rtc.now();

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(now.day());
  display.print('/');
  display.print(now.month());
  display.print('/');
  display.print(now.year() - 2000);    // round from '2018' to '18'
  display.print(" ");
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(" ");
  display.print(now.hour());
  display.print(':');
  display.println(now.minute());
  //display.print(':');
  //display.println(now.second());
  display.println("");
}

void showTimeNow() {                        // check time from RTC3231
  DateTime now = rtc.now();

  Serial.print(" since 1970 = " + (String) now.unixtime());                             // unixtime
  Serial.print((String) now.year() + '/' + (String) now.month() + '/' + (String) now.day());
  Serial.print(" (" + (String) daysOfTheWeek[now.dayOfTheWeek()] + ") ");
  Serial.println((String) now.hour() + ':' + (String) now.minute() + ':' + (String) now.second());
}

