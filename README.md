# ESP8266_inHome_weather
esp8266 local web server with:
 - NodeMCU v1.0;
 - BME280 - temperature, humidity, pressure sensor;
 - MQ135 - air quality sensor;
 - LCD SSD1306;
 - RTC ds3231 - 'offline' I2C clock;
 - Bootstrap web framework.

# How it works
<p> ESP8266 turn on local web server with OTA, mDNS, UDP for NTP, starts the SPIFFS to list all contents, starts a HTTP server with a file read/upload handler, connects all sensors, then displays them on the LCD and periodically store incoming data in the ESP memory.</p>
<p> So you can load webpage with domain names http://esp8266.local and enjoy cute weather server build on microcontroller with 4Mb flash and 80kb RAM :wink:.</p>

Webpage:
<img src="https://github.com/Valentin-Golyonko/esp8266_inHome_weather/blob/master/img/web_view.png" alt="web_view">

Prototype:
<img src="https://github.com/Valentin-Golyonko/esp8266_inHome_weather/blob/master/img/prototipe_view.jpg" alt="prototipe_view">

# TODO
 - <s>fix bug in the navigation menu;</s>
 - add RTC auto update from NTP;
 - PCB and protection casing.

# Thanks 
Huge thanks to <a href="https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html">Pieter P.</a> for the detailed guide on how to work with the microcontroller!
