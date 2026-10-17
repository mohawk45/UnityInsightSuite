#pragma once

#include "UnityInfo.h"
#include "UnityVersionParser.h"
#include "Il2CppRuntimeVersion.h"
#include "Diagnostics.h"
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <dlfcn.h>
#include <dirent.h>
#include <fstream>
#include <string>

namespace UnityTool {
namespace VersionDetector {

inline std::string ReadVersionFromFile(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return {};
    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    if (size <= 0 || size > 128 * 1024 * 1024) return {};
    file.seekg(0, std::ios::beg);
    std::string data(static_cast<std::size_t>(size), '\0');
    file.read(data.data(), size);
    if (!file) return {};
    return VersionParser::ExtractVersion(data);
}

inline std::string ReadVersionFromAsset(AAssetManager* manager, const char* assetPath) {
    if (!manager || !assetPath) return {};
    AAsset* asset = AAssetManager_open(manager, assetPath, AASSET_MODE_STREAMING);
    if (!asset) return {};
    const off_t length = AAsset_getLength(asset);
    if (length <= 0 || length > 128 * 1024 * 1024) {
        AAsset_close(asset);
        return {};
    }
    std::string data(static_cast<std::size_t>(length), '\0');
    std::size_t total = 0;
    while (total < data.size()) {
        const int count = AAsset_read(asset, data.data() + total, data.size() - total);
        if (count <= 0) break;
        total += static_cast<std::size_t>(count);
    }
    AAsset_close(asset);
    if (total == 0) return {};
    data.resize(total);
    return VersionParser::ExtractVersion(data);
}

inline std::string ReadVersionFromApk(JNIEnv* env, jobject activity, const std::string& expectedPackage, std::string& source) {
    if (!env || !activity || expectedPackage.empty() || expectedPackage == "com.ImGui") {
        source = "APK scan rejected: invalid/tool package";
        return {};
    }
    env->ExceptionClear();
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getPackageName = activityClass ? env->GetMethodID(activityClass, "getPackageName", "()Ljava/lang/String;") : nullptr;
    jmethodID getAssets = activityClass ? env->GetMethodID(activityClass, "getAssets", "()Landroid/content/res/AssetManager;") : nullptr;
    if (!getPackageName || !getAssets || env->ExceptionCheck()) {
        env->ExceptionClear();
        if (activityClass) env->DeleteLocalRef(activityClass);
        source = "Activity API unavailable";
        return {};
    }

    jstring packageString = (jstring)env->CallObjectMethod(activity, getPackageName);
    const char* packageChars = packageString ? env->GetStringUTFChars(packageString, nullptr) : nullptr;
    const std::string actualPackage = packageChars ? packageChars : "";
    if (packageChars) env->ReleaseStringUTFChars(packageString, packageChars);
    if (packageString) env->DeleteLocalRef(packageString);
    if (actualPackage != expectedPackage) {
        env->DeleteLocalRef(activityClass);
        env->ExceptionClear();
        source = "APK scan rejected: Activity package mismatch";
        return {};
    }

    jobject assetsObject = env->CallObjectMethod(activity, getAssets);
    if (env->ExceptionCheck() || !assetsObject) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityClass);
        source = "Activity AssetManager unavailable";
        return {};
    }

    AAssetManager* manager = AAssetManager_fromJava(env, assetsObject);
    const char* paths[] = {
        "bin/Data/globalgamemanagers",
        "assets/bin/Data/globalgamemanagers",
        "bin/Data/data.unity3d",
        "assets/bin/Data/data.unity3d",
        "bin/Data/resources.assets",
        "assets/bin/Data/resources.assets"
    };
    std::string result;
    for (const char* path : paths) {
        result = ReadVersionFromAsset(manager, path);
        if (!result.empty()) {
            source = "Activity AssetManager [base + split APK] / " + actualPackage + " / " + path;
            break;
        }
    }
    env->DeleteLocalRef(assetsObject);
    env->DeleteLocalRef(activityClass);
    env->ExceptionClear();
    return result;
}

inline std::string ReadVersionFromObb(const std::string& packageName, std::string& source) {
    if (packageName.empty() || packageName == "Not available") return {};
    const std::string directory = "/sdcard/Android/obb/" + packageName;
    DIR* dir = opendir(directory.c_str());
    if (!dir) return {};
    std::string result;
    while (dirent* entry = readdir(dir)) {
        const std::string name = entry->d_name;
        if (name == "." || name == "..") continue;
        if (name.find(".obb") == std::string::npos) continue;
        result = ReadVersionFromFile(directory + "/" + name);
        if (!result.empty()) {
            source = "OBB / " + name;
            break;
        }
    }
    closedir(dir);
    return result;
}

