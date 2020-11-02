# Smart Home Garage Door
A small module that can be added to an old garage door system which will allow for integration with Home Assistant using MQTT.

## How It Works
The code uses MQTT in order to communicate with the Home Assistant Server. To connect, it uses MQTT AutoDiscovery. You'll just have to connect the Particle Photon to the wifi, enable MQTT AutoDiscovery, and restart your Home Assistant server. For more information, please check [this link](https://www.home-assistant.io/docs/mqtt/discovery/).
Since the garage door module does not include any sensor for whether the garage door is open or closed, the stop button has been repurposed to work as a state switcher. Other than that, it is very similar to a normal smart garage door opener. 

## Hardware Required
- Particle Photon (ESP8266 Code will be added later)
- Relay

## Wiring
Wiring instructions will be added later.
