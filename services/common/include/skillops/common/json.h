#pragma once

#include <cstddef>
#include <sstream>
#include <string>
#include <vector>

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

inline char JsonUnescapeChar(char ch) {
    switch (ch) {
    case 'n':
        return '\n';
    case 'r':
        return '\r';
    case 't':
        return '\t';
    case '"':
        return '"';
    case '\\':
        return '\\';
    default:
        return ch;
    }
}

inline std::string JsonStringArray(const std::vector<std::string>& values) {
    std::string result = "[";
    for (std::size_t i = 0; i < values.size(); ++i) {
        if (i > 0) {
            result += ",";
        }
        result += JsonString(values[i]);
    }
    result += "]";
    return result;
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
            value.push_back(JsonUnescapeChar(ch));
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

inline std::vector<std::string> ExtractJsonStringArray(const std::string& body, const std::string& key) {
    std::vector<std::string> values;
    const auto key_pos = body.find("\"" + key + "\"");
    if (key_pos == std::string::npos) {
        return values;
    }

    const auto colon_pos = body.find(':', key_pos);
    if (colon_pos == std::string::npos) {
        return values;
    }

    const auto array_start = body.find('[', colon_pos + 1);
    if (array_start == std::string::npos) {
        return values;
    }

    const auto array_end = body.find(']', array_start + 1);
    if (array_end == std::string::npos) {
        return values;
    }

    std::string value;
    bool in_string = false;
    bool escaped = false;
    for (auto i = array_start + 1; i < array_end; ++i) {
        const char ch = body[i];
        if (!in_string) {
            if (ch == '"') {
                in_string = true;
                value.clear();
            }
            continue;
        }

        if (escaped) {
            value.push_back(JsonUnescapeChar(ch));
            escaped = false;
            continue;
        }
        if (ch == '\\') {
            escaped = true;
            continue;
        }
        if (ch == '"') {
            values.push_back(value);
            in_string = false;
            continue;
        }
        value.push_back(ch);
    }

    return values;
}

}  // namespace skillops::common
