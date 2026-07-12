#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>

namespace skillops::common {

struct EndpointConfig {
    std::string host = "127.0.0.1";
    std::uint16_t port = 0;
};

struct DatabaseConfig {
    bool enabled = false;
    std::string host = "127.0.0.1";
    std::uint16_t port = 3306;
    std::string user;
    std::string password;
    std::string schema;
};

struct ServiceConfig {
    std::string name;
    std::string host = "0.0.0.0";
    std::uint16_t port = 0;
    std::unordered_map<std::string, EndpointConfig> upstreams;
    std::optional<DatabaseConfig> database;
};

ServiceConfig LoadServiceConfig(const std::string& path, ServiceConfig defaults);

}  // namespace skillops::common
