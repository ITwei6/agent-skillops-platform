#pragma once

#include <sstream>
#include <string>

namespace skillops::common {

inline std::string JsonEscape(const std::string& value) {
    std::ostringstream out;
    for (const char ch : value) {
        switch (ch) {
        case '"':
            out << "\\\"";
            break;
        case '\\':
            out << "\\\\";
            break;
        case '\n':
            out << "\\n";
            break;
        case '\r':
            out << "\\r";
            break;
        case '\t':
            out << "\\t";
            break;
        default:
            out << ch;
            break;
        }
    }
    return out.str();
}

inline std::string JsonString(const std::string& value) {
    return "\"" + JsonEscape(value) + "\"";
}

inline std::string JsonEnvelope(
    const std::string& code,
    const std::string& message,
    const std::string& request_id,
    const std::string& data) {
    return "{\"code\":" + JsonString(code) +
           ",\"message\":" + JsonString(message) +
           ",\"request_id\":" + JsonString(request_id) +
           ",\"data\":" + data + "}";
}

inline std::string ExtractJsonString(const std::string& body, const std::string& key) {
    const auto key_pos = body.find("\"" + key + "\"");
    if (key_pos == std::string::npos) {
        return "";
    }

    const auto colon_pos = body.find(':', key_pos);
    if (colon_pos == std::string::npos) {
        return "";
    }

    const auto quote_start = body.find('"', colon_pos + 1);
    if (quote_start == std::string::npos) {
        return "";
    }

    std::string value;
    bool escaped = false;
    for (auto i = quote_start + 1; i < body.size(); ++i) {
        const char ch = body[i];
        if (escaped) {
            value.push_back(ch);
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            return value;
        }
        value.push_back(ch);
    }

    return "";
}

}  // namespace skillops::common
