/*
 *  This sketch sends data via HTTP GET requests to data.sparkfun.com service.
 *
 *  You need to get streamId and privateKey at data.sparkfun.com and paste them
 *  below. Or just customize this script to talk to other HTTP servers.
 *
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
#include <DHT.h>
#include <EEPROM.h>
#define DHTTYPE DHT22
#define DHTPIN  2


/* Set these to your desired credentials. */
const char *ssidap = "EZM2M_IOT";
const char *passwordap = "ih8veggies";

String currentTemp;
String currentHumidity;

int ap = 0;


ESP8266WebServer server(80);

IPAddress ip;

const char* Defaultssid     = "MotherShip2";
const char* Defaultpassword = "network23";
//const char* ssid     = "klaatu";
//const char* password = "28282828282828282828282828";

const char* host = "108.244.166.20";
const char* streamId   = "Moisture 74.5%";
const char* privateKey = "....................";
char m2xheader[] = "PUT /v2/devices/a813da24bad919f210af638b1d599c92/streams/humidity/value HTTP/1.1\r\nUser-Agent: ezm2m.com\r\nHost: 108.244.164.205\r\nAccept: */*\r\nX-M2X-KEY: 005a2183aad40afd80c89e7c8a86a73e\r\nContent-Type: application/json\r\n";
char m2xheader2[] = "PUT /v2/devices/a813da24bad919f210af638b1d599c92/streams/temperature/value HTTP/1.1\r\nUser-Agent: ezm2m.com\r\nHost: 108.244.164.205\r\nAccept: */*\r\nX-M2X-KEY: 005a2183aad40afd80c89e7c8a86a73e\r\nContent-Type: application/json\r\n";

char m2xdata[512];
char bigbuff[512];
char timestamp[64];
char m2xlength[128];
String st;
String content;


void handleRoot() {
//    IPAddress ip = WiFi.softAPIP();
    String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
    content = "<!DOCTYPE HTML>\r\n<html><body><center><h1>EZ IoT</h1><br>";
    content += ipStr;
    content += "<br>";
    content += "Current Sensor Readings<br>Temperature:";
    content += currentTemp;
    content += "<br>Humidity:";
    content += currentHumidity;
    content += "<br><p>";
    content += st;
    content += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><br>PASS:<input name='pass' length=64><br><br><input type='submit'></form>";
    if(ap == 0)
      content += "<br><br><img src=\"http://api-m2x.att.com/v2/charts/05d65a9a99cf197cf1907b69926ed15d.png?width=800&height=600&type=values\"><br><br>bill@ezm2m.com</center></body></html>";
    else
      content += "<br><br><br><br>bill@ezm2m.com</center></body></html>";
    server.send(200, "text/html", content);  
}

void clearEEprom() {
  content = "<!DOCTYPE HTML>\r\n<html>";
  content += "<p>Clearing the EEPROM</p></html>";
  server.send(200, "text/html", content);
  Serial.println("setting eeprom to 0x00");
  for (int i = 0; i < 128; ++i) { EEPROM.write(i, 0); }
  EEPROM.commit();
  delay(100);
}


void setCredentials() {
    String ssid = server.arg("ssid");
    String pass = server.arg("pass");
    if (ssid.length() > 0 && pass.length() > 0) {
      Serial.println("clearing eeprom");
      for (int i = 0; i < 96; ++i) { EEPROM.write(i, 0); }
      Serial.println(ssid);
      Serial.println("");
      Serial.println(pass);
      Serial.println("");

      Serial.print("writing eeprom ssid: ");
      for (int i = 0; i < ssid.length(); ++i)
      {
        EEPROM.write(i, ssid[i]);
        Serial.print(ssid[i]); 
      }
      Serial.print("writing eeprom pass: "); 
      for (int i = 0; i < pass.length(); ++i)
        {
          EEPROM.write(32+i, pass[i]);
          Serial.print(pass[i]); 
        }    
      EEPROM.commit();
      content = "<!DOCTYPE HTML>\r\n<html>";
      content += "<p>saved to eeprom... reset to boot into new wifi</p></html>";
    } else {
      content = "Error";
      Serial.println("Sending 404");
    }
    server.send(200, "text/html", content);
}


