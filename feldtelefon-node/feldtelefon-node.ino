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
//#include <ArduinoOTA.h>
#include <PubSubClient.h>
//#include <WiFiClientSecure.h>
#include <WiFiClient.h>

#include "settings.h"
#include "credentials.h"

#define DEBUG 1

#define TOPIC_RINGING SENSOR_TOPIC "/ringing"  // true or false
#define TOPIC_STATUS SENSOR_TOPIC "/status" //online or offline
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
bool ringing =false;
static void ringInterrupt() {
  ringing=true;
  debug_println("INNNTTTEEERRRUUPPPTTT :D");
  client.publish(TOPIC_RINGING, "true", true);
  debug_println("HIGH");
  debug_println("Sent data to mqtt Server");
}

static unsigned long natol(char *s, int n) {
  unsigned long x = 0;
  while(isdigit(s[0]) && n--) {
    x = x * 10 + (s[0] - '0');
    s++;
  }
  return x;
}

static void connectMQTT() {
  debug_println("Connecting toint MQTT server");
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
//  }CHANGE
  client.publish(TOPIC_STATUS, "online", true);

  // subscribe to energy counter to get last counter on (re)boot
 // client.setCallback(initializePowerTicksFromMqtt);
  //if(client.subscribe(TOPIC_RINGING)) {
    //debug_println("Ringing subscription sucessfull");
//  } else {
    //debug_println("Ringing subscription unsucessfull");
//  }
}

void setup() {
  Serial.begin(115200);
  delay(100);

  pinMode(4, INPUT);
  
  //attachInterrupt(digitalPinToInterrupt(RINGPIN), ringInterrupt, FALLING);
  //attachInterrupt(4, ringInterrupt, RISING);

  // Add wifi networks. wifiMulti supports multiple networks.
  // and retries to connect to all of them until it succeeds.
  //wifiMulti.addAP(ssid0, ssid0Password);
  //wifiMulti.addAP(ssid1, ssid1Password);
  WiFi.begin(ssid0, ssid0Password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  //while(wifiMulti.run() != WL_CONNECTED){
    //debug_println("Not Connected");
    //delay(500);
  //}

  // Initialize OTA updates. Must `begin` after wifiMulti.run.
  // OTA does not work reliable with all ESP boards (WiMos works well).
  // Restart of Arduino IDE might be required before node appears.
  ArduinoOTA.setHostname(sensorHostname);
  ArduinoOTA.setPassword(otaPassword);
  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();
  //client.publish(TOPIC_RINGING, String(a++).c_str(), true);debug_println("HIGH");
  //debug_println(String(a).c_str());
  if (!client.connected()){
    debug_printf("ErrorCode: %ld\n",client.state());
    connectMQTT();
    debug_printf("ErrorCode: %ld\n",client.state());
  } client.loop();
  if (digitalRead(4)==HIGH && !ringing) {
    ringing=true;
    client.publish(TOPIC_RINGING, "true", true);
    debug_println("HIGH");
    debug_println("Sent data to mqtt Server");
  } else if(digitalRead(4)==LOW && ringing) {
    ringing = LOW;
    client.publish(TOPIC_RINGING, "false", true);
    debug_println("LOW");
    debug_println("Sent data to mqtt Server");
  }
  //if(wifiMulti.run() != WL_CONNECTED) {
    //Serial.println("WiFi not connected!");
    //delay(1000);
  //}

}

