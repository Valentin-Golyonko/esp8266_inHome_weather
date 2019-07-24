// Made by https://github.com/Valentin-Golyonko. Apache License 2.0.
//
//Sketch uses 392300 bytes (37%) of program storage space. Maximum is 1044464 bytes.
//Global variables use 32312 bytes (39%) of dynamic memory,
//leaving 49608 bytes for local variables. Maximum is 81920 bytes.
//
// "data" holder size = 21,407 bytes
//
// Based on https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html

#include "HelpFunctions.h"
#include "Constants.h"
#include "Initialisation.h"
#include "NTP.h"

void setup(void) {

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);    // tern on
  delay(10);
  Serial.begin(115200);
  delay(10);

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

  bme.begin(0x76);      // I2C adr.
  rtc.begin();

  startWiFi();          // Start a Wi-Fi access point, and try to connect to some given access points.
  startOTA();           // Start the OTA service
  startSPIFFS();        // Start the SPIFFS and list all contents
  startMDNS();          // Start the mDNS responder
  startServer();        // Start a HTTP server with a file read handler and an upload handler
  startUDP();           // Start listening for UDP messages to port 123
  tryGetNTPresponse();  // connect to a NTP server

  digitalWrite(LED_BUILTIN, LOW); // tern off
}

/*________________________________________LOOP________________________________________*/
void loop(void) {

  ArduinoOTA.handle();      // listen for OTA events
  server.handleClient();    // run the server

  unsigned long currentMillis = millis();

  if (currentMillis - sensorsUpdateMillis > sensorsUpdatePeriod) {     // update sensors every 'period'
    sensorsUpdateMillis = currentMillis;

    sensorData();                           // get sensors data

    if (correction_delta) {                 // temperature correction: get delta
      if (currentMillis >= 600000) {        // timer = 10 min
        delta_t = t - t_zero;
        correction_delta = false;
        Serial.println("temperature correction done");
      }
    }
  } else if (first_power_on) {
    sensorData();
    sensorDataError(t, h, p);             // handle 'nan' data at first boot
    displayYourStaff();                   // show collected data

    first_power_on = false;
    writeSensorsDataTotheFiles();         // write sensor data on boot up (power On) when they are ready
  }

  if (currentMillis - lcd_update_Millis > lcd_update_period) {
    lcd_update_Millis = currentMillis;

    displayYourStaff();                   // show collected data
  }

  if (currentMillis - sensorsRequestMillis > sensorsRequestPeriod) {
    sensorsRequestMillis = currentMillis;

    writeSensorsDataTotheFiles();         // write data to .csv files
  }

  if (currentMillis - wifiReconnectMillis > wifiReconnectPeriod) {     // update sensors every 'period'
    wifiReconnectMillis = currentMillis;

    if (wifiMulti.run() == WL_DISCONNECTED ||
        wifiMulti.run() == WL_IDLE_STATUS ||
        wifiMulti.run() == WL_CONNECT_FAILED) { // reconnect to a new Wifi point
      startWiFi();
    }
  }

  if (currentMillis - prevNTPMillis > intervalNTP) {  // Request the time from the time server every hour
    prevNTPMillis = currentMillis;

    tryGetNTPresponse();                        // work with NTP and try to synchronize RTC with time Server
  }
}

/*________________________________________SETUP_FUNCTIONS________________________________________*/

void startWiFi() {      // Try to connect to some given access points. Then wait for a connection
  WiFi.mode(WIFI_STA);
  //WiFi.begin(ssid, password);
  wifiMulti.addAP(ssid_3, password_3);
  wifiMulti.addAP(ssid_4, password_4);
  wifiMulti.addAP(ssid_1, password_1);        // add Wi-Fi networks you want to connect to
  wifiMulti.addAP(ssid_2, password_2);

  Serial.println("Connecting...");            // Wait for the Wi-Fi to connect
  wifiMulti.run();                            // wifiMulti.run() or WiFi.status()
  delay(10);

  if (wifiMulti.run() == WL_CONNECTED) {      // Tell us what network we're connected to
    Serial.println("\r\n");
    Serial.println("Connected to:\t" + (String) WiFi.SSID());
    Serial.print("IP address:\t");
    Serial.print(WiFi.localIP());
    Serial.println("\r\n");
  }
}

void startUDP() {
  Serial.println("Starting UDP");
  UDP.begin(localPort);                       // Start listening for UDP messages to port 123
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
    digitalWrite(LED_BUILTIN, HIGH);          // tern on
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    digitalWrite(LED_BUILTIN, LOW);           // tern off
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

  //  SPIFFS.remove("/tempr.csv");                // remove data file
  //  SPIFFS.remove("/hum.csv");
  //  SPIFFS.remove("/air.csv");
  //  SPIFFS.remove("/pre.csv");
}

void startMDNS() {                              // Start the mDNS
  MDNS.begin(mdnsName);
  Serial.println("mDNS responder started: http://" + (String) mdnsName + ".local");
}

void startServer() {                            // Start a HTTP server with a file read handler and an upload handler
  server.on("/edit.html", HTTP_POST, []() {     // If a POST request is sent to the /edit.html address,
    server.send(200, "text/plain", "");
  }, handleFileUpload);                         // go to 'handleFileUpload'

  server.onNotFound(handleNotFound);            // if someone requests any other file or page, go to function 'handleNotFound'
  // and check if the file exists
  server.begin();                               // start the HTTP server
  Serial.println("HTTP server started.");
}

/*________________________________________SERVER_HANDLERS________________________________________*/

