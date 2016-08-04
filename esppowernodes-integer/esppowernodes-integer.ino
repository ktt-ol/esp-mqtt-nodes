/**
 * ESP MQTT node for the electic meters in Mainframe Hackerspace (1000 ticks per kWh).
 *
 * Supports reading last kWh counter from /energy after reboot or to update
 * current value. To update:
 *   mosquitto_pub -h spacegate.mainframe.lan -p 8883 --cafile spacegate.mainframe.lan.crt \
 *    -u user -P password -t /test/stromHinten/energy -m 123456.987
 *
 * Usage:
 * 1. Update/extend settings.h
 * 2. Create a credentials.h with all passwords. See settings.h for
 *  corresponding comments. Do not commit this header file.
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
//#include <ESP8266WiFiMulti.h>
#include <ArduinoOTA.h>
#include <PubSubClient.h>
//#include <WiFiClientSecure.h>
#include <WiFiClient.h>

#include "settings.h"
#include "credentials.h"

#define DEBUG 1

#define TOPIC_ENERGY SENSOR_TOPIC "/energy"  // Wh
#define TOPIC_POWER SENSOR_TOPIC "/power"    // W
#define TOPIC_STATUS SENSOR_TOPIC "/status"  // 'online' or 'offline'
#define TOPIC_RESET SENSOR_TOPIC "/reset"    // Wh
r im
#define debug_println(fmt) \
            do { if (DEBUG) Serial.println(fmt); } while (0)
#define debug_printf(fmt, ...) \
            do { if (DEBUG) Serial.printf(fmt, __VA_ARGS__); } while (0)
//#define MQTT_SOCKET_TIMEOUT 60
//#define MQTT_KEEPALIVE 15
//WiFiClientSecure wclient;
WiFiClient wclient;
//ESP8266WiFiMulti wifiMulti;
PubSubClient client(mqttHost, mqttPort, wclient);

volatile static unsigned long pulses = 0;
volatile static unsigned long power = 0;
volatile static unsigned long lastTick = 0;
volatile static bool changed = 0;

// debounce tickInterrupts. Max. one tick every 100ms (36kW) should be enough)
const long tickDebounce = 100;

static void tickInterrupt() {
  unsigned long now = millis();
  unsigned long diff = now - lastTick;

  if (diff >= tickDebounce) {
    pulses++;
    power = 3600000/diff;
    changed = true;
    lastTick = now;
  }
}

static unsigned long natol(char *s, int n) {
  unsigned long x = 0;
  while(isdigit(s[0]) && n--) {
    x = x * 10 + (s[0] - '0');
    s++;
  }
  return x;
}

// MQTT callback to power/energy value
static void readPowerTicksFromMqtt(const char* topic, byte* payload, unsigned int length) {
 unsigned long newPulses = natol((char*)payload, length);
  debug_printf("restore: %ld\n", newPulses);
  pulses = newPulses;
}

static void initializePowerTicksFromMqtt(const char* topic, byte* payload, unsigned int length) {  
  byte backup[length];
  memcpy(backup,payload,length);
  /* switch topic */
  client.unsubscribe(TOPIC_ENERGY);
  readPowerTicksFromMqtt(topic, backup, length);
  client.setCallback(readPowerTicksFromMqtt);
  if(client.subscribe(TOPIC_RESET)) {
    debug_println("reset subscription sucessfull");
  } else {
    debug_println("reset subscription unsucessfull");
  }

  
}

static void mqttPublish() {
  client.publish(TOPIC_ENERGY, String(pulses).c_str(), true);
  client.publish(TOPIC_POWER, String(power).c_str());
  debug_println("Sent data to mqtt Server");
}

static void connectMQTT() {
  debug_println("Connecting to MQTT server");
  debug_printf("ErrorCode: %ld\n",client.state());
  if (!client.connect(sensorHostname, mqttUser, mqttPassword, TOPIC_STATUS, 2, 1, "offline")) {
    debug_println("Could not connect to MQTT server");
    delay(5000);
    return;
  }

//  if (wclient.verify(mqttFingerprint, mqttHost)) {
//    debug_println("certificate matches");
//  } else {
//    debug_println("certificate doesn't match");
//    client.disconnect();
//    delay(5000);
//    return;
//  }
  client.publish(TOPIC_STATUS, "online", true);

  // subscribe to energy counter to get last counter on (re)boot
  client.setCallback(initializePowerTicksFromMqtt);
  if(client.subscribe(TOPIC_ENERGY)) {
    debug_println("energy subscription sucessfull");
  } else {
    debug_println("energy subscription unsucessfull");
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(PWRPIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PWRPIN), tickInterrupt, FALLING);

  // Add wifi networks. wifiMulti supports multiple networks.
  // and retries to connect to all of them until it succeeds.
  //wifiMulti.addAP(ssid0, ssid0Password);
  //wifiMulti.addAP(ssid1, ssid1Password);
  WiFi.begin(ssid0, ssid0Password);
  //while(wifiMulti.run() != WL_CONNECTED){
  //  debug_println("Not Connected");
  //  delay(500);
  //}

  // Initialize OTA updates. Must `begin` after wifiMulti.run.
  // OTA does not work reliable with all ESP boards (WiMos works well).
  // Restart of Arduino IDE might be required before node appears.
  ArduinoOTA.setHostname(sensorHostname);
  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.begin();
}

void loop() {
//  if(wifiMulti.run() != WL_CONNECTED){
//    debug_println("Reconnecting Wifi...");
//    delay(1000);
//    return;
//  }

  ArduinoOTA.handle();

  if (!client.connected())
    connectMQTT();

  if (client.connected()) {
    client.loop();
    if (changed) {
      if (pulses > 1000) {
        debug_printf("Pushing: %ld\n", pulses);
        mqttPublish();
      }

      changed = false;
    }
  }
}

