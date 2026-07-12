#include "skillops/common/logging.h"

int main() {
    skillops::common::InitLogging();
    skillops::common::LogInfo("project-service skeleton started");
    return 0;
}
