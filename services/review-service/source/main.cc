#include "skillops/common/config.h"
#include "skillops/common/http_server.h"
#include "skillops/common/json.h"
#include "skillops/common/logging.h"
#include "skillops/review/review_service.h"

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

std::string ReviewDraftIdFromPath(const std::string& path) {
    constexpr const char* prefix = "/internal/v1/skill-drafts/";
    constexpr const char* suffix = "/reviews";
    if (path.rfind(prefix, 0) != 0) {
        return "";
    }
    const auto value = path.substr(std::string(prefix).size());
    if (value.size() <= std::string(suffix).size()) {
        return "";
    }
    if (value.substr(value.size() - std::string(suffix).size()) != suffix) {
        return "";
    }
    return value.substr(0, value.size() - std::string(suffix).size());
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 18085;
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
        config.name = "review-service";
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
        skillops::review::ReviewService service;
        skillops::common::HttpServer server(
            host,
            port,
            [&service](const skillops::common::HttpRequest& request) {
                if (request.method == "GET" && request.path == "/healthz") {
                    return Envelope(200, "OK", "OK", "success", "{\"service\":\"review-service\"}");
                }

                if (request.method == "GET" && request.path == "/internal/v1/reviews/queue") {
                    return Envelope(
                        200,
                        "OK",
                        "OK",
                        "success",
                        skillops::review::ReviewQueueToJson(service.ListQueue(
                            QueryValue(request.query, "project_id"),
                            QueryValue(request.query, "status"))));
                }

                if (request.method == "POST") {
                    const auto draft_id = ReviewDraftIdFromPath(request.path);
                    if (!draft_id.empty()) {
                        try {
                            const auto review = service.CreateReview({
                                draft_id,
                                skillops::common::ExtractJsonString(request.body, "project_id"),
                                skillops::common::ExtractJsonString(request.body, "reviewer_id"),
                                skillops::common::ExtractJsonString(request.body, "decision"),
                                skillops::common::ExtractJsonString(request.body, "comments"),
                            });
                            return Envelope(200, "OK", "OK", "success", skillops::review::ReviewRecordToJson(review));
                        } catch (const std::invalid_argument&) {
                            return Envelope(400, "Bad Request", "VALIDATION_INVALID_ARGUMENT", "invalid review payload", "{}");
                        }
                    }
                }

                return Envelope(404, "Not Found", "SYSTEM_NOT_FOUND", "not found", "{}");
            });

        skillops::common::LogInfo("review-service listening on " + host + ":" + std::to_string(port));
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "review-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
