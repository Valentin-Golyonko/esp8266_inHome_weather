# ESP8266_inHome_weather
esp8266 local web server with:
 - NodeMCU v1.0;
 - BME280 - temperature, humidity, pressure sensor;
 - MQ135 - air quality sensor;
 - LCD SSD1306;
 - RTC ds3231 - 'offline' I2C clock;
 - Bootstrap web framework.

# How it works
In this example ESP8266 has:
 - local web server with OTA, mDNS, UDP (for NTP);
 - SPIFFS to list all contents;
 - HTTP server with a file read/upload handler;
 - connects all sensors and displays them on the LCD;
 - periodically store incoming data in the memory.
<p>So you can load webpage with domain names http://esp8266.local and enjoy cute weather server build on microcontroller with 4Mb flash and 80kb RAM :wink:. </p>

If you want to watch webpage (without ESP you can't run it): https://youtu.be/D3MsvsaVtrQ

Webpage:
<img src="https://github.com/Valentin-Golyonko/esp8266_inHome_weather/blob/master/img/web_view.png" alt="web_view">

Prototype:
<img src="https://github.com/Valentin-Golyonko/esp8266_inHome_weather/blob/master/img/prototipe_view.jpg" alt="prototipe_view">

# TODO
 - <s>fix bug in the navigation menu;</s>
 - <s>add RTC auto update from NTP;</s>
 - add SPI flash to store the data;
 - PCB and protection casing.

# Thanks 
Huge thanks to <a href="https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html">Pieter P.</a> for the detailed guide on how to work with the microcontroller!
