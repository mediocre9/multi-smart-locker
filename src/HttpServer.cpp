#define __ENABLE_DEVELOPMENT_MODE__ false

#include <LittleFS.h>
#include <UUID.h>
#include <VoyagerOTA.hpp>
#include <cstdint>
#include <functional>

#include "../includes/AsyncCookieAuthMiddleware.hpp"
#include "../includes/Config.hpp"
#include "../includes/CustomAsyncRateLimitMiddleware.hpp"
#include "../includes/FileSystem.hpp"
#include "../includes/FirebaseOperations.hpp"
#include "../includes/FirmwareUpdateHTMLPage.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/NetworkOperationManager.hpp"
#include "../includes/RTOSTasks.hpp"
#include "../includes/TaskCorePinController.hpp"
#include "../includes/WebSocket.hpp"

UserToGPIO HttpServer::users;
CfgFileSystem HttpServer::configFile;
AsyncWebServer HttpServer::server(80);
FirebaseOperations HttpServer::firebase;
TaskCorePinController taskHandler;

CookieIdGenerator cookie;
static NetworkOperationManager network;
static AsyncCookieAuthMiddleware authMiddleware(cookie);
static CustomAsyncRateLimitMiddleware rateLimitMiddleware;

inline void closeLockers() {
    for (std::uint16_t pinNo : GPIOS) {
        digitalWrite(pinNo, HIGH);
    }
}

void HttpServer::start() {
    users = configFile.read(CfgFilePath::USERS);
    server.begin();
    taskHandler.pinTaskToCPUCore(RTOSTask::firebaseListenerTask);
}

void HttpServer::setupRoutes() {
#if PROD_MODE
    rateLimitMiddleware.setMaxRequests(3);
#else
    rateLimitMiddleware.setMaxRequests(5);
#endif

    rateLimitMiddleware.setWindowSize(10);

    using Uri = const char*;
    using Method = WebRequestMethod;
    using RequestCallback = std::function<void(AsyncWebServerRequest*)>;
    using RouteDefinition = std::tuple<Uri, Method, RequestCallback, AsyncMiddleware*>;
    const std::uint16_t TOTAL_ROUTES = 16;

    const std::array<RouteDefinition, TOTAL_ROUTES> routes = {
        std::make_tuple("/", HTTP_GET, loginHandler_GET, &rateLimitMiddleware),
        std::make_tuple("/login", HTTP_POST, loginHandler_POST, &rateLimitMiddleware),
        std::make_tuple("/users", HTTP_GET, usersHandler_GET, &authMiddleware),
        std::make_tuple("/users", HTTP_POST, usersHandler_POST, &authMiddleware),
        std::make_tuple("/device-wifi", HTTP_GET, deviceWifiHandler_GET, &authMiddleware),
        std::make_tuple("/device/update/download", HTTP_GET, otaUpdateDownloadHandler_GET, &rateLimitMiddleware),
        std::make_tuple("/device/update", HTTP_GET, otaUpdateCheckHandler_GET, &authMiddleware),
        std::make_tuple("/device-wifi", HTTP_POST, deviceWifiHandler_POST, &authMiddleware),
        std::make_tuple("/home-wifi", HTTP_GET, homeWifiHandler_GET, &authMiddleware),
        std::make_tuple("/home-wifi", HTTP_POST, homeWifiHandler_POST, &authMiddleware),
        std::make_tuple("/device-password", HTTP_GET, changePasswordHandler_GET, &authMiddleware),
        std::make_tuple("/device-password", HTTP_POST, changePasswordHandler_POST, &authMiddleware),
        std::make_tuple("/device/reboot", HTTP_GET, rebootDeviceHandler_GET, &authMiddleware),
        std::make_tuple("/api/locks/unlock", HTTP_GET, unlockHandler_GET, &rateLimitMiddleware),
        std::make_tuple("/api/health", HTTP_GET, healthEndpointHandler_GET, &rateLimitMiddleware),
        std::make_tuple("/api/device/reboot", HTTP_GET, rebootEndpointHandler_GET, &rateLimitMiddleware)};

    for (const auto& [uri, method, handler, middleware] : routes) {
        server.on(uri, method, handler).addMiddleware(const_cast<AsyncMiddleware*>(middleware));
    }
    // protected endpoint for only client side applications . . .
    server.on("/api/locks/unlock", HTTP_POST, [](AsyncWebServerRequest* request) -> void {}, nullptr, lockController_POST);

    server.onNotFound(notFoundHandler_GET);
}

