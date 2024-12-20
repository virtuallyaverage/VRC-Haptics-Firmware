#include "OSC/callbacks.hpp"
#include "OSC/_osc.h"


void printRawPacket() {
    printRaw();
}

/// @brief start mDNS and OSC
void WirelessStart() {
    LOG_SET_LEVEL(DebugLogLevel::LVL_TRACE);

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false); // thanks Daky

    // Start WiFi connection
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    LOG_INFO("WIFI: Connecting");
    
    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(1); // needed for esp8266
    }

    // Print the IP address
    selfIP = WiFi.localIP().toString();
    LOG_INFO("Wifi Connected:", selfIP);

    // Start listening for OSC server
    OscWiFi.subscribe(RECIEVE_PORT, PING_ADDRESS, &handlePing);
    LOG_INFO("WIFI: OSC: server started on port:", RECIEVE_PORT);

    StartMDNS();// NEEDS TO BE AFTER SUBSCRIBER SET UP
    
}

void StartMDNS() {
    //close any previous entries
    MDNS.end();
    delay(100);

    // Start mDNS service
    if (MDNS.begin(mdnsName)) {
        MDNS.addService(MDNS_SERVICE_NAME, MDNS_SERVICE_PROTOCOL, RECIEVE_PORT);  // Advertise the service
        MDNS.addServiceTxt(MDNS_SERVICE_NAME, MDNS_SERVICE_PROTOCOL, "MAC", WiFi.macAddress());
        LOG_INFO("WIFI: mDNS service started");

    } else {
        LOG_ERROR("WIFI: Error starting mDNS");
    }

}

bool WiFiConnected(){
    return WiFi.status() == WL_CONNECTED;
}

void StartHeartBeat() {
    if (hostIP.isEmpty() | !sendPort) {
        LOG_ERROR("HeartBeat set up before ip filled!");
    } else if (heartbeatPublisher){
        LOG_ERROR(F("Heartbeat already set up"));
    }

    // Publish heart beat on one second intervals
    OscWiFi.publish(hostIP, sendPort, HEARTBEAT_ADDRESS, &millis, selfIP, selfMac) 
        -> setFrameRate(1);
    heartbeatPublisher = OscWiFi.getPublishElementRef(hostIP, sendPort, HEARTBEAT_ADDRESS);

    if (!heartbeatPublisher) {
        LOG_ERROR(F("HeartBeat Wasn't Established"));
    }
}

void handlePing(const OscMessage& message){
    //if we recieve a ping and we were already setup, it is likely a server restart. 
    //In that case the port, ip, and other values should be reinited

    sendPort = message.arg<uint16_t>(0);  // Get the host's port from the message
    hostIP = message.remoteIP();  // Get the host's IP address

    //create our own recieving server
    OscWiFi.subscribe(RECIEVE_PORT, MOTOR_ADDRESS, &motorMessage_callback);

    LOG_DEBUG("Received ping from:", hostIP);

    //sending client
    oscClient = OscWiFi.getClient();

    // Respond to ping
    OscMessage pingResponse(PING_ADDRESS);
    pingResponse.pushInt32(RECIEVE_PORT);
    pingResponse.pushString(WiFi.macAddress());
    oscClient.send(hostIP, sendPort, pingResponse);

    if (!heartbeatPublisher){
        StartHeartBeat();
    }

}

/// @brief Push and pull OSC updates
void WirelessTick() {
    OscWiFi.update(); // should be called to subscribe + publish osc
    
}
