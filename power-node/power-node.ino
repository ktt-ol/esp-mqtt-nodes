/**
 * ESP MQTT node for the electic meters in Mainframe Hackerspace.
 *
 * Supports reading last kWh counter from /cum after reboot or to update
 * current value. To update:
 *     mosquitto_pub -h spacegate.mainframe.lan -p 8883 --cafile spacegate.mainframe.lan.crt \
 *        -u user -P password -t /test/stromHinten/cum -m 123456.987
 *
 * Usage:
 * 1. Update/extend settings.h
 * 2. Create a credentials.h with all passwords. See settings.h for
 *    corresponding comments. Do not commit this header file.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>

#include "settings.h"
#include "credentials.h"


WiFiClientSecure wclient;
ESP8266WiFiMulti wifiMulti;
PubSubClient client(mqttHost, mqttPort, wclient);


volatile float kwhCounter = 0.0;
volatile unsigned long pulses = 0;
volatile bool changed = 0;
volatile unsigned long lastChange = 0;

// debounce tickInterrupts. Max. one tick every 100m (36kW) should be enough)
unsigned long lastTick = 0;
const long tickDebounce = 100;

void tickInterrupt() {
    if (millis() - lastTick >= tickDebounce) {
       pulses++;
       kwhCounter += 0.001;
       changed = true;
       lastTick = millis();
    }
}

// Convert length encoded string to String
String sconvert(const char *pCh, int arraySize){
    String str;
    if (pCh[arraySize-1] == '\0') {
        str+=pCh;
    } else {
        for(int i=0; i<arraySize; i++) {
            str+=pCh[i];
        }
    }
    return str;
}

// MQTT callback to power/cum value
void readPowerCumFromMqtt(const char* topic, byte* payload, unsigned int length){
    float newCounter = sconvert((char*)payload,length).toFloat();
    if (abs(newCounter - kwhCounter) > 0.1) {
        // only update if change is larger to avoid updating
        // with own value
        Serial.print("restore: ");
        Serial.println(kwhCounter, 3);
        kwhCounter = newCounter;
    }
}


void mqttPublish(float power, float counter, int pulses){
    client.publish(SENSOR_TOPIC "/cum", String(counter, 3).c_str(), true);
    client.publish(SENSOR_TOPIC "/power", String(power, 3).c_str());
    client.publish(SENSOR_TOPIC "/statusPulses", String(pulses).c_str());
    Serial.println("Sent data to mqtt Server");
}

void setup()   {
    Serial.begin(115200);
    delay(100);

    pinMode(PWRPIN, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(PWRPIN), tickInterrupt, FALLING);

    // set
    client.setCallback(readPowerCumFromMqtt);

    // Add wifi networks. wifiMulti supports multiple networks.
    // and retries to connect to all of them until it succeeds.
    wifiMulti.addAP(ssid0, ssid0Password);
    wifiMulti.addAP(ssid1, ssid1Password);

    while(wifiMulti.run() != WL_CONNECTED){
        Serial.println("Not Connected");
        delay(500);
    }

    // Initialize OTA updates. Must `begin` after wifiMulti.run.
    // OTA does not work reliable with all ESP boards (WiMos works well).
    // Restart of Arduino IDE might be required before node appears.
    ArduinoOTA.setHostname(sensorHostname);
    ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();
}

void loop() {
    if(wifiMulti.run() != WL_CONNECTED){
        Serial.println("Reconnecting Wifi...");
        delay(1000);
        return;
    }
    // else connected

    ArduinoOTA.handle();
    if (!client.connected()) {
        Serial.println("Connecting to MQTT server");

        if (client.connect(sensorHostname, mqttUser, mqttPassword, SENSOR_TOPIC "/status", 2, 1, "offline")) {
            if (wclient.verify(mqttFingerprint, mqttHost)) {
              Serial.println("certificate matches");
            } else {
              Serial.println("certificate doesn't match");
              client.disconnect();
              delay(5000);
              return;
            }
            //
            client.publish(SENSOR_TOPIC "/status", "online");

            // subscribe to kw counter to get last counter on boot/reset
            if(client.subscribe(SENSOR_TOPIC "/cum")){
                Serial.println("Subscription sucessfull");
            } else {
                Serial.println("Subscription unsucessfull");
            }
        } else {
            Serial.println("Could not connect to MQTT server");
            delay(5000);
        }
    }

    if (client.connected()) {
        client.loop();
        if (changed) {
            unsigned long changetime = millis();
            if (lastChange && kwhCounter > 1) {
                unsigned long diff = changetime-lastChange;
                float diffsec = float(diff)/1000;
                float currentPower = float(3600)/diffsec*0.001;
                Serial.print("Pushing: ");
                Serial.println(kwhCounter, 3);
                mqttPublish(currentPower, kwhCounter, pulses);
            }
            lastChange = changetime;
            changed = false;
        }
    }
}

