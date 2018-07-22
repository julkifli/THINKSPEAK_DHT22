/***************************************************************************************************************
 * IoT DHT Temp/Hum and Soil Moister Station using NodeMCU ESP-12 Develop Kit V1.0
 *  DHT connected to NodeMCU pin D3
 *  Sensor Data on local OLED Display
 *  Sensor data sent to Thingspeak Channel: https://thingspeak.com/channels/CHANNEL_ID 
 *  TECC GARAGE POLIKK
 ********************************************************************************************************************************/
#define SW_VERSION " ThinkSpeak.com" // SW version will appears at innitial LCD Display

/* ESP12-E & Thinkspeak*/
#include <ESP8266WiFi.h>
WiFiClient client;
const char* MY_SSID = "YOUR SSD ID HERE";
const char* MY_PWD = "YOUR PASSWORD HERE";
const char* TS_SERVER = "api.thingspeak.com";
String TS_API_KEY ="YOUR CHANNEL WRITE API KEY";
int sent = 0;

/* TIMER */
#include <SimpleTimer.h>
SimpleTimer timer;

/* OLED */
#include <ACROBOTIC_SSD1306.h> // library for OLED: SCL ==> D1; SDA ==> D2
#include <SPI.h>
#include <Wire.h>

/* DHT22*/
#include "DHT.h"
#define DHTPIN D3  
#define DHTTYPE DHT22 
DHT dht(DHTPIN, DHTTYPE);
float hum = 0;
float temp = 0;

/* Soil Moister */
#define soilMoisterPin A0
#define soilMoisterVcc D4 //not used. LM393 VCC connect to 3.3V
int soilMoister = 0;

void setup() 
{
  pinMode(soilMoisterVcc, OUTPUT);
  Serial.begin(115200);
  delay(10);
  dht.begin();
  oledStart();
  connectWifi();
  timer.setInterval(2000L, getDhtData);
  timer.setInterval(7000L, getSoilMoisterData);
  timer.setInterval(19000L, sendDataTS);
  digitalWrite (soilMoisterVcc, LOW);
}

void loop() 
{
  displayData();
  timer.run(); // Initiates SimpleTimer
}

/***************************************************
 * Start OLED
 **************************************************/
void oledStart(void)
{
  Wire.begin();  
  oled.init();                      // Initialze SSD1306 OLED display
  clearOledDisplay();
  oled.clearDisplay();              // Clear screen
  oled.setTextXY(0,0);              
  oled.putString("  TECC GARAGE POLIKK");
}

/***************************************************
 * Get DHT data
 **************************************************/
void getDhtData(void)
{
  float tempIni = temp;
  float humIni = hum;
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  if (isnan(hum) || isnan(temp))   // Check if any reads failed and exit early (to try again).
  {
    Serial.println("Failed to read from DHT sensor!");
    temp = tempIni;
    hum = humIni;
    return;
  }
}

/***************************************************
 * Get Soil Moister Sensor data
 **************************************************/
void getSoilMoisterData(void)
{
  soilMoister = 0;
  digitalWrite (soilMoisterVcc, HIGH);
  delay (500);
  int N = 3;
  for(int i = 0; i < N; i++) // read sensor "N" times and get the average
  {
    soilMoister += analogRead(soilMoisterPin);   
    delay(150);
  }
  digitalWrite (soilMoisterVcc, LOW);
  soilMoister = soilMoister/N; 
  Serial.println(soilMoister);
  soilMoister = map(soilMoister, 380, 0, 0, 100); 
}

/***************************************************
 * Display data at Serial Monitora & OLED Display
 **************************************************/
void displayData(void)
{
  Serial.print(" Temperature: ");
  Serial.print(temp);
  Serial.print("oC   Humidity: ");
  Serial.print(hum);
  Serial.println("%");
  
  oled.setTextXY(3,0);              // Set cursor position, start of line 2
  oled.putString("TEMP: " + String(temp) + " oC");
  oled.setTextXY(5,0);              // Set cursor position, start of line 2
  oled.putString("HUM:  " + String(hum) + " %");
   oled.setTextXY(7,0);              // Set cursor position, start of line 2
  oled.putString("Soil: " + String(soilMoister) + " %");
}

/***************************************************
 * Clear OLED Display
 **************************************************/
void clearOledDisplay()
{
  oled.setFont(font8x8);
  oled.setTextXY(0,0); oled.putString("                ");
  oled.setTextXY(1,0); oled.putString("                ");
  oled.setTextXY(2,0); oled.putString("                ");
  oled.setTextXY(3,0); oled.putString("                ");
  oled.setTextXY(4,0); oled.putString("                ");
  oled.setTextXY(5,0); oled.putString("                ");
  oled.setTextXY(6,0); oled.putString("                ");
  oled.setTextXY(7,0); oled.putString("                ");
  oled.setTextXY(0,0); oled.putString("                ");              
}

/***************************************************
 * Connecting WiFi
 **************************************************/
void connectWifi()
{
  Serial.print("Connecting to "+ *MY_SSID);
  WiFi.begin(MY_SSID, MY_PWD);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("");  
}

/***************************************************
 * Sending Data to Thinkspeak Channel
 **************************************************/
void sendDataTS(void)
{
   if (client.connect(TS_SERVER, 80)) 
   { 
     String postStr = TS_API_KEY;
     postStr += "&field1=";
     postStr += String(temp);
     postStr += "&field2=";
     postStr += String(hum);
     postStr += "&field3=";
     postStr += String(soilMoister);
     postStr += "\r\n\r\n";
   
     client.print("POST /update HTTP/1.1\n");
     client.print("Host: api.thingspeak.com\n");
     client.print("Connection: close\n");
     client.print("X-THINGSPEAKAPIKEY: " + TS_API_KEY + "\n");
     client.print("Content-Type: application/x-www-form-urlencoded\n");
     client.print("Content-Length: ");
     client.print(postStr.length());
     client.print("\n\n");
     client.print(postStr);
     delay(1000); 
   }
   sent++;
   client.stop();
}