void HttpServer::otaUpdateCheckHandler_GET(AsyncWebServerRequest* request) {
    Voyager::OTA<> ota(Device::FIRMWARE_VERSION);
    ota.setBaseURL(AuthKeys::VOYAGER_BASE_URL);
    ota.setCredentials(AuthKeys::VOYAGER_PROJECT_ID, AuthKeys::VOYAGER_PROJECT_API_KEY);

    if (!NetworkOperationManager::hasFoundInternet()) {
        HTML_NO_INTERNET_UPDATE_PAGE.replace("[[currentVersion]]", String(Device::FIRMWARE_VERSION));
        request->send(StatusCode::SERVICE_UNREACHABLE, ContentType::HTML, HTML_NO_INTERNET_UPDATE_PAGE);
        return;
    }

    std::optional<Voyager::VoyagerReleaseModel> release = ota.fetchLatestRelease();

    if (release.has_value() && ota.isNewVersion(release->version)) {
        String updateHTMLPage = HTML_FIRMWARE_NEW_UPDATE_PAGE;
        updateHTMLPage.replace("[[newVersion]]", release->version);
        updateHTMLPage.replace("[[currentVersion]]", ota.getCurrentVersion());
        updateHTMLPage.replace("[[changeLog]]", release->changeLog);
        updateHTMLPage.replace("[[releasedDate]]", release->releasedDate);
        updateHTMLPage.replace("[[downloadFileSize]]", release->prettySize);
        request->send(StatusCode::OK_CODE, ContentType::HTML, updateHTMLPage);
    } else if (release.has_value() && ota.isUpToDate(release->version)) {
        HTML_FIRMWARE_NO_UPDATE_PAGE.replace("[[currentVersion]]", String(Device::FIRMWARE_VERSION));
        request->send(StatusCode::NOT_FOUND, ContentType::HTML, HTML_FIRMWARE_NO_UPDATE_PAGE);
    } else {
        request->send(StatusCode::SERVICE_UNREACHABLE, ContentType::HTML, "Service not reachable to Voyager OTA Service.");
    }
}

void HttpServer::otaUpdateDownloadHandler_GET(AsyncWebServerRequest* request) {
    request->send(StatusCode::OK_CODE, ContentType::HTML, "<h1> Starting Update . . .  </h1>");
    request->onDisconnect([]() -> void {
        TaskCorePinController controller;
        controller.pinTaskToCPUCore(RTOSTask::otaUpdateHandlerTask);
    });
}

void HttpServer::loginHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::INDEX);
}

void HttpServer::usersHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::USERS);
}

void HttpServer::deviceWifiHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::DEVICE_WIFI);
}

void HttpServer::homeWifiHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::HOME_WIFI);
}

void HttpServer::changePasswordHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::CHANGE_PASSWORD);
}

void HttpServer::notFoundHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::NOT_FOUND);
}

void HttpServer::rebootDeviceHandler_GET(AsyncWebServerRequest* request) {
    servePage(request, RouteFilePath::RESTART);
    request->onDisconnect([]() -> void {
        WiFi.disconnect(true);
        ESP.restart();
    });
}

void HttpServer::servePage(AsyncWebServerRequest* request, const String& filename) {
    if (!LittleFS.exists(filename)) {
        LOGLN("Failed to open " + filename);
        request->send(StatusCode::NOT_FOUND, ContentType::PLAIN, ResponseMessage::NOT_FOUND);
        return;
    }

    request->send(LittleFS, filename, ContentType::HTML);
}

void HttpServer::loginHandler_POST(AsyncWebServerRequest* request) {
    String usernameArgument = request->arg("username");
    String passwordArgument = request->arg("password");

    usernameArgument.trim();
    passwordArgument.trim();

    String currentPassword = ESP_ADMIN_WEB_AUTH_PWD;
    CfgFormat loginPassword = configFile.read(CfgFilePath::LOGIN);

    CfgFormat::iterator password = loginPassword.find("password");
    if (password != loginPassword.end()) {
        auto& [_, customPassword] = *password;
        customPassword.trim();
        currentPassword = customPassword.isEmpty() ? currentPassword : customPassword;
    }

    if ((usernameArgument == ESP_ADMIN_WEB_AUTH_USERNAME) && (currentPassword == passwordArgument)) {
        AsyncWebServerResponse* response = request->beginResponse(StatusCode::FOUND, ContentType::PLAIN, ResponseMessage::EMPTY_BODY);
        cookie.generate();
        response->addHeader("Location", "/home-wifi");
        response->addHeader("Cache-Control", "no-cache");
        response->addHeader("Set-Cookie", "COOKIE_ID=" + cookie.getId() + "; Path=/; HttpOnly; Max-Age=600");  // 600 secs = 10mins
        request->send(response);
        return;
    }

    LOGLN("Login failed!");
    request->redirect("/");
}

