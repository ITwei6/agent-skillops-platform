#include "skillops/common/logging.h"
#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/project/project_service.h"

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

std::string ProjectIdFromPath(const std::string& path) {
    constexpr const char* prefix = "/internal/v1/projects/";
    const std::string value(path);
    if (value.rfind(prefix, 0) != 0) {
        return "";
    }
    return value.substr(std::string(prefix).size());
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18082;

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
        skillops::project::ProjectService service;
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"project-service\"}");
                }

                if (request.method == "POST" && request.path == "/internal/v1/projects") {
                    try {
                        const auto project = service.CreateProject({
                            skillops::common::ExtractJsonString(request.body, "team_id"),
                            skillops::common::ExtractJsonString(request.body, "name"),
                            skillops::common::ExtractJsonString(request.body, "description"),
                            skillops::common::ExtractJsonString(request.body, "repo_url"),
                            skillops::common::ExtractJsonString(request.body, "created_by"),
                        });
                        return Envelope(200, "OK", "OK", "success", skillops::project::ProjectToJson(project));
                    } catch (const std::invalid_argument&) {
                        return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid project payload", "{}");
                    }
                }

                if (request.method == "GET" && request.path == "/internal/v1/projects") {
                    return Envelope(200, "OK", "OK", "success", skillops::project::ProjectListToJson(service.ListProjects()));
                }

                if (request.method == "GET") {
                    const auto project_id = ProjectIdFromPath(request.path);
                    if (!project_id.empty()) {
                        const auto project = service.GetProject(project_id);
                        if (!project.has_value()) {
                            return Envelope(404, "Not Found", "PROJECT_NOT_FOUND", "project not found", "{}");
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::project::ProjectToJson(*project));
                    }
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("project-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "project-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
