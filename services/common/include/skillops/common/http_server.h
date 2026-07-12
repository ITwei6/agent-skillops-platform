#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace skillops::common {

struct HttpRequest {
    std::string method;
    std::string target;
    std::string path;
    std::string query;
    std::string body;
    std::unordered_map<std::string, std::string> headers;
};

struct HttpResponse {
    int status_code = 200;
    std::string reason = "OK";
    std::string content_type = "application/json";
    std::string body;

    static HttpResponse Json(int status_code, std::string reason, std::string body);
};

using HttpHandler = std::function<HttpResponse(const HttpRequest&)>;

class HttpServer {
public:
    HttpServer(std::string host, std::uint16_t port, HttpHandler handler);
    ~HttpServer();

    HttpServer(const HttpServer&) = delete;
    HttpServer& operator=(const HttpServer&) = delete;

    int Run();
    void Stop();

private:
    void HandleClient(int client_fd);
    static std::string SerializeResponse(const HttpResponse& response);

    std::string host_;
    std::uint16_t port_;
    HttpHandler handler_;
    std::atomic<bool> running_{false};
    int listen_fd_{-1};
};

}  // namespace skillops::common
