#pragma once

#include <atomic>
#include <cstdint>
#include <string>

namespace skillops::gateway {

class SimpleHttpServer {
public:
    SimpleHttpServer(std::string host, std::uint16_t port);
    ~SimpleHttpServer();

    SimpleHttpServer(const SimpleHttpServer&) = delete;
    SimpleHttpServer& operator=(const SimpleHttpServer&) = delete;

    int Run();
    void Stop();

private:
    void HandleClient(int client_fd);
    std::string BuildResponse(const std::string& method, const std::string& path) const;
    static std::string JsonEnvelope(const std::string& code, const std::string& message, const std::string& data);

    std::string host_;
    std::uint16_t port_;
    std::atomic<bool> running_{false};
    int listen_fd_{-1};
};

}  // namespace skillops::gateway
