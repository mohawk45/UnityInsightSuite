#pragma once

#include "UnityVersionParser.h"
#include <dlfcn.h>
#include <cstddef>
#include <cstdint>
#include <string>

namespace UnityTool {
namespace Il2CppRuntime {

struct Il2CppDomain;
struct Il2CppThread;
struct Il2CppAssembly;
struct Il2CppImage;
struct Il2CppClass;
struct MethodInfo;
struct Il2CppException;
struct Il2CppObject {
    void* klass;
    void* monitor;
};
struct Il2CppString {
    Il2CppObject object;
    int32_t length;
    char16_t chars[1];
};

using DomainGet = Il2CppDomain* (*)();
using ThreadAttach = Il2CppThread* (*)(Il2CppDomain*);
using DomainGetAssemblies = const Il2CppAssembly** (*)(const Il2CppDomain*, std::size_t*);
using AssemblyGetImage = const Il2CppImage* (*)(const Il2CppAssembly*);
using ImageGetName = const char* (*)(const Il2CppImage*);
using ClassFromName = Il2CppClass* (*)(const Il2CppImage*, const char*, const char*);
using ClassGetMethod = const MethodInfo* (*)(Il2CppClass*, const char*, int);
using RuntimeInvoke = Il2CppObject* (*)(const MethodInfo*, void*, void**, Il2CppException**);

struct Api {
    DomainGet domainGet = nullptr;
    ThreadAttach threadAttach = nullptr;
    DomainGetAssemblies domainGetAssemblies = nullptr;
    AssemblyGetImage assemblyGetImage = nullptr;
    ImageGetName imageGetName = nullptr;
    ClassFromName classFromName = nullptr;
    ClassGetMethod classGetMethod = nullptr;
    RuntimeInvoke runtimeInvoke = nullptr;

    bool Ready() const {
        return domainGet && threadAttach && domainGetAssemblies && assemblyGetImage &&
               imageGetName && classFromName && classGetMethod && runtimeInvoke;
    }
};

template <typename T>
inline T Resolve(void* handle, const char* name) {
    return reinterpret_cast<T>(dlsym(handle, name));
}

inline Api LoadApi(void* handle) {
    Api api;
    if (!handle) return api;
    api.domainGet = Resolve<DomainGet>(handle, "il2cpp_domain_get");
    api.threadAttach = Resolve<ThreadAttach>(handle, "il2cpp_thread_attach");
    api.domainGetAssemblies = Resolve<DomainGetAssemblies>(handle, "il2cpp_domain_get_assemblies");
    api.assemblyGetImage = Resolve<AssemblyGetImage>(handle, "il2cpp_assembly_get_image");
    api.imageGetName = Resolve<ImageGetName>(handle, "il2cpp_image_get_name");
    api.classFromName = Resolve<ClassFromName>(handle, "il2cpp_class_from_name");
    api.classGetMethod = Resolve<ClassGetMethod>(handle, "il2cpp_class_get_method_from_name");
    api.runtimeInvoke = Resolve<RuntimeInvoke>(handle, "il2cpp_runtime_invoke");
    return api;
}

inline bool IsCoreModule(const char* name) {
    if (!name) return false;
    const std::string image(name);
    return image == "UnityEngine.CoreModule" || image == "UnityEngine.CoreModule.dll";
}

inline std::string Utf16ToUtf8(const char16_t* chars, int32_t length) {
    if (!chars || length <= 0 || length > 256) return {};
    std::string result;
    result.reserve(static_cast<std::size_t>(length));
    for (int32_t i = 0; i < length; ++i) {
        const uint32_t code = chars[i];
        if (code <= 0x7f) {
            result.push_back(static_cast<char>(code));
        } else if (code <= 0x7ff) {
            result.push_back(static_cast<char>(0xc0 | (code >> 6)));
            result.push_back(static_cast<char>(0x80 | (code & 0x3f)));
        } else {
            result.push_back(static_cast<char>(0xe0 | (code >> 12)));
            result.push_back(static_cast<char>(0x80 | ((code >> 6) & 0x3f)));
            result.push_back(static_cast<char>(0x80 | (code & 0x3f)));
        }
    }
    return result;
}

inline std::string ReadVersion(void* handle, std::string& source) {
    Api api = LoadApi(handle);
    if (!api.Ready()) {
        source = "IL2CPP Runtime API unavailable";
        return {};
    }

    Il2CppDomain* domain = api.domainGet();
    if (!domain || !api.threadAttach(domain)) {
        source = "IL2CPP domain/thread unavailable";
        return {};
    }

    std::size_t assemblyCount = 0;
    const Il2CppAssembly** assemblies = api.domainGetAssemblies(domain, &assemblyCount);
    if (!assemblies || assemblyCount == 0 || assemblyCount > 4096) {
        source = "IL2CPP assemblies unavailable";
        return {};
    }

    const Il2CppImage* coreModule = nullptr;
    for (std::size_t i = 0; i < assemblyCount; ++i) {
        const Il2CppImage* image = api.assemblyGetImage(assemblies[i]);
        if (image && IsCoreModule(api.imageGetName(image))) {
            coreModule = image;
            break;
        }
    }
    if (!coreModule) {
        source = "UnityEngine.CoreModule not found";
        return {};
    }

    Il2CppClass* application = api.classFromName(coreModule, "UnityEngine", "Application");
    const MethodInfo* getter = application ? api.classGetMethod(application, "get_unityVersion", 0) : nullptr;
    if (!getter) {
        source = "Application.get_unityVersion not found";
        return {};
    }

    Il2CppException* exception = nullptr;
    Il2CppObject* object = api.runtimeInvoke(getter, nullptr, nullptr, &exception);
    if (exception || !object) {
        source = "Application.get_unityVersion invocation failed";
        return {};
    }

    const Il2CppString* unityVersion = reinterpret_cast<const Il2CppString*>(object);
    const std::string version = Utf16ToUtf8(unityVersion->chars, unityVersion->length);
    if (!VersionParser::IsValidVersion(version)) {
        source = "IL2CPP returned invalid Unity version";
        return {};
    }

    source = "IL2CPP Runtime API / UnityEngine.Application.get_unityVersion";
    return version;
}

} // namespace Il2CppRuntime
} // namespace UnityTool
