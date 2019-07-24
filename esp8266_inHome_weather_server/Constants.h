//---
const unsigned long wifiReconnectPeriod = 60000;
unsigned long wifiReconnectMillis = 0;

//--- wifi spots
const char* ssid_1 = "xxxx";        // set up your own wifi config !
const char* password_1 = "xxxx";
const char* ssid_2 = "xxxx";
const char* password_2 = "xxxx";
const char* ssid_3 = "xxxx";
const char* password_3 = "xxxx";
const char* ssid_4 = "xxxx";
const char* password_4 = "xxxx";

const char* host = "192.168.xxx.xxx.";    // server IP address

//--- OTA
const char* OTAName = "xxxx";       // A name and a password for the OTA service
const char* OTAPassword = "xxxx";   // set up your own OTA pass!

//--- LAN Name - http://esp8266.local
const char* mdnsName = "esp8266";                     // Domain name for the mDNS responder

//--- Sensors
const unsigned long sensorsRequestPeriod = 60000 * 10;    // store a date to a file every X min
unsigned long sensorsRequestMillis = 0;
const unsigned long sensorsUpdatePeriod = 1000 * 5;     // sensors update X sec
unsigned long sensorsUpdateMillis = 0;

float a = 0.0f, t = 0.0f, h = 0.0f, p = 0.0f;
bool show = true;
bool first_power_on = true;

bool correction_t = true;           // temperature correction after 10 min work, because of self heating !
bool correction_delta = true;
float t_zero = 0.0f;
float delta_t = 0.0f;

//--- LCD
const unsigned long lcd_update_period = 1000 * 5;              // X sec
unsigned long lcd_update_Millis = 0;

//--- RTC
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

//--- NTP
unsigned int localPort = 2390;   // 123 or 2390
const char* ntpServerName = "time.nist.gov"; // time.nist.gov, time.windows.com, time.google.com
const int NTP_PACKET_SIZE = 48;           // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[NTP_PACKET_SIZE];       // A buffer to hold incoming and outgoing packets

#define ONE_HOUR 3600000UL
const unsigned long intervalNTP = ONE_HOUR;       // Update the time every hour
unsigned long prevNTPMillis = 0;
unsigned long lastNTPResponse = millis();
uint32_t timeUNIX = 0;          // The most recent timestamp received from the time server

//--- set up your own security certificate for HTTPS
static const uint8_t x509[]
PROGMEM = {
  0x30, 0xd4
};

static const uint8_t rsakey[]
PROGMEM = {
  0x30, 0x3f
};
