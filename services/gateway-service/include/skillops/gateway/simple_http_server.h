#pragma once

#include "skillops/common/http_server.h"

#include <cstdint>
#include <memory>
#include <string>

namespace skillops::gateway {

class SimpleHttpServer {
public:
    SimpleHttpServer(
        std::string host,
        std::uint16_t port,
        std::string identity_host,
        std::uint16_t identity_port,
        std::string project_host,
        std::uint16_t project_port);
    ~SimpleHttpServer();

    SimpleHttpServer(const SimpleHttpServer&) = delete;
    SimpleHttpServer& operator=(const SimpleHttpServer&) = delete;

    int Run();
    void Stop();

private:
    skillops::common::HttpResponse HandleRequest(const skillops::common::HttpRequest& request) const;

    std::string host_;
    std::uint16_t port_;
    std::string identity_host_;
    std::uint16_t identity_port_;
    std::string project_host_;
    std::uint16_t project_port_;
    std::unique_ptr<skillops::common::HttpServer> server_;
};

}  // namespace skillops::gateway
