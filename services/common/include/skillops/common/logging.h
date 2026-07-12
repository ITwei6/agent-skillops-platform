#pragma once

#if __has_include(<microkit/log.h>)
#include <microkit/log.h>
#elif __has_include(<tew_scaffold/log.h>)
#include <tew_scaffold/log.h>
#elif __has_include(<bite_scaffold/log.h>)
#include <bite_scaffold/log.h>
#endif

#include <iostream>
#include <string>

namespace skillops::common {

inline void InitLogging() {
#if defined(TEWLOG_LEVEL_TRACE) || __has_include(<tew_scaffold/log.h>) || __has_include(<bite_scaffold/log.h>) || __has_include(<microkit/log.h>)
    tewlog::log_settings settings;
    settings.async = false;
    settings.level = 2;
    settings.format = "[%H:%M:%S][%-7l] %v";
    settings.path = "stdout";
    tewlog::tewlog_init(settings);
#endif
}

inline void LogInfo(const std::string& message) {
#if __has_include(<tew_scaffold/log.h>) || __has_include(<bite_scaffold/log.h>) || __has_include(<microkit/log.h>)
    if (tewlog::g_logger) {
        tewlog::g_logger->info(message);
        return;
    }
#endif
    std::cout << message << std::endl;
}

}  // namespace skillops::common
