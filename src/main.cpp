
#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Update.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

const char* currentVersion = "v1.0.0";
const char* versionURL = "https://raw.githubusercontent.com/duyhpe/esp32-firmware-platformio/main/version.txt";
const char* firmwareBaseURL = "https://raw.githubusercontent.com/duyhpe/esp32-firmware-platformio/main/firmware/";

#define LED_PIN 2
bool ledState = false;
unsigned long lastBlink = 0;

void blinkWhileOTA() {
    if (millis() - lastBlink > 500) {
        ledState = !ledState;
        digitalWrite(LED_PIN, ledState);
        lastBlink = millis();
    }
}

void checkAndUpdate() {
    HTTPClient http;
    http.begin(versionURL);
    int httpCode = http.GET();
    if (httpCode == 200) {
        String newVersion = http.getString();
        newVersion.trim();
        if (newVersion != currentVersion) {
            String firmwareURL = String(firmwareBaseURL) + "firmware_" + newVersion + ".bin";
            http.end();
            http.begin(firmwareURL);
            int code = http.GET();
            if (code == 200) {
                int contentLen = http.getSize();
                WiFiClient client;
                if (Update.begin(contentLen)) {
                    int written = Update.writeStream(http.getStream());
                    if (written == contentLen && Update.end()) {
                        ESP.restart();
                    }
                }
            }
        }
    }
    http.end();
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    Serial.begin(115200);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        blinkWhileOTA();
        delay(100);
    }
    Serial.println("WiFi connected!");
    checkAndUpdate();
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(500);
    digitalWrite(LED_PIN, LOW);
    delay(500);
}
