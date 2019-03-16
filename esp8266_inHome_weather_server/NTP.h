
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

void sendNTPpacket(IPAddress &address) {
  Serial.println("Sending NTP request");
  memset(packetBuffer, 0, NTP_PACKET_SIZE); // set all bytes in the buffer to 0
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;             // LI, Version, Mode
  packetBuffer[1] = 0;                      // Stratum, or type of clock
  packetBuffer[2] = 6;                      // Polling Interval
  packetBuffer[3] = 0xEC;                   // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  // send a packet requesting a timestamp:
  UDP.beginPacket(address, 123);            // NTP requests are to port 123
  UDP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP.endPacket();
}

void tryGetNTPresponse() {
  digitalWrite(LED_BUILTIN, LOW);          // tern on
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
  digitalWrite(LED_BUILTIN, HIGH);         // tern off
}
