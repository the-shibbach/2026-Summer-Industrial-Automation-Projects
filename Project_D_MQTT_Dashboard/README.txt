Goal: Publish data via a broker from a publisher to a subscriber. In this case, the publisher is the ESP32, the broker is a server called "HiveMQ" (a free broker), and the subscriber is Node-RED. 

I can connect the ESP32 with the broker via WiFi - in this project I used my mobile hotspot. I configured Node-RED by installing a dashboard so it can be easily viewed (see images in this folder). The ESP32 publishes data to an MQTT topic in JSON format.

By using a simple LDR circuit (high voltage is bright light etc.), I used a SP of 3000 and a limit of 500. If the PV deviated from this, then an alarm was raised on the Node-RED dashboard. 

ESP32 publishes the data in JSON format e.g. {"pv":3142...}.