/// private functions for the osc.cpp file

#include <ArduinoOSCWiFi.h>
#include <ESPmDNS.h>
#include <WiFi.h>

#include "config.hpp"
#include "globals.hpp"
#include "osc.h"

// OSC client to send messages back to the hosts
OscWiFiClient oscClient;

//publisher references
OscPublishElementRef heartbeatPublisher;

// we need to get host ip first
String selfIP = "";
String selfMac = WiFi.macAddress();
String hostIP = "";
uint32_t sendPort = 0;
uint32_t recvPort = RECIEVE_PORT;

#ifdef ROLE_VEST
const String mdnsName = "vest";
#endif
#ifdef ROLE_VEST_F
const String mdnsName = "vest_f";
#endif
#ifdef ROLE_VEST_B
const String mdnsName = "vest_b";
#endif
#ifdef ROLE_HEAD
const String mdnsName = "head";
#endif

void StartMDNS();
void StartHeartBeat();
void handlePing(const OscMessage& message);

