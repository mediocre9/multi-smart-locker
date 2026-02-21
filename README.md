![Platform](https://img.shields.io/badge/Microcontroller-ESP32-blue)
![WebServer](https://img.shields.io/badge/WebServer-ESPAsyncWebServer-green)
![WebSocket](https://img.shields.io/badge/WebSocket-Integrated-orange)
![FreeRTOS](https://img.shields.io/badge/FreeRTOS-Integrated-blue)
![Firebase](https://img.shields.io/badge/Firebase-Integrated-yellow)
![OTA Platform](https://img.shields.io/badge/OTA-VoyagerOTA-purple)

# Multi Smart Locker Firmware

> A modular, self-contained ESP32 firmware for managing multiple lockers with local access control, firmware revocation checks, and remote OTA updates.

## Features

- [x] Controls up to 10 lockers per device
- [x] Local dashboard for configuration and user management
- [x] Per-locker auto-lock with WebSocket timeout updates
- [x] Firmware revocation checks via Firebase
- [x] Remote OTA updates via VoyagerOTA
- [x] Smart-Link mobile app integration

> [!WARNING]
> Real-time Websocket updates are currently behind an `EXPERIMENTAL_FEATURE` flag.

---

## System Architecture

<img src="docs/architecure.png" width="70%">

---

## System Responses

| Type                         | Code | Message                                                                                   |
| ---------------------------- | ---- | ----------------------------------------------------------------------------------------- |
| Locker Unlocked              | 200  | `Locker (GPIO number) has been unlocked.`                                                 |
| WebSocket Exists             | 409  | `WebSocket already established.`                                                          |
| Access Denied                | 403  | `Access denied.`                                                                          |
| Firmware Restricted          | 403  | `Locker access restricted.`                                                               |
| Network Error                | 403  | `Connection failed. Check network settings.`                                              |
| Internet Network Error       | 403  | `Unable to connect. Please contact the admin to configure the system's network settings.` |
| Api Key Allowed Content-Type | 401  | `Allowed Content-Type is text/plain`                                                      |
| Api Key Not Found            | 400  | `API-Key was not provided!`                                                               |
| Invalid API Key              | 401  | `Invalid API-Key.`                                                                        |

---

## Getting Started

### Firebase Setup

```cpp
#define FIREBASE_WEB_API_KEY "<your-api-key>"
#define FIREBASE_RTDB_REFERENCE_URL "<your-rtdb-url>"
```

### Initial Boot

1. Set `REGISTER_ESP_ON_FIREBASE` to `true`
2. Flash the firmware
3. Reset flag to `false` after first boot

### Web Interface Files

1. Install [arduino-littlefs-upload](https://github.com/earlephilhower/arduino-littlefs-upload)
2. Place web files in `/data`
3. Upload with LittleFS plugin

---

## Usage

### Admin Access

1. Connect to ESP32 Wi-Fi
2. Visit `http://192.168.4.1`
3. Configure SSID, password, and authorized users

### Unlock Flow

1. Install [Smart Link](https://github.com/mediocre9/smart-link)
2. Sign in with Google
3. Scan fingerprint and request unlock
4. Firmware validates API key and revocation status of firmware on Firebase

> [!IMPORTANT]
> The Smart Link app connects to ESP32 via AP mode, but the device must have STA internet for Revocation Status.

---

## Admin Panel Screenshots

<img src="previews/1.png" width="70%">
<img src="previews/2.png" width="70%">
<img src="previews/3.png" width="70%">
<img src="previews/4.png" width="70%">
<img src="previews/5.png" width="70%">
<img src="previews/6.png" width="70%">

## Libraries Used

- [voyager-ota-client](https://github.com/mediocre9/voyager-ota-client)
- [Firebase ESP Client](https://github.com/mobizt/Firebase-ESP-Client)
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [LittleFS](https://github.com/earlephilhower/arduino-esp8266littlefs-plugin)
- [UUID](https://github.com/RobTillaart/UUID)

## License

This project is licensed under the MIT License. See the [LICENSE](https://github.com/mediocre9/multi-smart-locker/blob/main/LICENSE) for details.
