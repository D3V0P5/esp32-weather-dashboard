/**
 * Project: ESP32-C3 Super Mini Weather Dashboard
 * Hardware Pinout Reference (AHT20 + BMP280 I2C Module):
 * ----------------------------------------------------
 * ESP32-C3 Pin  |  Module Pin  |  Description
 * ----------------------------------------------------
 * 3.3V          |  VCC         |  Power Supply (Do NOT use 5V)
 * GND           |  GND         |  Common Ground
 * GPIO 4        |  SDA         |  I2C Data Line
 * GPIO 5        |  SCL         |  I2C Clock Line
 * ----------------------------------------------------
 */

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
#include "secrets.h"
#include "index.h"

AsyncWebServer server(80);
Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

bool aht_connected = false;
bool bmp_connected = false;

String internetWeatherJson = "{\"status\":\"fetching...\"}";
unsigned long lastFetchTime = 0;
const unsigned long FETCH_INTERVAL = 600000;

void fetchInternetWeather() {
    if (WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    String url = "https://api.open-meteo.com/v1/forecast?latitude=" + String(WEATHER_LAT) + 
                 "&longitude=" + String(WEATHER_LON) + 
                 "&current=temperature_2m,relative_humidity_2m,wind_speed_10m,weather_code" +
                 "&daily=weather_code,temperature_2m_max,temperature_2m_min&timezone=auto";

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
        String payload = http.getString();
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload);
        
        if (!error) {
            JsonDocument outDoc;
            outDoc["current"]["external_temp"] = doc["current"]["temperature_2m"];
            outDoc["current"]["external_humidity"] = doc["current"]["relative_humidity_2m"];
            outDoc["current"]["wind_speed"] = doc["current"]["wind_speed_10m"];
            outDoc["current"]["weather_code"] = doc["current"]["weather_code"];

            JsonArray dailyTime = doc["daily"]["time"];
            JsonArray dailyMax = doc["daily"]["temperature_2m_max"];
            JsonArray dailyMin = doc["daily"]["temperature_2m_min"];
            JsonArray dailyCode = doc["daily"]["weather_code"];

            JsonArray outDaily = outDoc["daily"].to<JsonArray>();
            for (size_t i = 0; i < dailyTime.size() && i < 5; i++) {
                JsonObject dayObj = outDaily.add<JsonObject>();
                dayObj["date"] = dailyTime[i];
                dayObj["max"] = dailyMax[i];
                dayObj["min"] = dailyMin[i];
                dayObj["code"] = dailyCode[i];
            }
            
            serializeJson(outDoc, internetWeatherJson);
        }
    }
    http.end();
}

void setup() {
    delay(3000);
    Serial.begin(115200);
    Serial.println("\nBooting ESP32 Weather Station...");

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("\nWiFi connected successfully!");
    Serial.print("Dashboard IP Address: http://");
    Serial.println(WiFi.localIP());

    Wire.begin(4, 5);
    aht_connected = aht.begin();
    bmp_connected = bmp.begin(0x76); 

    fetchInternetWeather();
    lastFetchTime = millis();

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send_P(200, "text/html", index_html);
    });

    server.on("/data", HTTP_GET, [](AsyncWebServerRequest *request){
        JsonDocument doc;

        if (aht_connected) {
            sensors_event_t humidity, temp_aht;
            aht.getEvent(&humidity, &temp_aht);
            doc["local"]["temperature"] = temp_aht.temperature;
            doc["local"]["humidity"] = humidity.relative_humidity;
        } else {
            doc["local"]["temperature"] = serialized("null");
            doc["local"]["humidity"] = serialized("null");
        }

        if (bmp_connected) {
            doc["local"]["pressure"] = bmp.readPressure() / 100.0F;
        } else {
            doc["local"]["pressure"] = serialized("null");
        }

        JsonDocument extDoc;
        deserializeJson(extDoc, internetWeatherJson);
        doc["internet"] = extDoc;

        String jsonString;
        serializeJson(doc, jsonString);
        request->send(200, "application/json", jsonString);
    });

    server.begin();
    Serial.println("HTTP server started.");
}
void loop() {
    if (millis() - lastFetchTime > FETCH_INTERVAL) {
        lastFetchTime = millis();
        fetchInternetWeather();
    }
}