# Smart Home Garage Door
A small module that can be added to an old garage door system which will allow for integration with Home Assistant using MQTT.

## How It Works
The code uses MQTT in order to communicate with the Home Assistant Server. To connect, it uses MQTT AutoDiscovery. You'll just have to connect the Particle Photon to the wifi, enable MQTT AutoDiscovery, and restart your Home Assistant server. For more information, please check [this link](https://www.home-assistant.io/docs/mqtt/discovery/).
Since the garage door module does not include any sensor for whether the garage door is open or closed, the stop button has been repurposed to work as a state switcher. Other than that, it is very similar to a normal smart garage door opener. 

## Example lovelace card
![image](https://user-images.githubusercontent.com/29025984/114292054-efbf7180-9a59-11eb-8362-9a7d227523e8.png)

## Hardware Required
- Particle Photon (ESP8266 Code will be added later)
- Relay
- Pre-existing garage door opener (only tested with Chamberlain, but should work with anything else)

## Wiring
There are two parts of wiring. The first part is connecting the relay to the ESP8266 / Particle Photon. The second part is connecting the relay to the garage door. 

### First Part
TODO