void handleNotFound() {     // if the requested file or page doesn't exist, return a 404 not found error
  digitalWrite(LED_BUILTIN, HIGH);
  if (!handleFileRead(server.uri())) {      // check if the file exists in the flash memory (SPIFFS), if so, send it
    server.send(404, "text/plain", "404: File Not Found");
  }
  digitalWrite(LED_BUILTIN, LOW);
}

bool handleFileRead(String path) {                          // send the right file to the client (if it exists)
  digitalWrite(LED_BUILTIN, HIGH);
  //Serial.println("handleFileRead: " + path);
  if (path.endsWith("/")) path += "index.html";             // If a folder is requested, send the index file
  String contentType = getContentType(path);                // Get the MIME type
  String pathWithGz = path + ".gz";
  if (SPIFFS.exists(pathWithGz) ||
      SPIFFS.exists(path)) {   // If the file exists, either as a compressed archive, or normal
    if (SPIFFS.exists(pathWithGz))                          // If there's a compressed version available
      path += ".gz";                                        // Use the compressed version
    File file = SPIFFS.open(path, "r");                     // Open the file
    size_t sent = server.streamFile(file, contentType);     // Send it to the client
    file.close();                                           // Close the file again
    //Serial.println(String("\tSent file: ") + path);
    return true;
  }
  Serial.println(String("\tFile Not Found: ") + path);      // If the file doesn't exist, return false
  digitalWrite(LED_BUILTIN, LOW);
  return false;
}

void handleFileUpload() { // upload a new file to the SPIFFS
  digitalWrite(LED_BUILTIN, HIGH);                        // tern on
  HTTPUpload &upload = server.upload();
  String path;
  if (upload.status == UPLOAD_FILE_START) {
    path = upload.filename;
    if (!path.startsWith("/")) path = "/" + path;
    if (!path.endsWith(
          ".gz")) {                         // The file server always prefers a compressed version of a file
      String pathWithGz =
        path + ".gz";                  // So if an uploaded file is not compressed, the existing compressed
      if (SPIFFS.exists(pathWithGz))                     // version of that file must be deleted (if it exists)
        SPIFFS.remove(pathWithGz);
    }
    Serial.println("handleFileUpload Name: " + (String) path);
    fsUploadFile = SPIFFS.open(path,
                               "w");               // Open the file for writing in SPIFFS (create if it doesn't exist)
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
  digitalWrite(LED_BUILTIN, LOW);           // tern off
}

/*________________________________________HELPER_FUNCTIONS________________________________________*/

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

void sendJson(int time_now) {
  if (!client.connect(host, httpPort)) {
    //Serial.println("connection failed");
    return;
  }

  StaticJsonDocument<200> doc;
  // DynamicJsonDocument  doc(200);
  doc["sensor"] = "esp8266";
  doc["time"] = time_now;
  doc["temp"] = t;
  doc["hum"] = h;
  doc["air"] = a;
  doc["pres"] = p;

  serializeJson(doc, client);

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  client.print("q");

  Serial.println();
  Serial.println("closing connection");
}

void writeSensorsDataTotheFiles() {
  DateTime now = rtc.now();
  uint32_t timeNow = now.unixtime() - 3 * 60 * 60; // minus 3h, TODO: Time Zone Problem !!!
  if (timeNow != 0) {

    sendJson(timeNow);

    //    String t_t = (String) t;                 // Compare to "nan", to avoid holes in the graphic
    //    if (!t_t.equals("nan")) {
    //      File tempLog = SPIFFS.open("/tempr.csv", "a");  // Write the time and the temperature to the csv file
    //      tempLog.println((String) timeNow + ',' + (String) t);
    //      tempLog.close();
    //    }
    //
    //    String h_t = (String) h;
    //    if (!h_t.equals("nan")) {
    //      File humLog = SPIFFS.open("/hum.csv", "a");
    //      humLog.println((String) timeNow + ',' + (String) h);
    //      humLog.close();
    //    }
    //
    //    String air_t = (String) a;
    //    if (!air_t.equals("nan")) {
    //      File airLog = SPIFFS.open("/air.csv", "a");
    //      airLog.println((String) timeNow + ',' + (String) a);
    //      airLog.close();
    //    }
    //
    //    String press_t = (String) p;
    //    if (!press_t.equals("nan")) {
    //      File preLog = SPIFFS.open("/pre.csv", "a");
    //      preLog.println((String) timeNow + ',' + (String) p);
    //      preLog.close();
    //    }
  } else {                                  // If we didn't receive an NTP response yet, send another request
    sendNTPpacket(timeServerIP);
    delay(10);
  }
}

void RTC() {
  DateTime now = rtc.now();
  String rtc_day = ifTimeNumber10(now.day());
  String rtc_month = ifTimeNumber10(now.month());
  String rtc_year = ifTimeNumber10(now.year() - 2000);  // round from '2018' to '18'
  String rtc_hour = ifTimeNumber10(now.hour());
  String rtc_minute = ifTimeNumber10(now.minute());

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.print(rtc_day);
  display.print('/');
  display.print(rtc_month);
  display.print('/');
  display.print(rtc_year);
  display.print(" ");
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(" ");
  display.print(rtc_hour);
  display.print(':');
  display.println(rtc_minute);
  //display.print(':');
  //display.println(now.second());
  display.println("");
}

void showTimeNow() {                        // check time from RTC3231
  DateTime now = rtc.now();

  Serial.print(" since 1970 = " + (String) now.unixtime());                             // unix time
  Serial.print((String) now.year() + '/' + (String) now.month() + '/' + (String) now.day());
  Serial.print(" (" + (String) daysOfTheWeek[now.dayOfTheWeek()] + ") ");
  Serial.println((String) now.hour() + ':' + (String) now.minute() + ':' + (String) now.second());
}
