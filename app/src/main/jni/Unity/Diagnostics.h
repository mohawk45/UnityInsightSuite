#pragma once

#include <android/log.h>
#include <cstdarg>
#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>
#include <unistd.h>

namespace UnityTool {
namespace Diagnostics {

inline std::mutex g_LogMutex;

inline void Write(const std::string& dataPath, const char* format, ...) {
    char message[2048] = {};
    va_list args;
    va_start(args, format);
    std::vsnprintf(message, sizeof(message), format, args);
    va_end(args);

    __android_log_print(ANDROID_LOG_INFO, "UnityInsight", "%s", message);

    if (dataPath.empty() || dataPath == "Not available") return;
    const std::string path = dataPath + "/unity_insight_diagnostic.log";
    std::lock_guard<std::mutex> lock(g_LogMutex);
    std::ofstream file(path, std::ios::app);
    if (!file) return;
    file << message << '\n';
    file.flush();
}

} // namespace Diagnostics
} // namespace UnityTool
