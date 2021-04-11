#include <MQTT.h>

// Credentials for MQTT server
const String mqttServerIP = "MQTT SERVER IP";
const String mqttUser = "INSERT MQTT USER TO BE USED TO CONNECT TO SERVER";
const String mqttPassword = "INSERT MQTT PASSWORD";

// MQTT client
// MQTT("ip address", "port", "keepalive timeout in seconds", "callback method", "max byte size of message")
MQTT client(mqttServerIP, 1883, 20, callback, 1024);

// Timer to send a heartbeat
int heartbeatTimer = 0;

// Boolean that stops two open/close functions sent at once
bool poweringRelay = false;

// Timer for the garage door opening/closing functionality
int timer = 0;

// Relay pin that controls the relay
const int relayPin = 0;

// Remembers the open / close state of the garage door (temp)
bool isClosed = true;

// Resets device when set to true
bool resetFlag = false;

// Timer for sending the current open/close state to the MQTT Broker
int sensorUpdateTimer = 0;

// Integer to remember whether we are opening or closing the garage
// 0: nothing
// 1: opening
// 2: closing
int openClose = 0;

// Open/close functions for garage
int open_garage(String thisIsOnlyForParticle = "");
int close_garage(String thisIsOnlyForParticle = "");

// MQTT Topics
const String configTopic = "homeassistant/cover/garage_door/config";
const String commandTopic = "homeassistant/cover/garage_door/command";
const String availabilityTopic = "homeassistant/cover/garage_door/LWT";
const String stateTopic = "homeassistant/cover/garage_door/state";

// MQTT AutoDiscovery Configuration Payload that lets H.A know the details of the device
const String configurationPayload = "{\"name\":\"Garage Door\",\"device_class\":\"cover\",\"state_topic\":\"homeassistant/cover/garage_door/state\",\"availability_topic\":\"homeassistant/cover/garage_door/LWT\",\"command_topic\":\"homeassistant/cover/garage_door/command\",\"device_class\":\"garage\",\"pl_avail\":\"online\",\"pl_not_avail\":\"offline\",\"stat_open\":\"open\",\"stat_opening\":\"opening\",\"stat_clsd\":\"closed\",\"stat_closing\":\"closing\",\"unique_id\":\"ASG11_1\",\"device\":{\"ids\":[\"ASG11\"],\"mf\":\"Aritro Studios\",\"sw\":\"1.0.0\",\"mdl\":\"Garage Door v1.0.0\",\"name\":\"Garage Door\"}}";

// This is called when a message is received.
void callback(char* topic, byte* payload, unsigned int length) 
{
    String msg = "";
    
    for (int i = 0; i < length; i++) {
        msg = msg + (char) payload[i];
    }
    
    // Take action based on message
    if (String(topic).indexOf(commandTopic) != -1) {
        if (msg.indexOf("OPEN") != -1) {
            open_garage();
        } else if (msg.indexOf("CLOSE") != -1) {
            close_garage();
        } else if (msg.indexOf("STOP") != -1) {
            // switch the open/close value, useful since there's no state management
            isClosed = !isClosed;
            if (isClosed) {
                client.publish(stateTopic, "closed", true);
            } else {
                client.publish(stateTopic, "open", true);
            }
        }
    }
}

// Opens the garage door
int open_garage(String thisIsOnlyForParticle){
    /* Let Home Assistant control the state checking since it's sometimes mismatched
    if (!isClosed) {
        return 0;
    }
    */
    
    // Prevent 2 of the functions colliding with each other
    if (!poweringRelay) {
        poweringRelay = true;
        
        // Pull the relay pin and wait for 15s before releasing
        digitalWrite(relayPin, HIGH);
        timer = millis();
        openClose = 1;
        
        client.publish(stateTopic, "opening");
    }
    
    return 0;
}

// Closes the garage door
int close_garage(String thisIsOnlyForParticle){
    /* Let Home Assistant control the state checking since it's sometimes mismatched
    if (isClosed) {
        return 0;
    }
    */
    
    // Prevent 2 of the functions colliding with each other
    if (!poweringRelay) {
        poweringRelay = true;
        
        // Similar to holding the button for 2 seconds
        digitalWrite(relayPin, HIGH);
        delay(2000);
        digitalWrite(relayPin, LOW);
        
        // Wait half a second before powering the relay again since the garage door mechanism has a waiting period
        delay(500);
        
        // Hold the button for 15s to close the garage door
        digitalWrite(D0, HIGH);
        timer = millis();
        openClose = 2;
        
        client.publish(stateTopic, "closing", true);
    }
    
    return 0;
}

// Reset the device from the cloud. Useful when it completely stops responding but is connected to the internet
int reset_photon(String command) {
    resetFlag = true;
    return 0;
}

// Setup the Photon
void setup() 
{
    // Connect to the server
    client.connect("garage_door_opener_photon", mqttUser, mqttPassword);
    
    // Configure relay pins
    pinMode(relayPin, OUTPUT);
    pinMode(D3, OUTPUT);
    
    digitalWrite(relayPin, LOW);
    digitalWrite(D3, LOW);
    
    // Let H.A know about what is needed to communicate with the device
    client.publish(configTopic, configurationPayload, true);
    
    // Subscribe to the open_close topic to get notified when an open/close event is sent
    client.subscribe(commandTopic);
    
    // Reset heartbeat timer
    heartbeatTimer = millis();
    sensorUpdateTimer = millis();
    
    // Set the particle functions
    Particle.function("openGarageDoor", open_garage);
    Particle.function("closeGarageDoor", close_garage);
    Particle.function("resetMicrocontroller", reset_photon);
}


// Main loop
void loop()
{
    // Reset device if reset flag rasied
    if (resetFlag) {
        delay(1500); // Wait 1.5s to allow for the reset request to complete, not waiting causes the connection to timeout
        System.reset();
    }
    
    // Only try to send messages if we are connected
    if (client.isConnected())
    {
        // Send a heartbeat every 20s
        if (millis() - heartbeatTimer >= 20000) {
            client.publish(availabilityTopic, "online");
            
            heartbeatTimer = millis();
        }
        
        // Check if the timer for the relay power is done
        if (millis() - timer >= 16000 && poweringRelay) {
            // stop pulling the pin
            digitalWrite(relayPin, LOW);
            
            poweringRelay = false;
            
            // Change state from opening/closing to open/closed
            switch (openClose) {
                // Garage Door is open
                case 1: 
                    isClosed = false;
                    client.publish(stateTopic, "open", true);
                    break;
                
                // Garage Door is closed
                case 2:
                    isClosed = true;
                    client.publish(stateTopic, "closed", true);
                    break;
            }
            
            // Reset open/close variable
            openClose = 0;
        }
        
        // CALL THIS at the end of your loop
        client.loop();
    } else {
        Particle.publish("mqttconnection/isconnected", "false");
        client.connect("garage_door_opener_photon", mqttUser, mqttPassword);
        delay(3000);
        if (!client.isConnected()) reset_photon(""); // Auto reset if it doesn't want to connect
    }
}
