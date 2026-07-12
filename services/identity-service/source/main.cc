#include "skillops/common/config.h"
#include "skillops/common/logging.h"
#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/identity/identity_service.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string ReadArgValue(const std::string& arg, const std::string& prefix) {
    return arg.rfind(prefix, 0) == 0 ? arg.substr(prefix.size()) : "";
}

skillops::common::HttpResponse Envelope(int status, const std::string& reason, const std::string& code, const std::string& message, const std::string& data) {
    return skillops::common::HttpResponse::Json(status, reason, skillops::common::JsonEnvelope(code, message, "req_1", data));
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18081;
    std::string config_path;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto config_value = ReadArgValue(arg, "--config=");
        if (!config_value.empty()) {
            config_path = config_value;
        }
    }

    if (!config_path.empty()) {
        skillops::common::ServiceConfig config;
        config.name = "identity-service";
        config.host = host;
        config.port = port;
        config = skillops::common::LoadServiceConfig(config_path, config);
        host = config.host;
        port = config.port;
    }

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto config_value = ReadArgValue(arg, "--config=");
        const auto host_value = ReadArgValue(arg, "--host=");
        const auto port_value = ReadArgValue(arg, "--port=");
        if (!config_value.empty()) {
            continue;
        } else if (!host_value.empty()) {
            host = host_value;
        } else if (!port_value.empty()) {
            port = static_cast<std::uint16_t>(std::stoi(port_value));
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 2;
        }
    }

    try {
        skillops::common::InitLogging();
        skillops::common::ServiceConfig config;
        if (!config_path.empty()) {
            config.name = "identity-service";
            config = skillops::common::LoadServiceConfig(config_path, config);
        }
        skillops::identity::IdentityService service(config.database);
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"identity-service\"}");
                }

                if (request.method == "POST" && request.path == "/internal/v1/users") {
                    try {
                        const auto user = service.CreateUser({
                            skillops::common::ExtractJsonString(request.body, "team_id"),
                            skillops::common::ExtractJsonString(request.body, "name"),
                            skillops::common::ExtractJsonString(request.body, "email"),
                            skillops::common::ExtractJsonString(request.body, "role"),
                        });
                        return Envelope(200, "OK", "OK", "success", skillops::identity::UserToJson(user));
                    } catch (const std::invalid_argument&) {
                        return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid user payload", "{}");
                    }
                }

                if (request.method == "GET" && request.path == "/internal/v1/users/me") {
                    const auto user = service.CurrentUser();
                    if (!user.has_value()) {
                        return Envelope(404, "Not Found", "AUTH_USER_NOT_FOUND", "user not found", "{}");
                    }
                    return Envelope(200, "OK", "OK", "success", skillops::identity::UserToJson(*user));
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("identity-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "identity-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