void becomeAP()
{
  WiFi.softAP(ssidap);
  delay(500);
  Serial.println("done");
  ip = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(ip);
  ap = 1;
}
void initServer()
{
  server.on("/", handleRoot);
  server.on("/cleareeprom", clearEEprom);
  server.on("/setting", setCredentials);
  server.begin();
  Serial.println("HTTP server started");
}

// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
 
float humidity, temp_f;  // Values read from sensor
String webString="";     // String to display
// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const long interval = 2000;              // interval at which to read sensor


int address = 0;
byte value;
char savedSSID[32];
char savedPASS[32];

void setup() {
  int timeout;
  int x;
  String mySSID;
  String myPassword;
  Serial.begin(115200);
  delay(10000);

  EEPROM.begin(512);
  delay(1000);
  Serial.println();
  Serial.println();
  // looking for saved SSID and PASSWORD


  x = 0;
  for (int i = 0; i < 32; ++i)
  {
    savedSSID[x++] = char(EEPROM.read(i));
  }
  Serial.print("SSID: ");
  Serial.println(savedSSID);
  Serial.println("Reading EEPROM pass");


  x = 0;
  for (int i = 32; i < 64; ++i)
  {
    savedPASS[x++] = char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(savedPASS);  
  // We start by connecting to a WiFi network
  mySSID = String(savedSSID);
  myPassword = String(savedPASS);
  if (mySSID.length() > 0 && myPassword.length() > 0) 
  {
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(savedSSID);
  

    WiFi.begin(savedSSID, savedPASS);
    timeout = 0;
    while (WiFi.status() != WL_CONNECTED) 
    {
      delay(500);
      Serial.print(".");
      timeout++;
      if(timeout > 50) // lets become an access point and serve a configuration page
      {
        WiFi.disconnect();
	Serial.print("Could not connect, Configuring access point...");
	/* You can remove the password parameter if you want the AP to be open. */
        becomeAP();
        break;
      }
    }
  } //if
  else
    becomeAP();
  initServer();  
  Serial.println("");
  Serial.println("WiFi connected");  
  if(ap)
  {
    Serial.println("Access Point");
    ip = WiFi.softAPIP();
  }
  else
    ip = WiFi.localIP();
  Serial.println("IP address: ");
  Serial.println(ip);
  gettemperature();
}

int which = 0;
int countdown = 3000;
void loop() {
//  delay(5000);
  countdown--;
  if(countdown <= 0)
  {
    countdown = 3000;
    if(ap == 0)
    {
      Serial.print("connecting to ");
      Serial.println(host);
      
      // Use WiFiClient class to create TCP connections
      WiFiClient client;
      const int httpPort = 80;
      if (!client.connect(host, httpPort)) {
        Serial.println("connection failed");
        return;
      }
      gettemperature();       // read sensor
      if(which == 0)
      { 
        which = 1;   
        String m2xurl = "{ \"value\": \""+String((float)humidity)+"\" }";
        String m2xlength = "Content-Length: " + String(m2xurl.length()) + "\r\n\r\n"; 
        String m2xdata = m2xheader;
        m2xdata += m2xlength;
        m2xdata += m2xurl;
        Serial.print("Requesting URL: ");
        Serial.println(m2xdata);
        client.print(m2xdata);
      }
      else
      { 
        which = 0;
     
        String m2xurl = "{ \"value\": \""+String((float)temp_f)+"\" }";
        String m2xlength = "Content-Length: " + String(m2xurl.length()) + "\r\n\r\n"; 
    
        String m2xdata = m2xheader2;
        m2xdata += m2xlength;
        m2xdata += m2xurl;
        Serial.print("Requesting URL: ");
        Serial.println(m2xdata);
        client.print(m2xdata);
      }
      delay(2000);
      
      // Read all the lines of the reply from server and print them to Serial
      while(client.available()){
        String line = client.readStringUntil('\r');
        Serial.print(line);
      }
      
      Serial.println();
      Serial.println("closing connection");
  //    delay(30000);
    } // if ap == 0
  }// if countdown
//  else
//  {
    server.handleClient();
    delay(10);
    countdown--;
//  }
}
void gettemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature(true);     // Read temperature as Fahrenheit
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
    else
    {
        currentHumidity = String((float)humidity);
        currentTemp = String((float)temp_f);
    }
  }
}
