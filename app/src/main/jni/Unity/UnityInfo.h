#pragma once

#include <cstdint>
#include <string>

namespace UnityTool {

enum class RuntimeState : std::uint8_t {
    Unknown = 0,
    Scanning,
    Detected,
    NotDetected,
    Unsupported
};

struct GameInfo {
    std::string name = "Not available";
    std::string packageName = "Not available";
    std::string versionName = "Not available";
    std::string versionCode = "Not available";
    std::string dataPath = "Not available";
    std::string apkPath = "Not available";
    std::string splitApkPaths = "None detected";
    std::string activityClass = "Not available";
};

struct UnityInfo {
    RuntimeState state = RuntimeState::Unknown;
    std::string runtime = "Not available";
    std::string version = "Not verified";
    std::string backend = "Not available";
    std::string module = "Not loaded";
    std::string architecture = "Not available";
    std::string graphicsApi = "OpenGL ES 3";
    std::string source = "No trusted source";
    std::string confidence = "Unknown";
    std::string profile = "Disabled safely";
    std::string sourcePackage = "Not verified";
};

struct DeviceInfo {
    std::string androidVersion = "Not available";
    std::string model = "Not available";
    std::string abi = "Not available";
    std::string resolution = "Not available";
};

} // namespace UnityTool
