#include "skillops/common/config.h"

#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace skillops::common {

namespace {

std::string Trim(const std::string& value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::size_t CountIndent(const std::string& value) {
    std::size_t indent = 0;
    while (indent < value.size() && value[indent] == ' ') {
        ++indent;
    }
    return indent;
}

bool EndsWith(const std::string& value, const std::string& suffix) {
    return value.size() >= suffix.size() && value.substr(value.size() - suffix.size()) == suffix;
}

std::pair<std::string, std::string> ParseKeyValue(const std::string& line) {
    const auto colon = line.find(':');
    if (colon == std::string::npos) {
        return {"", ""};
    }
    return {Trim(line.substr(0, colon)), Trim(line.substr(colon + 1))};
}

std::uint16_t ParsePort(const std::string& value) {
    if (value.empty()) {
        return 0;
    }
    return static_cast<std::uint16_t>(std::stoi(value));
}

}  // namespace

ServiceConfig LoadServiceConfig(const std::string& path, ServiceConfig defaults) {
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("failed to open config file: " + path);
    }

    auto config = std::move(defaults);
    std::string section;
    std::string upstream;

    std::string raw_line;
    while (std::getline(input, raw_line)) {
        const auto comment = raw_line.find('#');
        const auto line_without_comment = comment == std::string::npos ? raw_line : raw_line.substr(0, comment);
        const auto line = Trim(line_without_comment);
        if (line.empty()) {
            continue;
        }

        const auto indent = CountIndent(line_without_comment);
        if (indent == 0 && EndsWith(line, ":")) {
            section = line.substr(0, line.size() - 1);
            upstream.clear();
            continue;
        }

        if (section == "service" && indent == 2) {
            const auto [key, value] = ParseKeyValue(line);
            if (key == "name") {
                config.name = value;
            } else if (key == "host") {
                config.host = value;
            } else if (key == "port") {
                config.port = ParsePort(value);
            }
            continue;
        }

        if (section == "upstreams" && indent == 2 && EndsWith(line, ":")) {
            upstream = line.substr(0, line.size() - 1);
            config.upstreams.try_emplace(upstream);
            continue;
        }

        if (section == "database" && indent == 2) {
            if (!config.database.has_value()) {
                config.database = DatabaseConfig{};
            }
            const auto [key, value] = ParseKeyValue(line);
            if (key == "enabled") {
                config.database->enabled = (value == "true" || value == "1" || value == "yes");
            } else if (key == "host") {
                config.database->host = value;
            } else if (key == "port") {
                config.database->port = ParsePort(value);
            } else if (key == "user") {
                config.database->user = value;
            } else if (key == "password") {
                config.database->password = value;
            } else if (key == "schema") {
                config.database->schema = value;
            }
            continue;
        }

        if (section == "upstreams" && indent == 4 && !upstream.empty()) {
            const auto [key, value] = ParseKeyValue(line);
            if (key == "host") {
                config.upstreams[upstream].host = value;
            } else if (key == "port") {
                config.upstreams[upstream].port = ParsePort(value);
            }
        }
    }

    return config;
}

}  // namespace skillops::common
