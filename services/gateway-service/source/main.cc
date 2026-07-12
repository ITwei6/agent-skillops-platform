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

    for (int i = 1; i < argc; ++i) {
        const std::string arg(argv[i]);
        const auto host_value = ReadArgValue(arg, "--host=");
        const auto port_value = ReadArgValue(arg, "--port=");
        const auto identity_host_value = ReadArgValue(arg, "--identity-host=");
        const auto identity_port_value = ReadArgValue(arg, "--identity-port=");
        const auto project_host_value = ReadArgValue(arg, "--project-host=");
        const auto project_port_value = ReadArgValue(arg, "--project-port=");
        if (!host_value.empty()) {
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
            project_port);
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "gateway-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
