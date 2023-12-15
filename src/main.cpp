#include <Arduino.h>
#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ThingSpeak.h>
#include "config.h"

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
MKRIoTCarrier carrier;
char ssid[] = WIFI_NAME;        // your network SSID (name)
char pass[] = WIFI_PASSWORD;    // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;
const char* mqttServer = "mqtt3.thingspeak.com";  // Replace with your MQTT Broker address
const int mqttPort = 1883;                  // typical MQTT port

WiFiClient wifiClient;

void setupWiFi() {
 // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(INTERVAL);
  }
  // you're connected now, so print out the data:
  Serial.println("You're connected to the network");
}

void setup() {

  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  setupWiFi();
  ThingSpeak.begin(wifiClient);
  carrier.begin();

  // // Display text near each button
  // displayButtonText("Button 1", 30, 70);
  // displayButtonText("Button 2", 160, 70);
  // displayButtonText("Button 3", 160, 150);
  // displayButtonText("Button 4", 100, 200);
  // displayButtonText("Button 5", 30, 150);
}

void displayButtonText(int text, int x, int y) {
  carrier.display.setCursor(x, y);
  carrier.display.setTextSize(2);
  carrier.display.setTextColor(0xFFFF); // White color, you can change it as needed
  carrier.display.print(text);
}

void loop() {

// read the sensor values
  float temperature = carrier.Env.readTemperature();
  float humidity = carrier.Env.readHumidity();
  float pressure = carrier.Pressure.readPressure();
    // set the fields with the values
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);

  carrier.display.fillScreen(0x0000);
  // carrier.display.setTextColor(0x07FF);
  // carrier.display.setTextSize(2);

 // Display text near each button
  displayButtonText(temperature, 30, 70);
  displayButtonText(humidity, 160, 70);
  displayButtonText(pressure, 160, 150);
  displayButtonText(0, 100, 200);
  displayButtonText(0, 30, 150);


  
  // //temp display
  // carrier.display.setCursor(50, 50);
  // carrier.display.print("temp: ");
  // carrier.display.print(temperature);

  // //temp display
  // carrier.display.setCursor(50, 120);
  // carrier.display.print("humidity: ");
  // carrier.display.print(humidity);

  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   if(x == 200){
    Serial.println("Channel update successful.");
  }
  else{
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
  delay(INTERVAL);  // Delay for interval
}
