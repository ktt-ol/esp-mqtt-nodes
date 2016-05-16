/**
 * EPS MQTT node for DHT sensors
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <ESP8266HTTPClient.h>
#include <DHT.h>

#include "settings.h"
#include "credentials.h"

ESP8266WiFiMulti wifiMulti;

WiFiClientSecure wclient;
PubSubClient client(mqttHost, mqttPort, wclient);

DHT dht11(D3, DHT11);
DHT dht22(D4, DHT22);

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println();

    dht11.begin();
    dht22.begin();

    wifiMulti.addAP(ssid0, ssid0Password);
    wifiMulti.addAP(ssid1, ssid1Password);

    while(wifiMulti.run() != WL_CONNECTED){
      Serial.println("WiFi connection failed, retrying.");
      delay(500);
    }

    ArduinoOTA.setHostname(sensorHostname);
    ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();
}


bool measureAndPublish() {
    float t[2] = {dht11.readTemperature(), dht22.readTemperature()};
    float h[2] = {dht11.readHumidity(), dht22.readHumidity()};

    if (isnan(t[0]) || isnan(h[0])) {
        Serial.print("[DHT11] no temperature or humidity\n");
        return false;
    }
    if (isnan(t[1]) || isnan(h[1])) {
        Serial.print("[DHT22] no temperature or humidity\n");
        return false;
    }

    Serial.print("[DHT11]");
    Serial.print(t[0]);
    Serial.print(" ");
    Serial.print(h[0]);
    Serial.print("\n");

    Serial.print("[DHT22]");
    Serial.print(t[1]);
    Serial.print(" ");
    Serial.print(h[1]);
    Serial.print("\n");

    client.publish("/test/hackathon-sensors/dht11/temp", String(t[0]).c_str());
    client.publish("/test/hackathon-sensors/dht11/humidity", String(h[0]).c_str());
    client.publish("/test/hackathon-sensors/dht22/temp", String(t[1]).c_str());
    client.publish("/test/hackathon-sensors/dht22/humidity", String(h[1]).c_str());
    Serial.println("Published");
    return true;
}

unsigned long lastUpdate = 0;

void loop() {
    if(wifiMulti.run() != WL_CONNECTED) {
        Serial.println("WiFi not connected!");
        delay(1000);
        return;
    }
    // else connected

    ArduinoOTA.handle();

    if (!client.connected()) {
        Serial.println("Connecting to MQTT server");
        if (client.connect(sensorHostname, mqttUser, mqttPassword)) {
          Serial.println("Connected to MQTT server, checking cert");
          if (wclient.verify(mqttFingerprint, mqttHost)) {
            Serial.println("certificate matches");
          } else {
            Serial.println("certificate doesn't match");
            client.disconnect();
            delay(10000);
            return;
          }
        } else {
          Serial.println("Could not connect to MQTT server");
          delay(1000);
          return;
        }
    }

    if (client.connected()) {
        client.loop();
        if (millis() - lastUpdate >= updateInterval) {
            if (measureAndPublish()) {
                lastUpdate = millis();
            } else {
                // retry earlier on error
                lastUpdate = millis() - updateInterval + retyInterval;
            }
        }
    }
}

