
/*
 *  This app monitors the temperature of a room and sends a message
 *  to a Discord channel if it gets too high (or low).
 *  
 *  Background: a couple of times now the A/C has failed in the room with our
 *  test servers and networking gear (production stuff is in a proper hosting 
 *  facility, no problem there).  The last time we were lucky, someone
 *  walking by heard all the fan motors screaming and took action.  I decided
 *  to build something to warn us, in case we weren't so lucky next time
 *  
 *  Based on the Adafruit ESP8266 Hazzah featherwing board:  
 *    https://www.adafruit.com/product/2821
 *    
 *  Uses DHT mperature sensor:  
 *    https://www.sparkfun.com/products/10167
 *    (this is supposed to be a 5v part, but it works OK at 3.3v... YMMV)
 *    
 *  Uses SparkFun DeadOn RTC Breakout - DS3234
 *    https://www.sparkfun.com/products/10160
 *  
 *  You will need to install these libraries:
 *  
 *  DHT: https://github.com/adafruit/DHT-sensor-library
 *  WiFi: https://github.com/esp8266/Arduino
 *  RTC: https://github.com/sparkfun/SparkFun_DS3234_RTC_Arduino_Library
 */
#include <SPI.h>
#include <SparkFunDS3234RTC.h>
#include <ESP8266WiFi.h>
#include "DHT.h"

#define DS13074_CS_PIN 16   // real time clock chip select pin - RTC also uses SPI pins: 12,13,14
#define DHT_DATA_PIN    5   // pin for temp/humidity data

#define TEMP_THRESHOLD_MIN    60.0    // minimum temp threshold
#define TEMP_THRESHOLD_MAX    75.0    // maximum temp threshold

// wifi network to use:
const char* ssid = "<network SSID here>";
const char* password = "<network password here>";

// where to send the message:
const char* host = "discordapp.com";
String      url = "/api/webhooks/<channelID>/<channel API key>";  // get these from Discord
const int   httpPort = 443;
const int   sleepTimeSecs = 60;
const int   timeoutMillis = 5000;
String      bootMessage = "Hello%21+Server+room+temperature+monitor+online.";

// temperature sensor library object:
DHT         dht;

// just for startup message
boolean first_time = true;

/*
 * one-time setup function
 */
void setup() {
  Serial.begin(115200);       // just for debugging
  delay(100);

  Serial.println("Starting up:");
  Serial.println();
  
  dht.setup(DHT_DATA_PIN);    // start up teh temp sensor

  delay(200);

  rtc.begin(DS13074_CS_PIN);  // start up the real-time-clock

  delay(200);

  sendMessage(bootMessage);   // send a hello message

/*
 * Endless loop
 */
void loop() {

  // get data from temperature sensor
  float humidity = dht.getHumidity();
  float temp = dht.getTemperature();
  temp = dht.toFahrenheit(temp);
  Serial.print("Temp : ");
  Serial.println(temp);
  Serial.print("Humid: ");
  Serial.println(humidity);

  // if it's too hot (or cold), send message
  if((temp > TEMP_THRESHOLD_MAX) || (temp < TEMP_THRESHOLD_MIN) || first_time) {
    sendTemp((int) (temp + 0.5), (int) (humidity + 0.5));
  }

  // delay and do it all again
  Serial.println("Waiting...");
  first_time = false;
  delay(1000 * sleepTimeSecs);
}

/*
 * Pull time from real time clock as a String
 */
String getTime() {
  rtc.update();
  String wall_clock_time = String(rtc.hour()) + "%3A"; // %3A is URL Encoded form of ":"
  if (rtc.minute() < 10) {
    wall_clock_time = wall_clock_time + "0" + rtc.minute();
  } else {
    wall_clock_time = wall_clock_time + rtc.minute();
  }
  return wall_clock_time;
}

/*
 * Send temperature message
 */
void sendTemp(int temp, int humid) {
    String wct = getTime();
    
    // NOTE: this has to be url-encoded, since we're sending a form-type POST
    String message = String("+") + wct + "+-+Server+room%3A+" + temp + "%C2%BAF%2C+" + humid + "%25";
    sendMessage(message);
}

/*
 * Send data to Discord channel by connecting to wifi, connecting to the website, sending
 * the message as an HTTP POST, disconnecting from the website and disconnecting from wifi.
 * It makes sense to connect and disconnect with each message, because messages should
 * tyipically only be sent once in a while...
 */
void sendMessage (String message) {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // re-connect to WiFi each time, since calls to this should be rare
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf("Connection status: %d\n", WiFi.status());
    delay(2000);
  }
  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  // now connect to website
  Serial.print("Connecting to ");
  Serial.println(host);
  
  // Use WiFiClientSecure class to create TCP connections
  // Note: Discord *requires* a secure connection
  WiFiClientSecure client;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed!");
  } else {

    // build a packet (by hand, like an animal, since this is a microcontroller)
    Serial.println("Connected to ");
    Serial.println(host);

    // note that this has to be url-encoded, since we're sending a form-type POST
    String post = String("content=") + message;
    String packet = String("POST ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" + 
                 "Content-Type: application/x-www-form-urlencoded\r\n" +
                 "Content-Length: " + post.length() + "\r\n" +
                 "Connection: close\r\n\r\n" +
                 post + "\r\n";
                 
    Serial.println("Packet: ");
    Serial.println(packet);
    
    // This will send the message to the server
    client.print(packet);

    // disconnect from server
    client.stop();
    Serial.println("Disconnecting...");
  }

  // drop WiFi connection
  WiFi.disconnect();
}

