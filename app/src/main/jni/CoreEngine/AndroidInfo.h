#pragma once

#include "Globals.h"
#include "Unity/Diagnostics.h"
#include <android/native_window.h>
#include <sys/system_properties.h>
#include <string>

namespace AndroidInfo {

inline std::string JString(JNIEnv* env, jstring value) {
    if (!value) return {};
    const char* chars = env->GetStringUTFChars(value, nullptr);
    std::string result = chars ? chars : "";
    if (chars) env->ReleaseStringUTFChars(value, chars);
    env->DeleteLocalRef(value);
    return result;
}

inline std::string GetSystemProperty(const char* key) {
    char value[PROP_VALUE_MAX] = {};
    return __system_property_get(key, value) > 0 ? std::string(value) : std::string();
}

inline void Fill(JNIEnv* env, jobject activity) {
    if (!env || !activity) return;
    env->ExceptionClear();

    jclass activityClass = env->GetObjectClass(activity);
    if (activityClass) {
        jmethodID classGetName = env->GetMethodID(activityClass, "getClass", "()Ljava/lang/Class;");
        if (classGetName) {
            jobject runtimeClass = env->CallObjectMethod(activity, classGetName);
            if (runtimeClass && !env->ExceptionCheck()) {
                jclass classClass = env->GetObjectClass(runtimeClass);
                jmethodID nameMethod = env->GetMethodID(classClass, "getName", "()Ljava/lang/String;");
                if (nameMethod) g_GameInfo.activityClass = JString(env, (jstring)env->CallObjectMethod(runtimeClass, nameMethod));
                env->DeleteLocalRef(classClass);
                env->DeleteLocalRef(runtimeClass);
            }
            env->ExceptionClear();
        }
    }
    jmethodID getPackageManager = env->GetMethodID(activityClass, "getPackageManager", "()Landroid/content/pm/PackageManager;");
    jmethodID getPackageName = env->GetMethodID(activityClass, "getPackageName", "()Ljava/lang/String;");
    if (!getPackageManager || !getPackageName || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityClass);
        return;
    }

    std::string packageName;
    jobject packageManager = env->CallObjectMethod(activity, getPackageManager);
    jstring packageString = (jstring)env->CallObjectMethod(activity, getPackageName);
    packageName = JString(env, packageString);

    g_GameInfo.packageName = packageName.empty() ? "Not available" : packageName;

