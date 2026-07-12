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
        skillops::gateway::SimpleHttpServer server(host, port);
        return server.Run();
    } catch (const std::exception& ex) {
        std::cerr << "gateway-service failed: " << ex.what() << std::endl;
        return 1;
    }
}
