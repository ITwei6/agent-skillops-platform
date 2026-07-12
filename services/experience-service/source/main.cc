#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/common/logging.h"
#include "skillops/experience/experience_service.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string ReadArgValue(const std::string& arg, const std::string& prefix) {
    return arg.rfind(prefix, 0) == 0 ? arg.substr(prefix.size()) : "";
}

skillops::common::HttpResponse Envelope(
    int status,
    const std::string& reason,
    const std::string& code,
    const std::string& message,
    const std::string& data) {
    return skillops::common::HttpResponse::Json(status, reason, skillops::common::JsonEnvelope(code, message, "req_1", data));
}

std::string ExperienceIdFromPath(const std::string& path) {
    constexpr const char* prefix = "/internal/v1/experiences/";
    const std::string value(path);
    if (value.rfind(prefix, 0) != 0) {
        return "";
    }
    return value.substr(std::string(prefix).size());
}

std::string QueryValue(const std::string& query, const std::string& key) {
    const auto prefix = key + "=";
    std::size_t start = 0;
    while (start <= query.size()) {
        const auto end = query.find('&', start);
        const auto item = query.substr(start, end == std::string::npos ? std::string::npos : end - start);
        if (item.rfind(prefix, 0) == 0) {
            return item.substr(prefix.size());
        }
        if (end == std::string::npos) {
            break;
        }
        start = end + 1;
    }
    return "";
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18083;

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto host_value = ReadArgValue(arg, "--host=");
        const auto port_value = ReadArgValue(arg, "--port=");
        if (!host_value.empty()) {
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
        skillops::experience::ExperienceService service;
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"experience-service\"}");
                }

                if (request.method == "POST" && request.path == "/internal/v1/experiences") {
                    try {
                        const auto experience = service.CreateExperience({
                            skillops::common::ExtractJsonString(request.body, "project_id"),
                            skillops::common::ExtractJsonString(request.body, "title"),
                            skillops::common::ExtractJsonString(request.body, "source_type"),
                            skillops::common::ExtractJsonString(request.body, "summary"),
                            skillops::common::ExtractJsonStringArray(request.body, "artifact_ids"),
                        });
                        return Envelope(200, "OK", "OK", "success", skillops::experience::ExperienceToJson(experience));
                    } catch (const std::invalid_argument&) {
                        return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid experience payload", "{}");
                    }
                }

                if (request.method == "GET" && request.path == "/internal/v1/experiences") {
                    const auto project_id = QueryValue(request.query, "project_id");
                    return Envelope(
                        200,
                        "OK",
                        "OK",
                        "success",
                        skillops::experience::ExperienceListToJson(service.ListExperiences(project_id)));
                }

                if (request.method == "GET") {
                    const auto experience_id = ExperienceIdFromPath(request.path);
                    if (!experience_id.empty()) {
                        const auto experience = service.GetExperience(experience_id);
                        if (!experience.has_value()) {
                            return Envelope(404, "Not Found", "EXPERIENCE_NOT_FOUND", "experience not found", "{}");
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::experience::ExperienceToJson(*experience));
                    }
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("experience-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "experience-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
