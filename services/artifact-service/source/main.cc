#include "skillops/artifact/artifact_service.h"
#include "skillops/common/config.h"
#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/common/logging.h"

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

std::string ArtifactIdFromPath(const std::string& path) {
    constexpr const char* prefix = "/internal/v1/artifacts/";
    if (path.rfind(prefix, 0) != 0) {
        return "";
    }
    auto value = path.substr(std::string(prefix).size());
    constexpr const char* download_suffix = "/download";
    if (value.size() > std::string(download_suffix).size() &&
        value.substr(value.size() - std::string(download_suffix).size()) == download_suffix) {
        value = value.substr(0, value.size() - std::string(download_suffix).size());
    }
    return value;
}

std::uint64_t ParseSizeBytes(const std::string& value) {
    if (value.empty()) {
        return 0;
    }
    return static_cast<std::uint64_t>(std::stoull(value));
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18086;
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
        config.name = "artifact-service";
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
        skillops::artifact::ArtifactService service;
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"artifact-service\"}");
                }

                if (request.method == "POST" && request.path == "/internal/v1/artifacts") {
                    try {
                        const auto artifact = service.CreateArtifact({
                            skillops::common::ExtractJsonString(request.body, "project_id"),
                            skillops::common::ExtractJsonString(request.body, "owner_type"),
                            skillops::common::ExtractJsonString(request.body, "owner_id"),
                            skillops::common::ExtractJsonString(request.body, "file_name"),
                            skillops::common::ExtractJsonString(request.body, "content_type"),
                            skillops::common::ExtractJsonString(request.body, "sha256"),
                            ParseSizeBytes(skillops::common::ExtractJsonNumber(request.body, "size_bytes")),
                        });
                        return Envelope(200, "OK", "OK", "success", skillops::artifact::ArtifactToJson(artifact));
                    } catch (const std::exception&) {
                        return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid artifact payload", "{}");
                    }
                }

                if (request.method == "GET" && request.path == "/internal/v1/artifacts") {
                    return Envelope(
                        200,
                        "OK",
                        "OK",
                        "success",
                        skillops::artifact::ArtifactListToJson(service.ListArtifacts(
                            QueryValue(request.query, "project_id"),
                            QueryValue(request.query, "owner_type"),
                            QueryValue(request.query, "owner_id"))));
                }

                if (request.method == "GET") {
                    const auto artifact_id = ArtifactIdFromPath(request.path);
                    if (!artifact_id.empty()) {
                        const auto artifact = service.GetArtifact(artifact_id);
                        if (!artifact.has_value()) {
                            return Envelope(404, "Not Found", "ARTIFACT_NOT_FOUND", "artifact not found", "{}");
                        }
                        if (request.path.size() > std::string("/download").size() &&
                            request.path.substr(request.path.size() - std::string("/download").size()) == "/download") {
                            return Envelope(200, "OK", "OK", "success", skillops::artifact::ArtifactDownloadToJson(*artifact));
                        }
                        return Envelope(200, "OK", "OK", "success", skillops::artifact::ArtifactToJson(*artifact));
                    }
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("artifact-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "artifact-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
