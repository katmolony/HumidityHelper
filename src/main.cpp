#include <Arduino.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include <Arduino_MKRIoTCarrier.h>
#include <ThingSpeak.h>
#include "config.h"

#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>

unsigned long myChannelNumber = SECRET_CH_ID;   // ThinkSpeak Channel
const char *myWriteAPIKey = SECRET_WRITE_APIKEY;    //ThinkSpeak API
MKRIoTCarrier carrier;
char ssid[] = WIFI_NAME;       // your network SSID (name)
char pass[] = WIFI_PASSWORD;   // your network password (use for WPA, or use as key for WEP)
int status = WL_IDLE_STATUS;

WiFiClient wifiClient;  // Use WiFiClient for HTTP

float lat = LAT;
float lng = LONG;
char weatherAPI[] = WEATHER_API;

// Global variables for inside temperature, humidity, and pressure
float inTemperature;
float inHumidity;
float inPressure;

// Global variables for outside temperature, humidity, and pressure
float outTemperature;
float outHumidity;
float outPressure;

HttpClient client = HttpClient(wifiClient, "api.openweathermap.org", 80);  // Use port 80 for HTTP

bool buttonPressed = false;
unsigned long buttonStartTime = 0;
// variable for device power on for 2 mins
unsigned long buttonDuration = 120000;  // 2 minutes in milliseconds
unsigned long currentTime = millis();
int deviceStatus = 0;
char devicePower[] = "OFF";

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

  carrier.withCase();   //Remove this if your arduino is NOT in it's case/housing
  carrier.Buttons.updateConfig(100, TOUCH0);
  carrier.Button2.updateConfig(4); //change sensitivity of button 2

  carrier.begin();

}

// function to diplay weather readings on display screen
void displayButtonReading (float reading, int x, int y) {
  carrier.display.setCursor(x, y);
  carrier.display.setTextSize(2);
  carrier.display.setTextColor(0xFFFF); // White color, you can change it as needed
  carrier.display.print(reading, 1);  // Display with one decimal place
}

// function to display temperature with celsius on display screen
void displayTemperatureWithCelsius(float temperature, int x, int y) {
  carrier.display.setCursor(x, y);
  carrier.display.setTextSize(2);
  carrier.display.setTextColor(0xFFFF); // White color, you can change it as needed
  carrier.display.print(temperature, 1);  // Display temperature with one decimal place
  carrier.display.setTextSize(0.2);
  carrier.display.print("o"); // Display degree symbol
  carrier.display.setTextSize(2);
  carrier.display.print(F("C")); // Display the "C" for Celsius
}

// function to display string text for device on display screen
void displayButtonText (const char *text, int x, int y) {
  carrier.display.setCursor(x, y);
  carrier.display.setTextSize(2);
  carrier.display.setTextColor(0xFFFF); // White color, you can change it as needed
  //carrier.display.print(devicePower);  
  carrier.display.print(text);
  //carrier.display.print(text);
  //   // Display "on" or "off" based on the passed state
  // if (text == 'on') {
  //   carrier.display.print("on");
  // } else {
  //   carrier.display.print("off");
  // }
}

// this function uses openWeatherAPI to get readings of the outside weather
void outsideWeather() {
  // Construct the URL with variables
  String request = "/data/2.5/weather?lat=" + String(lat) + "&lon=" + String(lng) + "&units=metric&appid=" + String(weatherAPI);

  // Make a GET request to OpenWeatherMap API
  if (client.get(request) != 0) {
  Serial.println("Failed to connect to OpenWeatherMap API");
  return;
  }

  //Serial.print("API Request URL: ");
  //Serial.println(request);

  // Read the response
 // Serial.println("HTTP Response Code: " + String(client.responseStatusCode()));
  String response = client.responseBody();

  // Parse JSON
  const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + 270;
  DynamicJsonDocument doc(capacity);
 // Serial.println("Attempting JSON parsing...");
  deserializeJson(doc, response);
  //Serial.println("JSON parsing complete.");

 // Serial.println("API Response:");
  //Serial.println(response);

  // Extract temperature, humidity, and pressure
  outTemperature = doc["main"]["temp"];
  outHumidity = doc["main"]["humidity"];
  outPressure = doc["main"]["pressure"];

  // Print the values
  // Serial.print("Outdoor Temperature: ");
  // Serial.println(outTemperature);
  // Serial.print("Outdoor Humidity: ");
  // Serial.println(outHumidity);
  // Serial.print("Outdoor Pressure: ");
  // Serial.println(outPressure);
  
  Serial.println("Outdoor readings collected");
}

void insideWeather() {
 // read the sensor values
  inTemperature = carrier.Env.readTemperature();
  inHumidity = carrier.Env.readHumidity();
  inPressure = carrier.Pressure.readPressure();

}


void loop() {

  outsideWeather();

  insideWeather();

  // set the fields with the values
  ThingSpeak.setField(1, inTemperature);
  ThingSpeak.setField(2, inHumidity);
  ThingSpeak.setField(3, inPressure);
  ThingSpeak.setField(4, outTemperature);
  ThingSpeak.setField(5, outHumidity);
  ThingSpeak.setField(6, outPressure);

  ThingSpeak.setField(7, deviceStatus);

 // repeat background for new inputs
  carrier.display.fillScreen(0x0000);

  // Display text near each button
  displayTemperatureWithCelsius(inTemperature, 30, 70);
  displayButtonReading (inHumidity, 160, 70);
  displayButtonReading (inPressure, 160, 150);
  displayButtonText (devicePower, 110, 200); //bottom button
  // displayButtonReading (0, 30, 150);


  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
   if(x == 200){
    Serial.println("Channel update successful.");

  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }

// updates buttons status
  carrier.Buttons.update();

  // Checks if button 02 is touched
  if (carrier.Buttons.onTouchDown(TOUCH2)) {
    buttonPressed = true;
    Serial.println("Button 2 pressed down");
    buttonStartTime = millis();
    deviceStatus = 1; // turn device on
    ThingSpeak.setField(7, deviceStatus); // send to think speak
    if (deviceStatus == 1) {
      strcpy(devicePower, "ON"); //change display text
    }
    carrier.Buzzer.sound(131);
    delay(1500);
    Serial.println("Button MADE NOISE");

      // Check if 2 minutes have passed, to turn off device
  if (deviceStatus == 1 && millis() - buttonStartTime >= buttonDuration) {
    deviceStatus = 0;  // Reset the variable after 2 minutes
    ThingSpeak.setField(7, deviceStatus);
    if (deviceStatus == 0) {
      strcpy(devicePower, "OFF"); //change display text back to off
    }
    Serial.println("2 minutes passed. Device OFF");
  }


  }
  


    if (buttonPressed) {
      Serial.println("button stopped press");
      buttonPressed=false;
      carrier.Buzzer.noSound(); //Turn off the buzzer before it drives everbody mad.
      delay(15000);
    }

  delay(INTERVAL);

  }