void HttpServer::usersHandler_POST(AsyncWebServerRequest* request) {
    LOGLN("Users Configuration!");

    CfgFormat users;
    const std::uint16_t totalArgs = request->args();
    for (std::uint16_t i = 0; i < totalArgs; i++) {
        // building email_1, email_2 etc named arguments
        // because these are defined in html forms....
        String email = request->arg("email_" + String(i + 1));
        email.trim();

        if (!email.isEmpty()) {
            LOGLN("Saved email: " + email);
            users.emplace(email, String(GPIOS.at(i)));
        }
    }

    bool saved = configFile.write(CfgFilePath::USERS, users);
    if (!saved) {
        LOGLN("Failed to save users!");
        servePage(request, RouteFilePath::USERS);
        return;
    }

    LOGLN("Users have been saved!");
    request->redirect("/device/reboot");
}

void HttpServer::deviceWifiHandler_POST(AsyncWebServerRequest* request) {
    String deviceSSID = request->arg("device_ssid");
    String devicePassword = request->arg("device_pwd");

    deviceSSID.trim();
    devicePassword.trim();

    CfgFormat deviceNetworkConfig;
    deviceNetworkConfig.emplace("device_ssid", deviceSSID);
    deviceNetworkConfig.emplace("device_pwd", devicePassword);

    bool saved = configFile.write(CfgFilePath::DEVICE, deviceNetworkConfig);

    if (!saved) {
        LOGLN("Failed to save device wifi configuration");
        request->send(StatusCode::INTERNAL_SERVER_ERROR, ContentType::PLAIN, "Failed to save device wifi configuration");
        return;
    }

    LOGLN("Device Wifi configuration saved successfully");
    request->redirect("/device/reboot");
}

void HttpServer::homeWifiHandler_POST(AsyncWebServerRequest* request) {
    String homeSSID = request->arg("home_ssid");
    String homePassword = request->arg("home_pwd");

    homeSSID.trim();
    homePassword.trim();

    CfgFormat homeNetworkConfig;
    homeNetworkConfig.emplace("home_ssid", homeSSID);
    homeNetworkConfig.emplace("home_pwd", homePassword);

    bool saved = configFile.write(CfgFilePath::HOME, homeNetworkConfig);

    if (!saved) {
        LOGLN("Failed to save home wifi configuration");
        request->send(StatusCode::INTERNAL_SERVER_ERROR, ContentType::PLAIN, "Failed to save home wifi configuration");
        return;
    }

    LOGLN("Home Wifi configuration saved successfully");
    request->redirect("/device/reboot");
}

void HttpServer::changePasswordHandler_POST(AsyncWebServerRequest* request) {
    String password = request->arg("password");
    password.trim();

    CfgFormat loginCfg;
    loginCfg.emplace("password", password);

    bool saved = configFile.write(CfgFilePath::LOGIN, loginCfg);

    if (!saved) {
        LOGLN("Failed to save login password!");
        request->send(StatusCode::INTERNAL_SERVER_ERROR, ContentType::PLAIN, "Failed to change password!");
        return;
    }

    LOGLN("Login password changed saved successfully");
    request->redirect("/device/reboot");
}

void HttpServer::lockController_POST(AsyncWebServerRequest* request, uint8_t* data, size_t length, size_t index, size_t total) {
    String contentType = request->header("Content-Type");

    if (!contentType.equals(ContentType::PLAIN)) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::ALLOWED_CONTENT_TYPE);
        return;
    }

    if (!network.isWiFiConnected() && !NetworkOperationManager::hasFoundInternet()) {
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::NETWORK_CONFIGURTION_REQUIRED);
        return;
    }

    if (!firebase.isOrganizationAuthorized()) {
        closeLockers();
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::LOCKER_ACCESS_RESTRICTED);
        return;
    }

    const String& apiKey = request->header("X-Authorization");
    if (apiKey.isEmpty()) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::API_KEY_NOT_FOUND);
        return;
    }

    if (!apiKey.equals(AuthKeys::CLIENT)) {
        request->send(StatusCode::UNAUTHORIZED, ContentType::PLAIN, ResponseMessage::INVALID_API_KEY);
        return;
    }

    String email;
    for (size_t i = 0; i < length; i++) {
        email += static_cast<char>(data[i]);
    }

    CfgFormat::iterator it = users.find(email);

    if (it == users.end()) {
        email.clear();
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::UNAUTHORIZED_LOCKER_ACCESS);
        return;
    }

    const auto& [_, gpio] = *it;
    std::uint16_t pinNo = static_cast<std::uint16_t>(gpio.toInt());

