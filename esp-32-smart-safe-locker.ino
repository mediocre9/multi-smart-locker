#if __cplusplus < 201703L
#error This project requires a compiler of c++ 17 or above.
#endif

#include <Arduino.h>
#include <LittleFS.h>

#include "includes/Config.hpp"
#include "includes/HttpServer.hpp"
#include "includes/NetworkOperationManager.hpp"
#include "includes/WebSocket.hpp"

#if EXPERIMENTAL_FEATURE
WebSocket webSocket("/locks");
#endif

NetworkOperationManager network;

inline void initializeGPIOS();
void setupServers();

void setup() {
    Serial.begin(Device::BAUD_RATE);
    LOGLN(Device::FIRMWARE_VERSION);
    initializeGPIOS();

    if (!LittleFS.begin()) {
        LOGLN("LittleFS Failed!");
        LOGLN("Restarting . . . ");
        return;
    }

    setupServers();
}

void loop() {
#if EXPERIMENTAL_FEATURE
    if (!network.isWiFiConnected()) {
        LOGLN("Device has no WiFi connection!");
        return;
    }

    if (!NetworkOperationManager::hasFoundInternet()) {
        LOGLN("Device has no internet connection!")
        return;
    }

    if (!HttpServer::firebase.isAuthenticated()) {
        LOGLN("Firebase auth failed!");
        return;
    }

    LOGLN("Websocket support is running in experimental mode . . .");
    webSocket.getWebSocketInstance().cleanupClients();
#endif
}

inline void initializeGPIOS() {
    for (std::uint16_t pinNo : GPIOS) {
        pinMode(pinNo, OUTPUT);
        digitalWrite(pinNo, HIGH);
    }
}

void setupServers() {
    network.setupNetworks();
    HttpServer::start();
    HttpServer::setupRoutes();

    if (!network.isWiFiConnected() && !NetworkOperationManager::hasFoundInternet()) {
        return;
    }

    HttpServer::firebase.authenticate();
    if (!HttpServer::firebase.isAuthenticated()) {
        return;
    }

#if EXPERIMENTAL_FEATURE
    webSocket.init();
    HttpServer::server.addHandler(&(webSocket.getWebSocketInstance()));
#endif
    HttpServer::firebase.listenForAuthorizationStatus();
}
