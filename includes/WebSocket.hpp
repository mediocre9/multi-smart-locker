#ifndef WEB_SOCKET_H
#define WEB_SOCKET_H

#include <ESPAsyncWebServer.h>
#include <WString.h>
#include <cstdint>
#include <map>

#include "Config.hpp"
#include "FileSystem.hpp"

using ClientID = std::uint32_t;

struct ClientInfo {
    std::uint16_t gpio;
    std::uint16_t timeout;
    String email;
};

using ClientManager = std::map<ClientID, ClientInfo>;

class WebSocket final {
public:
    WebSocket() = delete;

    explicit WebSocket(const String& channel);

    void init();

    [[nodiscard]] bool addClient(std::uint16_t gpio, const String& email);

    void onEvent(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, std::uint8_t* data, std::size_t length);

    [[nodiscard]] AsyncWebSocket& getWebSocketInstance() {
        return _webSocket;
    }

    [[nodiscard]] ClientManager& getClients() {
        return _clients;
    }

private:
    String _email;
    std::uint16_t _gpio;
    ClientManager _clients;
    AsyncWebSocket _webSocket;
};

extern WebSocket webSocket;

#endif