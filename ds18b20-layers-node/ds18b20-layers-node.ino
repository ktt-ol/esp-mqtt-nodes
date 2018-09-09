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

#define ONE_WIRE_BUS D1
#define TEMPERATURE_PRECISION 12

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

DeviceAddress level1 = { 0x28, 0xFF, 0x0B, 0xF6, 0x52, 0x17, 0x04, 0x97 };
DeviceAddress level2 = { 0x28, 0xFF, 0xE6, 0x04, 0x53, 0x17, 0x04, 0x7C };
DeviceAddress level3 = { 0x28, 0xFF, 0x70, 0x51, 0x52, 0x17, 0x04, 0xA5 };
DeviceAddress level4 = { 0x28, 0xFF, 0xD5, 0xA3, 0x51, 0x17, 0x04, 0xD6 };
DeviceAddress level5 = { 0x28, 0xFF, 0x18, 0xBE, 0x51, 0x17, 0x04, 0xBD };
DeviceAddress level6 = { 0x28, 0xFF, 0xE1, 0x1C, 0x53, 0x17, 0x04, 0x09 };
DeviceAddress level7 = { 0x28, 0xFF, 0xC6, 0x9D, 0x52, 0x17, 0x04, 0x5D };

void setup() {
    Serial.begin(115200);
    delay(100);

    Serial.println();

    // Initialize sensors. For example:
    // dht11.begin();
    // <------------>
    DS18B20.begin();

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
    DS18B20.setResolution(level1, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level2, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level3, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level4, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level5, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level6, TEMPERATURE_PRECISION);
    DS18B20.setResolution(level7, TEMPERATURE_PRECISION);

    Serial.print("Device 1 Resolution: ");
  Serial.print(DS18B20.getResolution(level1), DEC);
  Serial.println();

  Serial.print("Device 2 Resolution: ");
  Serial.print(DS18B20.getResolution(level2), DEC);
  Serial.println();

  Serial.print("Device 3 Resolution: ");
  Serial.print(DS18B20.getResolution(level3), DEC);
  Serial.println();

  Serial.print("Device 4 Resolution: ");
  Serial.print(DS18B20.getResolution(level4), DEC);
  Serial.println();

  Serial.print("Device 5 Resolution: ");
  Serial.print(DS18B20.getResolution(level5), DEC);
  Serial.println();

  Serial.print("Device 6 Resolution: ");
  Serial.print(DS18B20.getResolution(level6), DEC);
  Serial.println();

  Serial.print("Device 7 Resolution: ");
  Serial.print(DS18B20.getResolution(level7), DEC);
  Serial.println();
}

void printData(DeviceAddress deviceAddress)
{
  Serial.print("Device Address: ");
  printAddress(deviceAddress);
  Serial.print(" ");
  printTemperature(deviceAddress);
  Serial.println();
}

void printTemperature(DeviceAddress deviceAddress)
{
  float tempC = DS18B20.getTempC(deviceAddress);
  Serial.print("Temp C: ");
  Serial.print(tempC);
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print("0x");
    Serial.print(deviceAddress[i], HEX);
    Serial.print(", ");
  }
}

bool measureAndPublish() {
    // Read sensors and publish to MQTT
    // Wifi and client are connected when this function is called.
    // Return false when reading failed, to retry faster then the normal
    // update cycle.
    String tempstring;
    DS18B20.requestTemperatures();
    
    printData(level1);
    tempstring=getData_mK(level1);
    client.publish("/test/environment/esp-temp-layers/layer1/temperature",tempstring.c_str());
    Serial.println(tempstring);

    
    printData(level2);
    tempstring=getData_mK(level2);
    client.publish("/test/environment/esp-temp-layers/layer2/temperature",tempstring.c_str());
    Serial.println(tempstring);

    printData(level3);
    tempstring=getData_mK(level3);
    client.publish("/test/environment/esp-temp-layers/layer3/temperature",tempstring.c_str());
    Serial.println(tempstring);

    printData(level4);
    tempstring=getData_mK(level4);
    client.publish("/test/environment/esp-temp-layers/layer4/temperature",tempstring.c_str());
    Serial.println(tempstring);
    
    printData(level5);
    tempstring=getData_mK(level5);
    client.publish("/test/environment/esp-temp-layers/layer5/temperature",tempstring.c_str());
    Serial.println(tempstring);
    
    printData(level6);
    tempstring=getData_mK(level6);
    client.publish("/test/environment/esp-temp-layers/layer6/temperature",tempstring.c_str());
    Serial.println(tempstring);
    
    printData(level7);
    tempstring=getData_mK(level7);
    client.publish("/test/environment/esp-temp-layers/layer7/temperature",tempstring.c_str());
    Serial.println(tempstring);
    true;
}

unsigned long lastUpdate = 0;

String getData_mK(DeviceAddress deviceAddress)
{
    float tempC=DS18B20.getTempC(deviceAddress);
    long tempCl=tempC*1000+273150;
    String tempCs = String(tempCl);
    return tempCs;
}

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