#if EXPERIMENTAL_FEATURE
    bool isNew = webSocket.addClient(pinNo, email);
    email.clear();
    if (isNew) {
        request->send(StatusCode::OK_CODE, ContentType::PLAIN, "Locker " + gpio + " has been unlocked!");
        return;
    }
    request->send(StatusCode::CONFLICT, ContentType::PLAIN, ResponseMessage::WEBSOCKET_CONNECTION_EXISTS);
#else
    email.clear();
    request->send(StatusCode::OK_CODE, ContentType::PLAIN, "Locker " + gpio + " has been unlocked!");
#endif
}

void HttpServer::unlockHandler_GET(AsyncWebServerRequest* request) {
    if (!request->hasParam("user")) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::EMAIL_QUERY_PARAM_REQUIRED);
        return;
    }

    if (!request->hasParam("token")) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::TOKEN_QUERY_PARAM_REQUIRED);
        return;
    }

    const String& email = request->getParam("user")->value();
    const String& token = request->getParam("token")->value();

    if (!token.equals(AuthKeys::ADMIN)) {
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::INVALID_API_KEY);
        return;
    }

    CfgFormat::iterator it = users.find(email);

    if (it == users.end()) {
        request->send(StatusCode::NOT_FOUND, ContentType::PLAIN, ResponseMessage::USER_NOT_FOUND);
        return;
    }

    const auto& [_, gpio] = *it;
    std::uint16_t pinNo = static_cast<std::uint16_t>(gpio.toInt());
    digitalWrite(pinNo, LOW);
    request->send(StatusCode::OK_CODE, ContentType::PLAIN, "Locker " + gpio + " has been unlocked!");
}

void HttpServer::rebootEndpointHandler_GET(AsyncWebServerRequest* request) {
    if (!request->hasParam("token")) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::TOKEN_QUERY_PARAM_REQUIRED);
        return;
    }

    const String& token = request->getParam("token")->value();
    if (!token.equals(AuthKeys::ADMIN)) {
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::INVALID_API_KEY);
        return;
    }

    request->send(StatusCode::OK_CODE, ContentType::PLAIN, ResponseMessage::REBOOT);
    request->onDisconnect([]() -> void {
        WiFi.disconnect(true);
        ESP.restart();
    });
}

void HttpServer::healthEndpointHandler_GET(AsyncWebServerRequest* request) {
    if (!request->hasParam("token")) {
        request->send(StatusCode::BAD_REQUEST, ContentType::PLAIN, ResponseMessage::TOKEN_QUERY_PARAM_REQUIRED);
        return;
    }

    const String& token = request->getParam("token")->value();
    if (!token.equals(AuthKeys::ADMIN)) {
        request->send(StatusCode::FORBIDDEN, ContentType::PLAIN, ResponseMessage::INVALID_API_KEY);
        return;
    }

    unsigned long uptimeSeconds = millis() / 1000;
    unsigned long uptimeMinutes = uptimeSeconds / 60;
    unsigned long uptimeHours = uptimeMinutes / 60;
    unsigned long uptimeDays = uptimeHours / 24;

    String log =
        "Build Version: " + String(Device::FIRMWARE_VERSION) + "\n" +
        "RSSI: " + String(WiFi.RSSI()) + "\n" +
        "Uptime: " + String(uptimeDays) + "d " + String(uptimeHours % 24) + "h " + String(uptimeMinutes % 60) + "m " + String(uptimeSeconds % 60) + "s\n" +
        "Wifi Status: " + String(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected") + "\n" +
        "Cloud Status: " + String(HttpServer::firebase.isAuthenticated() ? "Connected" : "Disconnected") + "\n" +
        "Free Heap: " + String(ESP.getFreeHeap() / 1024) + " Kb - " + String(ESP.getFreeHeap()) + " Bytes\n" +
        "Connected Clients: " + String(WiFi.AP.stationCount()) + "\n" +
        "Registered Local Users: " + String(users.size()) + "\n" +
        "ORG Authorization: " + String(HttpServer::firebase.isOrganizationAuthorized() ? "Granted " : "Revoked");

    LOGLN(log);
    request->send(StatusCode::OK_CODE, ContentType::PLAIN, log);
}
