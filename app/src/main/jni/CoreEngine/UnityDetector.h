#pragma once

#include "Globals.h"
#include "Unity/UnityVersionDetector.h"

namespace UnityDetector {

inline void ScanLoadedRuntime(JNIEnv* env = nullptr, jobject activity = nullptr, bool allowRuntimeApi = false) {
    JNIEnv* scanEnv = env;
    bool attachedHere = false;

    if (!scanEnv && g_JVM && g_ActivityGlobalRef) {
        if (g_JVM->GetEnv(reinterpret_cast<void**>(&scanEnv), JNI_VERSION_1_6) != JNI_OK) {
            if (g_JVM->AttachCurrentThread(&scanEnv, nullptr) == JNI_OK) attachedHere = true;
        }
        if (!activity) activity = g_ActivityGlobalRef;
    }

    UnityTool::ScanRuntime(g_UnityInfo, g_GameInfo, scanEnv, activity, allowRuntimeApi);

    if (attachedHere) g_JVM->DetachCurrentThread();
}

} // namespace UnityDetector
