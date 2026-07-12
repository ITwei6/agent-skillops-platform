#include "skillops/gateway/simple_http_server.h"

#include "skillops/common/http_client.h"
#include "skillops/common/json.h"
#include "skillops/common/logging.h"

#include <string>
#include <utility>

namespace skillops::gateway {

namespace {

std::string WithQuery(std::string path, const skillops::common::HttpRequest& request) {
    if (!request.query.empty()) {
        path += "?" + request.query;
    }
    return path;
}

}  // namespace

SimpleHttpServer::SimpleHttpServer(
    std::string host,
    std::uint16_t port,
    std::string identity_host,
    std::uint16_t identity_port,
    std::string project_host,
    std::uint16_t project_port,
    std::string experience_host,
    std::uint16_t experience_port,
    std::string skill_host,
    std::uint16_t skill_port,
    std::string review_host,
    std::uint16_t review_port)
    : host_(std::move(host)),
      port_(port),
      identity_host_(std::move(identity_host)),
      identity_port_(identity_port),
      project_host_(std::move(project_host)),
      project_port_(project_port),
      experience_host_(std::move(experience_host)),
      experience_port_(experience_port),
      skill_host_(std::move(skill_host)),
      skill_port_(skill_port),
      review_host_(std::move(review_host)),
      review_port_(review_port) {
    server_ = std::make_unique<skillops::common::HttpServer>(
        host_,
        port_,
        [this](const skillops::common::HttpRequest& request) {
            return HandleRequest(request);
        });
}

SimpleHttpServer::~SimpleHttpServer() {
    Stop();
}

int SimpleHttpServer::Run() {
    skillops::common::LogInfo("gateway-service listening on " + host_ + ":" + std::to_string(port_));
    return server_->Run();
}

void SimpleHttpServer::Stop() {
    if (server_) {
        server_->Stop();
    }
}

skillops::common::HttpResponse SimpleHttpServer::HandleRequest(const skillops::common::HttpRequest& request) const {
    if (request.method == "GET" && (request.path == "/healthz" || request.path == "/api/v1/ping")) {
        return skillops::common::HttpResponse::Json(
            200,
            "OK",
            skillops::common::JsonEnvelope("OK", "success", "req_1", "{}"));
    }

    if (request.path == "/api/v1/users" || request.path == "/api/v1/users/me") {
        const auto internal_path = request.path == "/api/v1/users"
                                       ? "/internal/v1/users"
                                       : "/internal/v1/users/me";
        skillops::common::HttpClient client(identity_host_, identity_port_);
        return client.Send(request.method, WithQuery(internal_path, request), request.body);
    }

    if (request.path == "/api/v1/projects" || request.path.rfind("/api/v1/projects/", 0) == 0) {
        const auto internal_path = "/internal/v1" + request.path.substr(std::string("/api/v1").size());
        skillops::common::HttpClient client(project_host_, project_port_);
        return client.Send(request.method, WithQuery(internal_path, request), request.body);
    }

    if (request.path == "/api/v1/experiences" || request.path.rfind("/api/v1/experiences/", 0) == 0) {
        const auto internal_path = "/internal/v1" + request.path.substr(std::string("/api/v1").size());
        skillops::common::HttpClient client(experience_host_, experience_port_);
        return client.Send(request.method, WithQuery(internal_path, request), request.body);
    }

    if (request.path == "/api/v1/skill-drafts" || request.path.rfind("/api/v1/skill-drafts/", 0) == 0 ||
        request.path == "/api/v1/skills") {
        if (request.path.rfind("/api/v1/skill-drafts/", 0) == 0 &&
            request.path.size() > std::string("/reviews").size() &&
            request.path.substr(request.path.size() - std::string("/reviews").size()) == "/reviews") {
            const auto internal_path = "/internal/v1" + request.path.substr(std::string("/api/v1").size());
            skillops::common::HttpClient client(review_host_, review_port_);
            return client.Send(request.method, WithQuery(internal_path, request), request.body);
        }
        const auto internal_path = "/internal/v1" + request.path.substr(std::string("/api/v1").size());
        skillops::common::HttpClient client(skill_host_, skill_port_);
        return client.Send(request.method, WithQuery(internal_path, request), request.body);
    }

    if (request.path == "/api/v1/reviews/queue") {
        const auto internal_path = "/internal/v1/reviews/queue";
        skillops::common::HttpClient client(review_host_, review_port_);
        return client.Send(request.method, WithQuery(internal_path, request), request.body);
    }

    return skillops::common::HttpResponse::Json(
        404,
        "Not Found",
        skillops::common::JsonEnvelope("SYSTEM_NOT_FOUND", "not found", "req_1", "{}"));
}

}  // namespace skillops::gateway
