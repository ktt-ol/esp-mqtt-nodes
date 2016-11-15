/**
 * ESP MQTT sensornode example.
 *
 * Example ESP node that reads sensors and publishes the data to
 * MQTT.
 *
 * Usage:
 *
 * 1. Update all <------------> parts with your sensor specific code.
 * 2. Update/extend settings.h
 * 3. Create a credentials.h with all passwords. See settings.h for
 *    corresponding comments. Do not commit this header file.
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "settings.h"
#include "credentials.h"

ESP8266WiFiMulti wifiMulti;

WiFiClientSecure wclient;
PubSubClient client(mqttHost, mqttPort, wclient);

#define ONE_WIRE_BUS 2

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);


void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println();

    // Initialize sensors. For example:
    // dht11.begin();
    // <------------>

    // Add wifi networks. wifiMulti supports multiple networks.
    // and retries to connect to all of them until it succeeds.
    wifiMulti.addAP(ssid0, ssid0Password);
    wifiMulti.addAP(ssid1, ssid1Password);

    // Wait till connected.
    while(wifiMulti.run() != WL_CONNECTED){
      Serial.println("WiFi connection failed, retrying.");
      delay(500);
    }

    // Initialize OTA updates. Must `begin` after wifiMulti.run.
    // OTA does not work reliable with all ESP boards (WiMos works well).
    // Restart of Arduino IDE might be required before node appears.
    ArduinoOTA.setHostname(sensorHostname);
    ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();
}


bool measureAndPublish() {
    // Read sensors and publish to MQTT
    // Wifi and client are connected when this function is called.
    // Return false when reading failed, to retry faster then the normal
    // update cycle.

    // <------------>
    DS18B20.requestTemperatures();
    float temp = (DS18B20.getTempCByIndex(0)*1000)+273150;
    //int tempInt = temp
    // Send to MQTT as float with no decimal.o
    client.publish(mqttSensorTopic, String(temp, 0).c_str(), true);
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

