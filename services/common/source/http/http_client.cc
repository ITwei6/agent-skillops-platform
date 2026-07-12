#include "skillops/common/http_client.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace skillops::common {

namespace {

std::string ReasonForStatus(int status_code) {
    switch (status_code) {
    case 200:
        return "OK";
    case 400:
        return "Bad Request";
    case 404:
        return "Not Found";
    case 500:
        return "Internal Server Error";
    default:
        return "OK";
    }
}

HttpResponse ParseRawResponse(const std::string& raw) {
    const auto header_end = raw.find("\r\n\r\n");
    const auto header_part = header_end == std::string::npos ? raw : raw.substr(0, header_end);

    std::istringstream headers(header_part);
    std::string http_version;
    int status_code = 500;
    headers >> http_version >> status_code;

    HttpResponse response;
    response.status_code = status_code;
    response.reason = ReasonForStatus(status_code);
    response.body = header_end == std::string::npos ? "" : raw.substr(header_end + 4);
    return response;
}

}  // namespace

HttpClient::HttpClient(std::string host, std::uint16_t port)
    : host_(std::move(host)), port_(port) {}

HttpResponse HttpClient::Send(const std::string& method, const std::string& path, const std::string& body) const {
    const int client_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd < 0) {
        throw std::runtime_error(std::string("socket failed: ") + std::strerror(errno));
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(port_);
    if (::inet_pton(AF_INET, host_.c_str(), &address.sin_addr) != 1) {
        ::close(client_fd);
        throw std::runtime_error("invalid IPv4 host: " + host_);
    }

    if (::connect(client_fd, reinterpret_cast<sockaddr*>(&address), sizeof(address)) < 0) {
        ::close(client_fd);
        throw std::runtime_error(std::string("connect failed: ") + std::strerror(errno));
    }

    std::ostringstream request;
    request << method << " " << path << " HTTP/1.1\r\n"
            << "Host: " << host_ << ":" << port_ << "\r\n"
            << "Content-Type: application/json\r\n"
            << "Content-Length: " << body.size() << "\r\n"
            << "Connection: close\r\n\r\n"
            << body;

    const auto serialized = request.str();
    static_cast<void>(::send(client_fd, serialized.data(), serialized.size(), 0));

    std::string raw;
    char buffer[4096] = {};
    while (true) {
        const auto received = ::recv(client_fd, buffer, sizeof(buffer), 0);
        if (received <= 0) {
            break;
        }
        raw.append(buffer, static_cast<std::size_t>(received));
    }

    ::close(client_fd);
    return ParseRawResponse(raw);
}

}  // namespace skillops::common
