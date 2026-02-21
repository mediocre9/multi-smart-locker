#ifndef NETWORK_MANAGER_H
#define NETWORK_MANAGER_H

#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <map>
#include <optional>

#include "Config.hpp"
#include "FileSystem.hpp"

using NetworkConfig = std::pair<String, String>;

class NetworkOperationManager final {
public:
    NetworkOperationManager() = default;

    void setupNetworks();

    [[nodiscard]] bool isWiFiConnected();

    [[nodiscard]] static bool hasFoundInternet() {
        WiFiClient client;
        return client.connect("1.1.1.1", 80);
    }

private:
    [[nodiscard]] std::optional<NetworkConfig> _getNetworkConfigs(const char* first, const char* second, const char* filename);

private:
    CfgFileSystem _config;
};

#endif