#include "skillops/common/http_server.h"

#include "skillops/common/logging.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cctype>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace skillops::common {

namespace {

std::string Trim(std::string value) {
    const auto first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) {
        return "";
    }
    const auto last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

HttpRequest ParseRequest(const std::string& raw) {
    HttpRequest request;

    const auto header_end = raw.find("\r\n\r\n");
    const auto header_part = header_end == std::string::npos ? raw : raw.substr(0, header_end);
    request.body = header_end == std::string::npos ? "" : raw.substr(header_end + 4);

    std::istringstream headers(header_part);
    headers >> request.method >> request.target;
    const auto query_pos = request.target.find('?');
    request.path = query_pos == std::string::npos ? request.target : request.target.substr(0, query_pos);
    request.query = query_pos == std::string::npos ? "" : request.target.substr(query_pos + 1);

    std::string line;
    std::getline(headers, line);
    while (std::getline(headers, line)) {
        const auto colon_pos = line.find(':');
        if (colon_pos == std::string::npos) {
            continue;
        }
        auto name = Trim(line.substr(0, colon_pos));
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char ch) {
            return static_cast<char>(std::tolower(ch));
        });
        request.headers[name] = Trim(line.substr(colon_pos + 1));
    }

    return request;
}

}  // namespace

HttpResponse HttpResponse::Json(int status_code, std::string reason, std::string body) {
    HttpResponse response;
    response.status_code = status_code;
    response.reason = std::move(reason);
    response.body = std::move(body);
    return response;
}

HttpServer::HttpServer(std::string host, std::uint16_t port, HttpHandler handler)
    : host_(std::move(host)), port_(port), handler_(std::move(handler)) {}

HttpServer::~HttpServer() {
    Stop();
}

int HttpServer::Run() {
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
    LogInfo("http server listening on " + host_ + ":" + std::to_string(port_));

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

        std::thread(&HttpServer::HandleClient, this, client_fd).detach();
    }

    return 0;
}

void HttpServer::Stop() {
    running_ = false;
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

void HttpServer::HandleClient(int client_fd) {
    char buffer[8192] = {};
    const auto received = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    if (received <= 0) {
        ::close(client_fd);
        return;
    }

    HttpResponse response;
    try {
        response = handler_(ParseRequest(std::string(buffer, static_cast<std::size_t>(received))));
    } catch (const std::exception& ex) {
        response = HttpResponse::Json(500, "Internal Server Error", "{\"code\":\"SYSTEM_ERROR\",\"message\":\"internal error\",\"request_id\":\"req_1\",\"data\":{}}");
        LogInfo(std::string("http handler failed: ") + ex.what());
    }

    const auto serialized = SerializeResponse(response);
    static_cast<void>(::send(client_fd, serialized.data(), serialized.size(), 0));
    ::close(client_fd);
}

std::string HttpServer::SerializeResponse(const HttpResponse& response) {
    std::ostringstream out;
    out << "HTTP/1.1 " << response.status_code << " " << response.reason << "\r\n"
        << "Content-Type: " << response.content_type << "\r\n"
        << "Content-Length: " << response.body.size() << "\r\n"
        << "Connection: close\r\n\r\n"
        << response.body;
    return out.str();
}

}  // namespace skillops::common
