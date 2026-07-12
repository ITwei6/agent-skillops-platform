#include "skillops/gateway/simple_http_server.h"

#include "skillops/common/logging.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace skillops::gateway {

SimpleHttpServer::SimpleHttpServer(std::string host, std::uint16_t port)
    : host_(std::move(host)), port_(port) {}

SimpleHttpServer::~SimpleHttpServer() {
    Stop();
}

int SimpleHttpServer::Run() {
    listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd_ < 0) {
        throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));
    }

    int reuse = 1;
    if (::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        throw std::runtime_error(std::string("setsockopt failed: ") + std::strerror(errno));
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    if (::inet_pton(AF_INET, host_.c_str(), &address.sin_addr) != 1) {
        throw std::runtime_error("invalid IPv4 host: " + host_);
    }

    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        throw std::runtime_error(std::string("bind failed: ") + std::strerror(errno));
    }

    if (::listen(listen_fd_, SOMAXCONN) < 0) {
        throw std::runtime_error(std::string("listen failed: ") + std::strerror(errno));
    }

    running_ = true;
    skillops::common::LogInfo("gateway-service listening on " + host_ + ":" + std::to_string(port_));

    while (running_) {
        sockaddr_in client_address{};
        socklen_t client_length = sizeof(client_address);
        int client_fd = ::accept(listen_fd_, reinterpret_cast<sockaddr*>(&client_address), &client_length);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            throw std::runtime_error(std::string("accept failed: ") + std::strerror(errno));
        }

        std::thread(&SimpleHttpServer::HandleClient, this, client_fd).detach();
    }

    return 0;
}

void SimpleHttpServer::Stop() {
    running_ = false;
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

void SimpleHttpServer::HandleClient(int client_fd) {
    char buffer[4096] = {};
    const auto received = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        ::close(client_fd);
        return;
    }

    std::istringstream request(buffer);
    std::string method;
    std::string path;
    request >> method >> path;

    const auto response = BuildResponse(method, path);
    static_cast<void>(::send(client_fd, response.data(), response.size(), 0));
    ::close(client_fd);
}

std::string SimpleHttpServer::BuildResponse(const std::string& method, const std::string& path) const {
    std::string status = "200 OK";
    std::string body;

    if (method == "GET" && (path == "/healthz" || path == "/api/v1/ping")) {
        body = JsonEnvelope("OK", "success", "{}");
    } else {
        status = "404 Not Found";
        body = JsonEnvelope("SYSTEM_NOT_FOUND", "not found", "{}");
    }

    std::ostringstream response;
    response << "HTTP/1.1 " << status << "\r\n"
             << "Content-Type: application/json\r\n"
             << "Content-Length: " << body.size() << "\r\n"
             << "Connection: close\r\n\r\n"
             << body;
    return response.str();
}

std::string SimpleHttpServer::JsonEnvelope(
    const std::string& code,
    const std::string& message,
    const std::string& data) {
    return "{\"code\":\"" + code + "\",\"message\":\"" + message + "\",\"request_id\":\"req_1\",\"data\":" + data + "}";
}

}  // namespace skillops::gateway
