# MC-MQTT-sensor-joystick
ESP32 based sensor for joystick, it sends events via MQTT to Minecraft.
# Description
Reads analog inputs connected to the joystick (X and Y axis measured via potentiomenters) and sends the measurements to the configured MQTT broker. Use https://github.com/jsitera/MC-RCON-MQTT to import the MQTT messages to a Minecraft server.
# Implementation comments
- The code is prepared for ESP32. ESP8266 have only one analog input.
