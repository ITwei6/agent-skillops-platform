#pragma once

#include "skillops/common/http_server.h"

#include <cstdint>
#include <string>

namespace skillops::common {

class HttpClient {
public:
    HttpClient(std::string host, std::uint16_t port);

    HttpResponse Send(const std::string& method, const std::string& path, const std::string& body = "") const;

private:
    std::string host_;
    std::uint16_t port_;
};

}  // namespace skillops::common
