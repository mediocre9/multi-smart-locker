#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <WString.h>
#include <map>

#include "Config.hpp"

using CfgFormat = std::map<String, String>;

template <typename T>
class BaseFileSystem {
public:
    [[nodiscard]] virtual bool write(const String& filename, const T& data) = 0;
    [[nodiscard]] virtual T read(const String& filename) = 0;
    virtual ~BaseFileSystem() {}
};

class CfgFileSystem final : public BaseFileSystem<CfgFormat> {
public:
    [[nodiscard]] bool write(const String& filename, const CfgFormat& data) override final;
    [[nodiscard]] CfgFormat read(const String& filename) override final;
};

#endif