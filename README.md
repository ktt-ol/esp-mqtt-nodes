ESP MQTT Nodes
==============

This repository contains various Arduino sketches for ESP8266 nodes.
All sketches read some sensor data and publish the results to an MQTT server,
or display something received from it.

Most configurations contain sensible defaults for the Mainframe hackerspace.

Features
--------

- Support for multiple Wifi networks with WifiMulti
- Automatic reconnect
- `delay`-free for reliable Wifi/MQTT connections
- External `settings.h` and `credentials.h` for easy configuration


Nodes
-----

`dht-node`: Reads temperature and humidity from DHT sensors.

`power-node`: Reads ticks from an electric meter, calculates current
    power consumption and published this and the counted meter value.

`status-node`: Reads the Mainframe's opening status and signals it via
    non-SPI RGB LED's (or stripes)

Create new nodes
----------------

`template` is a commented template for a new node.
