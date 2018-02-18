# ServerRoomSentry

This app monitors the temperature of a room and sends a message
to a Discord channel if it gets too high (or low).

Background: a couple of times now the A/C has failed in the room with our
test servers and networking gear (production stuff is in a proper hosting 
facility, no problem there).  The last time we were lucky, someone
walking by heard all the fan motors screaming and took action.  I decided
o build something to warn us, in case we weren't so lucky next time

Based on the Adafruit ESP8266 Hazzah featherwing board:  
  https://www.adafruit.com/product/2821

Uses DHT mperature sensor:  
  https://www.sparkfun.com/products/10167
   (this is supposed to be a 5v part, but it works OK at 3.3v... YMMV)
   
Uses SparkFun DeadOn RTC Breakout - DS3234
  https://www.sparkfun.com/products/10160
  
You will need to install these libraries:
  
  DHT: https://github.com/adafruit/DHT-sensor-library
  WiFi: https://github.com/esp8266/Arduino
  RTC: https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library

 


