#include "../includes/FileSystem.hpp"

#include <LittleFS.h>
#include <optional>

#include "../includes/Config.hpp"

[[nodiscard]] bool CfgFileSystem::write(const String& filename, const CfgFormat& data) {
    File file = LittleFS.open(filename.c_str(), LittleFSFileMode::WRITE);

    if (!file) {
        LOGLN("Failed to open " + filename);
        file.close();
        return false;
    }

    for (const auto& [key, value] : data) {
        String record = key + "=" + value + "\n";
        file.print(record);
    }

    file.close();
    return true;
}

[[nodiscard]] CfgFormat CfgFileSystem::read(const String& filename) {
    CfgFormat data;
    File file = LittleFS.open(filename.c_str(), LittleFSFileMode::READ);

    if (!file) {
        LOGLN("Failed to open " + filename);
        file.close();
        return data;
    }

    while (file.available()) {
        String line = file.readStringUntil('\n');
        int index = line.indexOf('=');
        if (index != -1) {
            String key = line.substring(0, index);
            String value = line.substring(index + 1);
            data[key] = value;
        }
    }

    file.close();
    return data;
}