#pragma once

#include <string>

namespace skillops::common {

void SetCurrentRequestId(std::string request_id);
const std::string& CurrentRequestId();
void ClearCurrentRequestId();
std::string GenerateRequestId();

}  // namespace skillops::common