    if (packageManager && !packageName.empty()) {
        jclass pmClass = env->GetObjectClass(packageManager);
        jmethodID getPackageInfo = env->GetMethodID(pmClass, "getPackageInfo", "(Ljava/lang/String;I)Landroid/content/pm/PackageInfo;");
        if (getPackageInfo && !env->ExceptionCheck()) {
            jstring nameArg = env->NewStringUTF(packageName.c_str());
            jobject packageInfo = env->CallObjectMethod(packageManager, getPackageInfo, nameArg, 0);
            env->DeleteLocalRef(nameArg);
            if (packageInfo && !env->ExceptionCheck()) {
                jclass piClass = env->GetObjectClass(packageInfo);
                jfieldID versionName = env->GetFieldID(piClass, "versionName", "Ljava/lang/String;");
                jfieldID versionCode = env->GetFieldID(piClass, "versionCode", "I");
                jfieldID applicationInfo = env->GetFieldID(piClass, "applicationInfo", "Landroid/content/pm/ApplicationInfo;");
                if (versionName) g_GameInfo.versionName = JString(env, (jstring)env->GetObjectField(packageInfo, versionName));
                if (versionCode) g_GameInfo.versionCode = std::to_string(env->GetIntField(packageInfo, versionCode));
                if (applicationInfo) {
                    jobject appInfo = env->GetObjectField(packageInfo, applicationInfo);
                    if (appInfo) {
                        jclass aiClass = env->GetObjectClass(appInfo);
                        jfieldID sourceDir = env->GetFieldID(aiClass, "sourceDir", "Ljava/lang/String;");
                        jfieldID dataDir = env->GetFieldID(aiClass, "dataDir", "Ljava/lang/String;");
                        jfieldID splitSourceDirs = env->GetFieldID(aiClass, "splitSourceDirs", "[Ljava/lang/String;");
                        jfieldID appLabel = env->GetFieldID(aiClass, "labelRes", "I");
                        if (sourceDir) g_GameInfo.apkPath = JString(env, (jstring)env->GetObjectField(appInfo, sourceDir));
                        if (dataDir) g_GameInfo.dataPath = JString(env, (jstring)env->GetObjectField(appInfo, dataDir));
                        if (splitSourceDirs) {
                            jobjectArray splits = (jobjectArray)env->GetObjectField(appInfo, splitSourceDirs);
                            if (splits) {
                                const jsize count = env->GetArrayLength(splits);
                                g_GameInfo.splitApkPaths.clear();
                                for (jsize i = 0; i < count; ++i) {
                                    jstring split = (jstring)env->GetObjectArrayElement(splits, i);
                                    const std::string path = JString(env, split);
                                    if (!g_GameInfo.splitApkPaths.empty()) g_GameInfo.splitApkPaths += "; ";
                                    g_GameInfo.splitApkPaths += path;
                                }
                                if (g_GameInfo.splitApkPaths.empty()) g_GameInfo.splitApkPaths = "None detected";
                                env->DeleteLocalRef(splits);
                            }
                        }
                        // Resolve the human-readable label through PackageManager.
                        if (appLabel && getPackageManager) {
                            jmethodID getText = env->GetMethodID(pmClass, "getApplicationLabel", "(Landroid/content/pm/ApplicationInfo;)Ljava/lang/CharSequence;");
                            if (getText) {
                                jobject label = env->CallObjectMethod(packageManager, getText, appInfo);
                                if (label) {
                                    jclass charsClass = env->GetObjectClass(label);
                                    jmethodID toString = env->GetMethodID(charsClass, "toString", "()Ljava/lang/String;");
                                    if (toString) g_GameInfo.name = JString(env, (jstring)env->CallObjectMethod(label, toString));
                                    env->DeleteLocalRef(charsClass);
                                    env->DeleteLocalRef(label);
                                }
                            }
                        }
                        env->DeleteLocalRef(aiClass);
                        env->DeleteLocalRef(appInfo);
                    }
                }
                env->DeleteLocalRef(piClass);
                env->DeleteLocalRef(packageInfo);
            }
        }
        env->ExceptionClear();
        env->DeleteLocalRef(pmClass);
        env->DeleteLocalRef(packageManager);
    }

    jclass buildClass = env->FindClass("android/os/Build");
    jclass versionClass = env->FindClass("android/os/Build$VERSION");
    if (buildClass && versionClass) {
        jfieldID model = env->GetStaticFieldID(buildClass, "MODEL", "Ljava/lang/String;");
        jfieldID release = env->GetStaticFieldID(versionClass, "RELEASE", "Ljava/lang/String;");
        if (model) g_DeviceInfo.model = JString(env, (jstring)env->GetStaticObjectField(buildClass, model));
        if (release) g_DeviceInfo.androidVersion = JString(env, (jstring)env->GetStaticObjectField(versionClass, release));
    }
    g_DeviceInfo.abi = GetSystemProperty("ro.product.cpu.abi");
    if (g_DeviceInfo.abi.empty()) g_DeviceInfo.abi = sizeof(void*) == 8 ? "arm64-v8a" : "armeabi-v7a";
    g_UnityInfo.sourcePackage = g_GameInfo.packageName;
    UnityTool::Diagnostics::Write(g_GameInfo.dataPath, "activity.package=%s activity.class=%s apk=%s split_apk=%s", g_GameInfo.packageName.c_str(), g_GameInfo.activityClass.c_str(), g_GameInfo.apkPath.c_str(), g_GameInfo.splitApkPaths.c_str());

    if (buildClass) env->DeleteLocalRef(buildClass);
    if (versionClass) env->DeleteLocalRef(versionClass);
    env->DeleteLocalRef(activityClass);
    env->ExceptionClear();
}

} // namespace AndroidInfo