inline void ScanRuntime(const GameInfo& game, UnityInfo& info, JNIEnv* env = nullptr, jobject activity = nullptr, bool allowRuntimeApi = false) {
    info.state = RuntimeState::Scanning;
    info.architecture = sizeof(void*) == 8 ? "ARM64" : "ARMv7";
    info.sourcePackage = game.packageName;
    Diagnostics::Write(game.dataPath, "scan.begin pid=%d package=%s data=%s runtime_api=%s", getpid(), game.packageName.c_str(), game.dataPath.c_str(), allowRuntimeApi ? "enabled" : "deferred");

    void* unity = dlopen("libunity.so", RTLD_NOW | RTLD_NOLOAD);
    void* il2cpp = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
    const bool detected = unity || il2cpp;

    if (!detected) {
        info.state = RuntimeState::NotDetected;
        info.runtime = "Not detected";
        info.module = "No Unity module";
        info.backend = "Not available";
        info.version = "Not identified";
        info.source = "Loaded module scan";
        info.confidence = "None";
        info.profile = "Disabled safely";
    } else {
        info.state = RuntimeState::Detected;
        info.runtime = "Unity";
        info.module = il2cpp ? "libil2cpp.so" : "libunity.so";
        info.backend = il2cpp ? "IL2CPP" : "Mono / Unity runtime";

        std::string version;
        std::string source;
        if (il2cpp && allowRuntimeApi) {
            Diagnostics::Write(game.dataPath, "il2cpp.probe.begin module=%s", info.module.c_str());
            version = Il2CppRuntime::ReadVersion(il2cpp, source);
            Diagnostics::Write(game.dataPath, "il2cpp.probe.result version=%s source=%s", version.empty() ? "<empty>" : version.c_str(), source.empty() ? "<none>" : source.c_str());
        } else if (il2cpp) {
            source = "IL2CPP Runtime probe deferred until first rendered frame";
            Diagnostics::Write(game.dataPath, "il2cpp.probe.deferred reason=runtime_not_ready");
        }

        if (!version.empty() && VersionParser::IsValidVersion(version)) {
            info.version = version;
            info.source = source;
            info.confidence = "Verified";
            info.profile = "Pending profile validation";
        }

        if (version.empty()) {
            source.clear();
        }
        if (version.empty() && env && activity) version = ReadVersionFromApk(env, activity, game.packageName, source);

        if (version.empty() && !game.dataPath.empty() && game.dataPath != "Not available") {
            const char* names[] = {"globalgamemanagers", "data.unity3d", "resources.assets", "boot.config"};
            for (const char* name : names) {
                version = ReadVersionFromFile(game.dataPath + "/" + name);
                if (!version.empty()) {
                    source = "Unity data file / " + std::string(name) + " / " + game.packageName;
                    break;
                }
            }
        }

        if (version.empty()) version = ReadVersionFromObb(game.packageName, source);

        if (!version.empty() && VersionParser::IsValidVersion(version)) {
            info.version = version;
            info.source = source;
            info.confidence = source.find("IL2CPP Runtime API") != std::string::npos
                ? "Verified"
                : (source.find("globalgamemanagers") != std::string::npos ? "High" : "Medium");
            info.profile = "Pending profile validation";
            Diagnostics::Write(game.dataPath, "scan.success version=%s confidence=%s source=%s", info.version.c_str(), info.confidence.c_str(), info.source.c_str());
        } else {
            info.version = "Not verified";
            info.source = source.empty() ? "No readable Unity source" : source;
            info.confidence = "Low";
            info.profile = "Disabled safely";
            Diagnostics::Write(game.dataPath, "scan.no_verified_version source=%s", info.source.c_str());
        }
    }

    // Do not dlclose engine handles owned by the running game.
}

} // namespace VersionDetector

inline void ScanRuntime(UnityInfo& info, const GameInfo& game, JNIEnv* env = nullptr, jobject activity = nullptr, bool allowRuntimeApi = false) {
    VersionDetector::ScanRuntime(game, info, env, activity, allowRuntimeApi);
}

} // namespace UnityTool
