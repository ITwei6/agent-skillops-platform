#include "skillops/common/logging.h"

int main() {
    skillops::common::InitLogging();
    skillops::common::LogInfo("identity-service skeleton started");
    return 0;
}
