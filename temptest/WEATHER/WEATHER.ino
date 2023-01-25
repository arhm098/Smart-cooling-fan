#include "DHT.h"
#define DHTPIN 4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C   lcd(0x27,16,2);
#define ENB 14
#define INT1 12
#define INT2 13

// const char* ssid = "SKY Fibercom";
// const char* password = "03004030033";

const char* ssid = "WiFi-Students-A";
const char* password = "FccWiFi5500";

// Your Domain name with URL path or IP address with path
String openWeatherMapApiKey = "9b6c1e683e977f9f1297a5f4979cc951";
// Example:
//String openWeatherMapApiKey = "bd939aa3d23ff33d3c8f5dd1dd4";

// Replace with your country code and city
String city = "Lahore";
String countryCode = "PK";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;

String jsonBuffer;

void setup() {
  pinMode(ENB,OUTPUT);
  pinMode(INT1,OUTPUT);
  pinMode(INT2,OUTPUT);
  digitalWrite(INT1,LOW);
  digitalWrite(INT2,HIGH);
  analogWrite(ENB,0);
  // digitalWrite(ENB,HIGH);
  Serial.begin(115200);
  Wire.begin(2,0);
  dht.begin();
  lcd.init();
  lcd.backlight();
  WiFi.begin(ssid, password);
  lcd.print("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.print(".");
  }
  lcd.clear();
}

void loop() {
  lcd.setCursor(0,0);
  lcd.print("T:");
  // Read temperature as Celsius (the default)
  int t = dht.readTemperature();
   lcd.setCursor(2,0);
  lcd.print(t);
  lcd.print("C ");
  lcd.setCursor(0,1);
  lcd.print("H:");
  //humidity
  int h = dht.readHumidity();
  lcd.setCursor(2,1);
  lcd.print(h);
  lcd.print("% ");

  // Send an HTTP GET request
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
      jsonBuffer = httpGETRequest(serverPath.c_str());
      //Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
    
      //temperature forcast
      lcd.setCursor(6,0);
      lcd.print("T:");
      int tempDegC = int(myObject["main"]["temp"]); 
      lcd.print(tempDegC-273);
      lcd.print(" ");
      lcd.setCursor(10,0);
      lcd.print(" H:");
      lcd.print(myObject["main"]["humidity"]);
      lcd.print(" ");
      lcd.setCursor(6,1);
      lcd.print(myObject["weather"][0]["main"]);
      Serial.print(myObject["weather"]);
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }
  ///fan control
  if (t > 20.0)
  {
    int new_t = int(t) -19;
    analogWrite(ENB,map(new_t*100, 0, 1023, 0, 255));
  }
  if (t <= 20.0)
  {/*  */
    analogWrite(ENB,0);
  }
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
