#include "skillops/common/request_context.h"

#include <cstdint>
#include <atomic>
#include <utility>

namespace skillops::common {

namespace {

thread_local std::string g_current_request_id;
std::atomic<std::uint64_t> g_request_sequence{1};

}  // namespace

void SetCurrentRequestId(std::string request_id) {
    g_current_request_id = std::move(request_id);
}

const std::string& CurrentRequestId() {
    return g_current_request_id;
}

void ClearCurrentRequestId() {
    g_current_request_id.clear();
}

std::string GenerateRequestId() {
    return "req_" + std::to_string(g_request_sequence.fetch_add(1, std::memory_order_relaxed));
}

}  // namespace skillops::common
