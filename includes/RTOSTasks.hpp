#ifndef RTOS_TASK_H
#define RTOS_TASK_H

#define __ENABLE_DEVELOPMENT_MODE__ false

#include <VoyagerOTA.hpp>
#include "../includes/Config.hpp"
#include "../includes/HttpServer.hpp"
#include "../includes/NetworkOperationManager.hpp"
#include "../includes/WebSocket.hpp"

namespace RTOSTask {
    void otaUpdateHandlerTask(void* _) {
        Voyager::OTA<> ota(Device::FIRMWARE_VERSION);
        ota.setBaseURL(AuthKeys::VOYAGER_BASE_URL);
        ota.setCredentials(AuthKeys::VOYAGER_PROJECT_ID, AuthKeys::VOYAGER_PROJECT_API_KEY);

        if (!NetworkOperationManager::hasFoundInternet()) {
            LOGLN(ResponseMessage::NO_INTERNET_CONNECTION);
            return;
        }

        std::optional<Voyager::VoyagerReleaseModel> release = ota.fetchLatestRelease();
        if (!release) {
            LOGLN("No updates available!");
            return;
        }

        if (release && ota.isNewVersion(release->version)) {
            ota.setDownloadURL(release->downloadURL);
            ota.performUpdate();
        } else if (release && ota.isUpToDate(release->version)) {
            LOGLN(release->message);
        } else {
            LOGLN(release->message);
        }
    }

    void firebaseListenerTask(void* _) {
        unsigned long previousMillisClient = 0;
        unsigned long previousMillisFirebase = 0;
        const unsigned long CLIENT_INTERVAL = 1000;  // 1 sec . . .

#if PROD_MODE
        const unsigned long FIREBASE_INTERVAL = 600 * 1000;  // 600 seconds (10 minutes) . . .
#else
        const unsigned long FIREBASE_INTERVAL = 60 * 1000;  // 60 seconds (1 minute) . . .
#endif

        for (;;) {
            unsigned long currentMillis = millis();
            if ((currentMillis - previousMillisFirebase) >= FIREBASE_INTERVAL) {
                previousMillisFirebase = currentMillis;

                if (WiFi.status() == WL_CONNECTED) {
                    if (HttpServer::firebase.isAuthenticated()) {
                        HttpServer::firebase.listenForAuthorizationStatus();
                    }
                }
            }

#if EXPERIMENTAL_FEATURE
            // WebSocket client operations....
            if ((currentMillis - previousMillisClient) >= CLIENT_INTERVAL) {
                previousMillisClient = currentMillis;

                ClientManager& manager = webSocket.getClients();
                for (auto& [clientId, clientInfo] : manager) {
                    if (clientInfo.timeout > 0) {
                        clientInfo.timeout -= CLIENT_INTERVAL;
                        webSocket.getWebSocketInstance().text(clientId, String(clientInfo.timeout));
                        LOGLN("Client ID: " + String(clientId) + " " + clientInfo.email);
                    }

                    // disconnect client and close locker...
                    if (clientInfo.timeout <= 0) {
                        digitalWrite(clientInfo.gpio, HIGH);
                        webSocket.getWebSocketInstance().close(clientId);
                    }
                }
            }
#endif

            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
}  // namespace RTOSTask

#endif