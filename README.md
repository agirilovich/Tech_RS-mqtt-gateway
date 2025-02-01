# Tech_RS-mqtt-gateway
Tech is a producer of heat system controllers from Poland. Some of controllers are embedded with RS communication port.
This one ESP32 based gateway read parameters of heating system and transmit them via MQTT broker.
Afteward that data can be used with Home Assistant integrations.

The project is based on information from the Elektroda forum https://www.elektroda.pl/rtvforum/topic2689981-180.html
The main part of class 'CTechManager' is taken from the same forum discussion.

# Used modules

* Any ESP32 microcontroller.
* BC547 transistors. There are many alternatives, such as russian КТ3102
* DC-DC step down module for transforming ~15V from Tech controller to the 5V for ESP32 supply
* RJ12 connector

# Some features

* Auto-discovery is used for automatic configuration of sensor in HomeAssistant and grouping them in a single device entity

![Device schematic](Schematic_Tech-RS-MQTT.png)
