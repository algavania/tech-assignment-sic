#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <WiFi.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// TODO: change it to your own WiFi credentials
const char* ssid = "ssid"; 
const char* password = "password";

// TODO: change it to your own IP Address
String baseUrl = "http://192.168.39.94:5000/api";
String sensorApiUrl = baseUrl + "/sensors";
unsigned long int lastTimestamp = 0;

void setup() {
  Serial.begin(115200);

  dht.begin();
  WiFi.begin(ssid, password);

  Serial.print("Connecting to WiFi");

  checkAndConnectWifi();

  Serial.print("WiFi Connected");

}

void loop() {
  // Always recheck the wifi, if it's disconnected, then re-connect it
  checkAndConnectWifi();

  int now = millis();
  // Send data every 5 seconds
  if (now - lastTimestamp < 5000) return;
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  // Check if any reads failed and exit early (to try again).
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.println("Now: " + String(now));
  Serial.println("Temperature: " + String(temperature));
  Serial.println("Humidity: " + String(humidity));

  // Send sensors data to API
  HTTPClient http;
  http.begin(sensorApiUrl);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["time"] = now;
  String requestBody;
  serializeJson(doc, requestBody);

  Serial.println("Sending data to: " + sensorApiUrl);
  Serial.println("Data to be sent: " + requestBody);

  int httpResponseCode = http.POST(requestBody);
  // Check if HTTP request succeeded or failed
  if (httpResponseCode > 0) {
    String response = http.getString();
    lastTimestamp = millis();
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.print("Response: ");
    Serial.println(response);
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  delay(5000);
}

void checkAndConnectWifi() {
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
}
