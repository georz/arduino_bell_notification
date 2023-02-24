#include "M5Atom.h"
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

HTTPClient http;

const char* WIFI_SSID = "-- SSID --";
const char* WIFI_PASSWORD = "-- PASSWORD --";

const unsigned int DELAY_WIFI_CONNECT = 1000;
const unsigned int INTERVAL_WIFI_RECONNECT = 15000;
const unsigned int MAX_RETRY_WIFI_RECONNECT = 5;

const unsigned int ANALOG_READ_PIN = 33;
const unsigned int ANALOG_READ_THRESHOLD = 3000; // 0-4095

const unsigned int DELAY_ANALOG_READ_DEFAULT = 20000;
const unsigned int DELAY_ANALOG_READ_MIN = 1000;
const unsigned int DELAY_ANALOG_READ_MAX = 60000;

const char* HTTP_GET_URL = "http://-- WebServer --/cgi-bin/bell_notification.cgi";
const unsigned int HTTP_CONNECT_TIMEOUT = 3000;
const unsigned int HTTP_READ_TIMEOUT = 5000;

const unsigned int DELAY_LOOP = 200;

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  //Serial.println("Connecting to WiFi");

  unsigned long previousMillis = millis();
  unsigned int retryCount = 0;
  bool isConnecting = true;

  while (isConnecting) {
    //Serial.print(". ");
    delay(DELAY_WIFI_CONNECT);

    if ((WiFi.status() == WL_CONNECTED)) {
      isConnecting = false;
    } else {
      unsigned long currentMillis = millis();
      if ((currentMillis - previousMillis >= INTERVAL_WIFI_RECONNECT)) {
        if (retryCount >= MAX_RETRY_WIFI_RECONNECT) {
          //Serial.println("ESP.restart");
          ESP.restart();
        }
        WiFi.disconnect();
        WiFi.reconnect();
        retryCount += 1;
        previousMillis = currentMillis;
        //Serial.printf("Reconnecting to WiFi [retryCount: %d]\n", retryCount);
      }
    }
  }
  //Serial.println(WiFi.localIP());
}

void setup() {
  M5.begin(true, false, true);
  //Serial.begin(9600);
  connectWiFi();
}

void loop() {
  static unsigned long delay_analog_read = DELAY_ANALOG_READ_DEFAULT;

  if ((WiFi.status() == WL_CONNECTED)) {
    uint16_t analog_read_value = analogRead(ANALOG_READ_PIN);
    if (analog_read_value > ANALOG_READ_THRESHOLD) {
      http.begin(HTTP_GET_URL);
      http.setConnectTimeout(HTTP_CONNECT_TIMEOUT);
      http.setTimeout(HTTP_READ_TIMEOUT);

      int httpCode = http.GET();
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        payload.trim();
        delay_analog_read = payload.toInt();

        if ((delay_analog_read < DELAY_ANALOG_READ_MIN) || (delay_analog_read > DELAY_ANALOG_READ_MAX)) {
          delay_analog_read = DELAY_ANALOG_READ_DEFAULT;
        }
        //Serial.printf("delay_analog_read: %ld\n", delay_analog_read);
      }
      http.end();
      delay(delay_analog_read);
    }
  } else {
    //Serial.println("Reconnecting to WiFi");
    WiFi.disconnect();
    connectWiFi();
  }
  delay(DELAY_LOOP);
}