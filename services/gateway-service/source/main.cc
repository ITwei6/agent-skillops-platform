#include "skillops/common/config.h"
#include "skillops/common/logging.h"
#include "skillops/gateway/simple_http_server.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

std::string ReadArgValue(const std::string& arg, const std::string& prefix) {
    return arg.rfind(prefix, 0) == 0 ? arg.substr(prefix.size()) : "";
}

}  // namespace

int main(int argc, char** argv) {
    std::string host = "0.0.0.0";
    std::uint16_t port = 8080;
    std::string identity_host = "127.0.0.1";
    std::uint16_t identity_port = 18081;
    std::string project_host = "127.0.0.1";
    std::uint16_t project_port = 18082;
    std::string experience_host = "127.0.0.1";
    std::uint16_t experience_port = 18083;
    std::string skill_host = "127.0.0.1";
    std::uint16_t skill_port = 18084;
    std::string review_host = "127.0.0.1";
    std::uint16_t review_port = 18085;
    std::string artifact_host = "127.0.0.1";
    std::uint16_t artifact_port = 18086;
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
        config.name = "gateway-service";
        config.host = host;
        config.port = port;
        config.upstreams["identity-service"] = {identity_host, identity_port};
        config.upstreams["project-service"] = {project_host, project_port};
        config.upstreams["experience-service"] = {experience_host, experience_port};
        config.upstreams["skill-service"] = {skill_host, skill_port};
        config.upstreams["review-service"] = {review_host, review_port};
        config.upstreams["artifact-service"] = {artifact_host, artifact_port};
        config = skillops::common::LoadServiceConfig(config_path, config);

        host = config.host;
        port = config.port;
        identity_host = config.upstreams["identity-service"].host;
        identity_port = config.upstreams["identity-service"].port;
        project_host = config.upstreams["project-service"].host;
        project_port = config.upstreams["project-service"].port;
        experience_host = config.upstreams["experience-service"].host;
        experience_port = config.upstreams["experience-service"].port;
        skill_host = config.upstreams["skill-service"].host;
        skill_port = config.upstreams["skill-service"].port;
        review_host = config.upstreams["review-service"].host;
        review_port = config.upstreams["review-service"].port;
        artifact_host = config.upstreams["artifact-service"].host;
        artifact_port = config.upstreams["artifact-service"].port;
    }

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto config_value = ReadArgValue(arg, "--config=");
        const auto host_value = ReadArgValue(arg, "--host=");
        const auto port_value = ReadArgValue(arg, "--port=");
        const auto identity_host_value = ReadArgValue(arg, "--identity-host=");
        const auto identity_port_value = ReadArgValue(arg, "--identity-port=");
        const auto project_host_value = ReadArgValue(arg, "--project-host=");
        const auto project_port_value = ReadArgValue(arg, "--project-port=");
        const auto experience_host_value = ReadArgValue(arg, "--experience-host=");
        const auto experience_port_value = ReadArgValue(arg, "--experience-port=");
        const auto skill_host_value = ReadArgValue(arg, "--skill-host=");
        const auto skill_port_value = ReadArgValue(arg, "--skill-port=");
        const auto review_host_value = ReadArgValue(arg, "--review-host=");
        const auto review_port_value = ReadArgValue(arg, "--review-port=");
        const auto artifact_host_value = ReadArgValue(arg, "--artifact-host=");
        const auto artifact_port_value = ReadArgValue(arg, "--artifact-port=");
        if (!config_value.empty()) {
            continue;
        } else if (!host_value.empty()) {
            host = host_value;
        } else if (!port_value.empty()) {
            port = static_cast<std::uint16_t>(std::stoi(port_value));
        } else if (!identity_host_value.empty()) {
            identity_host = identity_host_value;
        } else if (!identity_port_value.empty()) {
            identity_port = static_cast<std::uint16_t>(std::stoi(identity_port_value));
        } else if (!project_host_value.empty()) {
            project_host = project_host_value;
        } else if (!project_port_value.empty()) {
            project_port = static_cast<std::uint16_t>(std::stoi(project_port_value));
        } else if (!experience_host_value.empty()) {
            experience_host = experience_host_value;
        } else if (!experience_port_value.empty()) {
            experience_port = static_cast<std::uint16_t>(std::stoi(experience_port_value));
        } else if (!skill_host_value.empty()) {
            skill_host = skill_host_value;
        } else if (!skill_port_value.empty()) {
            skill_port = static_cast<std::uint16_t>(std::stoi(skill_port_value));
        } else if (!review_host_value.empty()) {
            review_host = review_host_value;
        } else if (!review_port_value.empty()) {
            review_port = static_cast<std::uint16_t>(std::stoi(review_port_value));
        } else if (!artifact_host_value.empty()) {
            artifact_host = artifact_host_value;
        } else if (!artifact_port_value.empty()) {
            artifact_port = static_cast<std::uint16_t>(std::stoi(artifact_port_value));
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 2;
        }
    }

    try {
        skillops::common::InitLogging();
        skillops::gateway::SimpleHttpServer server(
            host,
            port,
            identity_host,
            identity_port,
            project_host,
            project_port,
            experience_host,
            experience_port,
            skill_host,
            skill_port,
            review_host,
            review_port,
            artifact_host,
            artifact_port);
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "gateway-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
