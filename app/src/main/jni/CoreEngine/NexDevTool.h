#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <dlfcn.h>
#include <android/log.h>

// تعريف أنواع دوال الـ IL2CPP بشكل دقيق
using il2cpp_domain_get_t = void* (*)();
using il2cpp_domain_get_assemblies_t = void** (*)(const void* domain, size_t* size);
using il2cpp_assembly_get_image_t = void* (*)(const void* assembly);
using il2cpp_image_get_name_t = const char* (*)(const void* image);
using il2cpp_class_from_name_t = void* (*)(void* image, const char* namespaze, const char* name);
using il2cpp_class_get_method_from_name_t = void* (*)(void* clazz, const char* name, int argsCount);
using il2cpp_field_get_offset_t = size_t (*)(void* field);
using il2cpp_runtime_invoke_t = void* (*)(const void* method, void* obj, void** params, void** exc);
using il2cpp_array_new_t = void* (*)(void* elementType, uint32_t length);
using il2cpp_array_get_t = void* (*)(void* array, int32_t index);
using il2cpp_object_unbox_t = void* (*)(void* obj);
using il2cpp_object_get_class_t = void* (*)(void* obj);
using il2cpp_string_length_t = int32_t (*)(void* string);
using il2cpp_string_chars_t = const char16_t* (*)(void* string);

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
    inline il2cpp_domain_get_t s_domain_get = nullptr;
    inline il2cpp_domain_get_assemblies_t s_domain_get_assemblies = nullptr;
    inline il2cpp_assembly_get_image_t s_assembly_get_image = nullptr;
    inline il2cpp_image_get_name_t s_image_get_name = nullptr;
    inline il2cpp_class_from_name_t s_class_from_name = nullptr;
    inline il2cpp_class_get_method_from_name_t s_class_get_method_from_name = nullptr;
    inline il2cpp_field_get_offset_t s_field_get_offset = nullptr;
    inline il2cpp_runtime_invoke_t s_runtime_invoke = nullptr;
    inline il2cpp_array_new_t s_array_new = nullptr;
    inline il2cpp_array_get_t s_array_get = nullptr;
    inline il2cpp_object_unbox_t s_object_unbox = nullptr;
    inline il2cpp_object_get_class_t s_object_get_class = nullptr;
    inline il2cpp_string_length_t s_string_length = nullptr;
    inline il2cpp_string_chars_t s_string_chars = nullptr;

    inline std::vector<CameraData> g_LoadedCameras;
    inline std::vector<GameObjectData> g_SceneObjects;
    inline bool g_IsInitialized = false;

    inline std::string Utf16ToUtf8(const char16_t* chars, int32_t length) {
        if (!chars || length <= 0 || length > 4096) return {};
        std::string result;
        result.reserve(static_cast<size_t>(length));
        for (int32_t i = 0; i < length; ++i) {
            const uint32_t code = static_cast<uint32_t>(chars[i]);
            if (code <= 0x7F) {
                result.push_back(static_cast<char>(code));
            } else if (code <= 0x7FF) {
                result.push_back(static_cast<char>(0xC0 | (code >> 6)));
                result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
            } else {
                result.push_back(static_cast<char>(0xE0 | (code >> 12)));
                result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3F)));
                result.push_back(static_cast<char>(0x80 | (code & 0x3F)));
            }
        }
        return result;
    }

    inline void ResolveIl2CppSymbols() {
        if (!libil2cpp_handle) {
            libil2cpp_handle = dlopen("libil2cpp.so", RTLD_LAZY);
        }
        if (!libil2cpp_handle) {
            __android_log_print(ANDROID_LOG_ERROR, "NexDevTool", "Failed to dlopen libil2cpp.so");
            return;
        }

        s_domain_get = reinterpret_cast<il2cpp_domain_get_t>(dlsym(libil2cpp_handle, "il2cpp_domain_get"));
        s_domain_get_assemblies = reinterpret_cast<il2cpp_domain_get_assemblies_t>(dlsym(libil2cpp_handle, "il2cpp_domain_get_assemblies"));
        s_assembly_get_image = reinterpret_cast<il2cpp_assembly_get_image_t>(dlsym(libil2cpp_handle, "il2cpp_assembly_get_image"));
        s_image_get_name = reinterpret_cast<il2cpp_image_get_name_t>(dlsym(libil2cpp_handle, "il2cpp_image_get_name"));
        s_class_from_name = reinterpret_cast<il2cpp_class_from_name_t>(dlsym(libil2cpp_handle, "il2cpp_class_from_name"));
        s_class_get_method_from_name = reinterpret_cast<il2cpp_class_get_method_from_name_t>(dlsym(libil2cpp_handle, "il2cpp_class_get_method_from_name"));
        s_runtime_invoke = reinterpret_cast<il2cpp_runtime_invoke_t>(dlsym(libil2cpp_handle, "il2cpp_runtime_invoke"));
        s_array_new = reinterpret_cast<il2cpp_array_new_t>(dlsym(libil2cpp_handle, "il2cpp_array_new"));
        s_array_get = reinterpret_cast<il2cpp_array_get_t>(dlsym(libil2cpp_handle, "il2cpp_array_get"));
        s_object_unbox = reinterpret_cast<il2cpp_object_unbox_t>(dlsym(libil2cpp_handle, "il2cpp_object_unbox"));
        s_object_get_class = reinterpret_cast<il2cpp_object_get_class_t>(dlsym(libil2cpp_handle, "il2cpp_object_get_class"));
        s_string_length = reinterpret_cast<il2cpp_string_length_t>(dlsym(libil2cpp_handle, "il2cpp_string_length"));
        s_string_chars = reinterpret_cast<il2cpp_string_chars_t>(dlsym(libil2cpp_handle, "il2cpp_string_chars"));
    }

    inline bool InitializeIl2Cpp() {
        if (g_IsInitialized) return true;

        ResolveIl2CppSymbols();

        if (!s_domain_get || !s_domain_get_assemblies || !s_assembly_get_image || !s_image_get_name ||
            !s_class_from_name || !s_class_get_method_from_name || !s_runtime_invoke ||
            !s_array_new || !s_array_get || !s_object_unbox || !s_object_get_class ||
            !s_string_length || !s_string_chars) {
            __android_log_print(ANDROID_LOG_ERROR, "NexDevTool", "IL2CPP symbols unavailable for camera scanning");
            return false;
        }

        g_IsInitialized = true;
        return true;
    }

    inline std::string GetIl2CppStringValue(void* managedString) {
        if (!managedString || !s_string_length || !s_string_chars) return {};
        const int32_t length = s_string_length(managedString);
        if (length <= 0) return {};
        const char16_t* chars = s_string_chars(managedString);
        return Utf16ToUtf8(chars, length);
    }

    inline void ScanAllCameras() {
        g_LoadedCameras.clear();
        if (!InitializeIl2Cpp()) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        void* domain = s_domain_get();
        if (!domain) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        size_t assembly_count = 0;
        void** assemblies = s_domain_get_assemblies(domain, &assembly_count);
        void* cameraClass = nullptr;
        if (assemblies && assembly_count > 0) {
            for (size_t i = 0; i < assembly_count; ++i) {
                void* image = s_assembly_get_image(assemblies[i]);
                if (!image) continue;
                const char* image_name = s_image_get_name(image);
                if (image_name && (std::string(image_name) == "UnityEngine.CoreModule" || std::string(image_name) == "UnityEngine.CoreModule.dll")) {
                    cameraClass = s_class_from_name(image, "UnityEngine", "Camera");
                    if (cameraClass) break;
                }
            }
        }

        if (!cameraClass) {
            __android_log_print(ANDROID_LOG_WARN, "NexDevTool", "UnityEngine.Camera class not found");
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        void* getAllCamerasMethod = s_class_get_method_from_name(cameraClass, "GetAllCameras", 1);
        void* objectClass = nullptr;
        void* objectNameMethod = nullptr;
        void* objectType = nullptr;

        if (!getAllCamerasMethod) {
            __android_log_print(ANDROID_LOG_WARN, "NexDevTool", "UnityEngine.Camera.GetAllCameras method not found");
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        objectType = s_class_from_name(nullptr, "UnityEngine", "Object");
        if (!objectType) {
            __android_log_print(ANDROID_LOG_WARN, "NexDevTool", "UnityEngine.Object class not found");
            objectType = cameraClass;
        }
        objectClass = objectType;
        objectNameMethod = s_class_get_method_from_name(objectClass, "get_name", 0);

        const int32_t defaultCapacity = 64;
        void* cameraArray = s_array_new(cameraClass, static_cast<uint32_t>(defaultCapacity));
        if (!cameraArray) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        void* args[1] = { cameraArray };
        void* invokeException = nullptr;
        void* invokeResult = s_runtime_invoke(getAllCamerasMethod, nullptr, args, &invokeException);
        if (invokeException || !invokeResult) {
            __android_log_print(ANDROID_LOG_WARN, "NexDevTool", "Camera.GetAllCameras invocation failed");
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
            return;
        }

        int totalCameras = defaultCapacity;
        if (s_object_unbox) {
            const int32_t* outCount = reinterpret_cast<int32_t*>(s_object_unbox(invokeResult));
            if (outCount) {
                totalCameras = *outCount;
            }
        }
        if (totalCameras <= 0) {
            totalCameras = 0;
        }

        for (int i = 0; i < totalCameras; ++i) {
            void* cameraObj = s_array_get(cameraArray, i);
            if (!cameraObj) continue;

            std::string cameraName = "Camera";
            if (objectNameMethod) {
                void* nameResult = s_runtime_invoke(objectNameMethod, cameraObj, nullptr, &invokeException);
                if (!invokeException && nameResult) {
                    cameraName = GetIl2CppStringValue(nameResult);
                    if (cameraName.empty()) {
                        cameraName = "Camera";
                    }
                }
            }

            CameraData cam;
            cam.instance = reinterpret_cast<uintptr_t>(cameraObj);
            cam.name = cameraName;
            cam.isActive = true;
            g_LoadedCameras.push_back(cam);
        }

        if (g_LoadedCameras.empty()) {
            CameraData fallback;
            fallback.instance = 0;
            fallback.name = "No Camera Detected";
            fallback.isActive = false;
            g_LoadedCameras.push_back(fallback);
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
