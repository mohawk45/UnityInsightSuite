#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <dlfcn.h>
#include <stdio.h>
#include <android/log.h>

// تعريف أنواع دوال الـ IL2CPP بشكل دقيق
typedef void* (*il2cpp_domain_get_t)();
typedef void** (*il2cpp_domain_get_assemblies_t)(const void* domain, size_t* size);
typedef void* (*il2cpp_assembly_get_image_t)(const void* assembly);
typedef void* (*il2cpp_class_from_name_t)(void* image, const char* namespaze, const char* name);
typedef void* (*il2cpp_class_get_method_from_name_t)(void* clazz, const char* name, int argsCount);
typedef void* (*il2cpp_class_get_field_from_name_t)(void* clazz, const char* name);
typedef size_t (*il2cpp_field_get_offset_t)(void* field);

#define LOG_TAG "NexDev"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

struct CameraData {
    uintptr_t instance;
    std::string name;
    bool isActive;
};

struct GameObjectData {
    uintptr_t instance;
    std::string name;
    float position[3];
    bool isSelected;
};

namespace NexDevEngine {

    inline void* libil2cpp_handle = nullptr;
    
    // تعريف المؤشرات بأسماء واضحة لتجنب تداخل التسميات
    inline il2cpp_domain_get_t s_domain_get = nullptr;
    inline il2cpp_domain_get_assemblies_t s_domain_get_assemblies = nullptr;
    inline il2cpp_assembly_get_image_t s_assembly_get_image = nullptr;
    inline il2cpp_class_from_name_t s_class_from_name = nullptr;
    inline il2cpp_class_get_method_from_name_t s_class_get_method_from_name = nullptr;
    inline il2cpp_class_get_field_from_name_t s_class_get_field_from_name = nullptr;
    inline il2cpp_field_get_offset_t s_field_get_offset = nullptr;

    inline std::vector<CameraData> g_LoadedCameras;
    inline std::vector<GameObjectData> g_SceneObjects;
    inline bool g_IsInitialized = false;

    // تهيئة مؤشرات الـ dlsym الخاصة بـ libil2cpp ديناميكياً
    inline bool InitializeIl2Cpp() {
        if (g_IsInitialized) return true;

        libil2cpp_handle = dlopen("libil2cpp.so", RTLD_LAZY);
        if (!libil2cpp_handle) {
            const char* err = dlerror();
            LOGE("dlopen(libil2cpp.so) failed: %s", err ? err : "unknown");
            printf("[NexDev] dlopen(libil2cpp.so) failed: %s\n", err ? err : "unknown");
            return false;
        }

        // clear dlerror
        dlerror();
        s_domain_get = (il2cpp_domain_get_t)dlsym(libil2cpp_handle, "il2cpp_domain_get");
        s_domain_get_assemblies = (il2cpp_domain_get_assemblies_t)dlsym(libil2cpp_handle, "il2cpp_domain_get_assemblies");
        s_assembly_get_image = (il2cpp_assembly_get_image_t)dlsym(libil2cpp_handle, "il2cpp_assembly_get_image");
        s_class_from_name = (il2cpp_class_from_name_t)dlsym(libil2cpp_handle, "il2cpp_class_from_name");
        s_class_get_method_from_name = (il2cpp_class_get_method_from_name_t)dlsym(libil2cpp_handle, "il2cpp_class_get_method_from_name");
        s_class_get_field_from_name = (il2cpp_class_get_field_from_name_t)dlsym(libil2cpp_handle, "il2cpp_class_get_field_from_name");
        s_field_get_offset = (il2cpp_field_get_offset_t)dlsym(libil2cpp_handle, "il2cpp_field_get_offset");

        const char* dlsym_err = dlerror();
        if (dlsym_err) {
            LOGE("dlsym error: %s", dlsym_err);
            printf("[NexDev] dlsym error: %s\n", dlsym_err);
        }

        LOGI("symbols: domain_get=%p assemblies=%p image=%p class_from_name=%p",
             (void*)s_domain_get, (void*)s_domain_get_assemblies, (void*)s_assembly_get_image, (void*)s_class_from_name);
        printf("[NexDev] symbols: domain_get=%p assemblies=%p image=%p class_from_name=%p\n",
               (void*)s_domain_get, (void*)s_domain_get_assemblies, (void*)s_assembly_get_image, (void*)s_class_from_name);

        if (s_domain_get && s_class_from_name) {
            g_IsInitialized = true;
        } else {
            LOGE("InitializeIl2Cpp failed: missing required symbols");
            printf("[NexDev] InitializeIl2Cpp failed: missing required symbols\n");
        }
        return g_IsInitialized;
    }

    // دالة ديناميكية لجلب الكاميرات الكلاسيكية عبر الـ API
    inline void ScanAllCameras() {
        g_LoadedCameras.clear();
        if (!InitializeIl2Cpp()) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected (Init Failed)";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        void* domain = s_domain_get();
        if (!domain) {
            LOGE("s_domain_get returned null");
            printf("[NexDev] s_domain_get returned null\n");
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected (No Domain)";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        size_t size = 0;
        void** assemblies = s_domain_get_assemblies(domain, &size);
        LOGI("assemblies count = %zu", size);
        printf("[NexDev] assemblies count = %zu\n", size);
        for (size_t i = 0; i < size; ++i) {
            void* image = s_assembly_get_image(assemblies[i]);
            if (!image) continue;

            void* cameraClass = s_class_from_name(image, "UnityEngine", "Camera");
            if (cameraClass) {
                CameraData cam;
                cam.instance = reinterpret_cast<uintptr_t>(cameraClass);
                cam.name = "UnityEngine.Camera [Dynamic Hooked]";
                cam.isActive = true;
                g_LoadedCameras.push_back(cam);
                LOGI("Camera class found: %p", cameraClass);
                printf("[NexDev] Camera class found: %p\n", cameraClass);
                break;
            }
        }

        if (g_LoadedCameras.empty()) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected (No Camera class found)";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            LOGI("No Camera class found in assemblies");
            printf("[NexDev] No Camera class found in assemblies\n");
        }
    }

    // دالة ديناميكية لفحص واستخراج كائنات المشهد
    inline void RefreshSceneObjects() {
        g_SceneObjects.clear();
        if (!InitializeIl2Cpp()) return;

        void* domain = s_domain_get();
        if (!domain) return;

        size_t size = 0;
        void** assemblies = s_domain_get_assemblies(domain, &size);
        for (size_t i = 0; i < size; ++i) {
            void* image = s_assembly_get_image(assemblies[i]);
            if (!image) continue;

            void* goClass = s_class_from_name(image, "UnityEngine", "GameObject");
            if (goClass) {
                GameObjectData obj;
                obj.instance = reinterpret_cast<uintptr_t>(goClass);
                obj.name = "Dynamic_GameObject_Root";
                obj.position[0] = 0.0f;
                obj.position[1] = 0.0f;
                obj.position[2] = 0.0f;
                obj.isSelected = false;
                g_SceneObjects.push_back(obj);
                break;
            }
        }
    }

    inline void SetAllObjectsSelected(bool selected) {
        for (auto& obj : g_SceneObjects) {
            obj.isSelected = selected;
        }
    }

    inline void InitEngine() {
        if (InitializeIl2Cpp()) {
            ScanAllCameras();
            RefreshSceneObjects();
        }
    }

} // namespace NexDevEngine
