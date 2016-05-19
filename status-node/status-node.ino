/**
 * ESP MQTT status signal example.
 *
 * Example ESP node that reads the space status and signals it using
 * a non-SPI RGB LED (stripe). Could be used to show "Radstelle"
 * status as well.
 *
 * Usage:
 *
 * 1. Update/extend settings.h
 * 2. Create a credentials.h with all passwords. See settings.h for
 *    corresponding comments. Do not commit this header file.
 */

#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <PubSubClient.h>

#include "settings.h"
#include "credentials.h"

ESP8266WiFiMulti wifiMulti;
WiFiClientSecure wclient;
PubSubClient client(mqttHost, mqttPort, wclient);

enum status {
  opened,
  closed,
  closing,
  unknown
};
status space_status = unknown;

// Memorize current values for better fading
int red = 0;
int green = 0;
int blue = 0;

// Fancy animation signalizing startup (looks best on stripes)
// Handy for recognizing (unwanted) device restarts
void greeting_animation(){
  for(int iterations = 0; iterations< 3; iterations++){
    for(int fading = 0; fading < 255; fading+=10){
      analogWrite(PIN_RED, fading);
      analogWrite(PIN_BLUE, fading);
      delay(ANIMATIONSPEED);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_BLUE, 0);
      delay(ANIMATIONSPEED);
    }
    for(int fading = 255; fading > 0; fading-=10){
      analogWrite(PIN_RED, fading);
      analogWrite(PIN_BLUE, fading);
      delay(ANIMATIONSPEED);
      analogWrite(PIN_RED, 0);
      analogWrite(PIN_BLUE, 0);
      delay(ANIMATIONSPEED);
    }
  }
}

// Fade every pin to zero, used to switch between states
void fade_to_blank(){
  // Fade to zero while making sure all pins hit zero at the same time.
  // This special treatment is necessary for colors mixed with colour values other than 0 or 255.
  for (int i = 255; i > 0; i--){
    if (red == i){
      red--;
      analogWrite(PIN_RED, red);
    }
    if (green == i){
      green--;
      analogWrite(PIN_GREEN, green);
    }
    if (blue == i){
      blue--;
      analogWrite(PIN_BLUE, blue);
    }
    delay(FADESPEED);
  }
}

void open_space(){
  space_status = opened;
  Serial.println("Space is open.");

  // Fade blank, then to green
  fade_to_blank();
  for (; green < 255; green++){
    analogWrite(PIN_GREEN, green);
    delay(FADESPEED);
  }
}

void close_space(){
  space_status = closed;
  Serial.println("Space closes.");

  // Fade blank, then to red
  fade_to_blank();
  for (; red < 255; red++){
    analogWrite(PIN_RED, red);
    delay(FADESPEED);
  }
}

void closing_space_soon(){
  space_status = closing;
  Serial.println("Space is closing.");

  // Fade blank, then to yellow
  // This is a special yellow. Fading green to 255 did not look very yellow,
  // more like green. Therefore the custom mixing.
  fade_to_blank();
  for (int i = 0; i < 255; i++){
    if (green < 200){
      green++;
      analogWrite(PIN_GREEN, green);
    }
    if (red < 255){
      red++;
      analogWrite(PIN_RED, red);
    }
    delay(FADESPEED);
  }
}

void unknown_status(){
  space_status = unknown;
  Serial.println("Status is unknown.");

  // Fade blank, then to blue
  fade_to_blank();
  for (; blue < 255; blue++){
    analogWrite(PIN_BLUE, blue);
    delay(FADESPEED);
  }
}

// callback for subscribed topics
void callback(char* topic, byte* payload, unsigned int length){
  Serial.print(topic);
  Serial.print(" => ");

  String status = String((char *)payload);
  status = status.substring(0, length);
  Serial.println(status);

  // Ignore anything but the space state
  if(String((char *)topic) == statusTopic){
    Serial.println("Topic matches!");
    // Change space status, if necessary
    if(status == "opened"){
      if( space_status != opened){
        open_space();
      }
    }else if(status == "closed"){
      if(space_status != closed){
        close_space();
      }
    }else if(status == "closing"){
      if(space_status != closing){
        closing_space_soon();
      }
    }else{
      if(space_status != unknown)
        unknown_status();
    }
  }
}

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println();

    // Initialize client
    client.setServer(mqttHost, mqttPort);
    client.setCallback(callback);

    // Initialize output pins
    pinMode(PIN_RED, OUTPUT);
    pinMode(PIN_GREEN, OUTPUT);
    pinMode(PIN_BLUE, OUTPUT);

    greeting_animation();

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
    ArduinoOTA.setHostname(signalHostname);
    ArduinoOTA.setPassword(otaPassword);
    ArduinoOTA.begin();
}


void loop() {
    if(wifiMulti.run() != WL_CONNECTED){
        unknown_status();
        Serial.println("WiFi not connected!");
        delay(1000);
        return;
    }

    ArduinoOTA.handle();

    if (!client.connected()){
      unknown_status();
      Serial.println("Connecting to MQTT server...");
      if (client.connect(signalHostname)){
        Serial.println("Connected to MQTT server, checking cert...");
        if (wclient.verify(mqttFingerprint, mqttHost)){
          Serial.println("Certificate matches!");
        }else{
          Serial.println("Certificate doesn't match!");
          unknown_status();
          delay(60000);
          return;
        }
        client.subscribe(statusTopic);
      }else{
        Serial.println("Could not connect to MQTT server.");
        delay(2000);
      }
    }

    if (client.connected()){
      client.loop();
    }
}

