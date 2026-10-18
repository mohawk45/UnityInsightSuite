#pragma once

#include "imgui.h"
#include "Globals.h"
#include "UnityDetector.h"
#include "NexDevTool.h" // ربط ملف المحرك الخلفي الجديد

namespace ToolUI {

inline const ImVec4 kAccent = ImVec4(0.20f, 0.72f, 1.00f, 1.00f);
inline const ImVec4 kGood = ImVec4(0.25f, 0.85f, 0.55f, 1.00f);
inline const ImVec4 kWarn = ImVec4(1.00f, 0.72f, 0.25f, 1.00f);
inline const ImVec4 kMuted = ImVec4(0.58f, 0.64f, 0.72f, 1.00f);

inline void ApplyStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding = ImVec2(14.0f, 12.0f);
    s.FramePadding = ImVec2(8.0f, 6.0f);
    s.ItemSpacing = ImVec2(7.0f, 7.0f);
    s.WindowRounding = 9.0f;
    s.ChildRounding = 7.0f;
    s.FrameRounding = 5.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.035f, 0.055f, 0.080f, 0.98f);
    c[ImGuiCol_ChildBg] = ImVec4(0.055f, 0.080f, 0.110f, 0.92f);
    c[ImGuiCol_Border] = ImVec4(0.15f, 0.28f, 0.40f, 0.75f);
    c[ImGuiCol_Button] = ImVec4(0.08f, 0.18f, 0.27f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.34f, 0.48f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.44f, 0.58f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.08f, 0.24f, 0.35f, 1.00f);
    c[ImGuiCol_Tab] = ImVec4(0.05f, 0.13f, 0.20f, 1.00f);
    c[ImGuiCol_TabActive] = ImVec4(0.08f, 0.25f, 0.36f, 1.00f);
    c[ImGuiCol_CheckMark] = kAccent;
}

inline void Row(const char* label, const char* value, const ImVec4& valueColor = ImVec4(0.86f, 0.90f, 0.96f, 1.0f)) {
    ImGui::TextColored(kMuted, "%s", label);
    ImGui::SameLine(140.0f);
    ImGui::TextColored(valueColor, "%s", value ? value : "Unknown");
}

inline void CardTitle(const char* title) {
    ImGui::TextColored(kAccent, "%s", title);
    ImGui::Separator();
    ImGui::Spacing();
}

inline int g_SelectedCameraIndex = 0;

// 1. تبويب المميزات والـ ESP (Features)
inline void FeaturesPage() {
    ImGui::TextColored(kAccent, "NexDevTool - Features & ESP");
    ImGui::TextColored(kMuted, "Real-time hooked modules");
    ImGui::Separator();
    ImGui::Spacing();

    // قسم جلب الكاميرات الفعلي المرتبط بالمحرك
    ImGui::BeginChild("camera_section", ImVec2(0, 80), true);
    CardTitle("TARGET CAMERA");
    if (ImGui::Button("Scan Cameras", ImVec2(130, 28))) {
        NexDevEngine::ScanAllCameras();
    }
    ImGui::SameLine();

    std::vector<const char*> comboItems;
    for (const auto& cam : NexDevEngine::g_LoadedCameras) {
        comboItems.push_back(cam.name.c_str());
    }
    if (comboItems.empty()) {
        comboItems.push_back("No Camera Scanned");
    }

    ImGui::SetNextItemWidth(240.0f);
    ImGui::Combo("##CameraList", &g_SelectedCameraIndex, comboItems.data(), comboItems.size());
    ImGui::EndChild();

    ImGui::Spacing();

    // خيارات الـ ESP الحقيقية
    ImGui::BeginChild("esp_options", ImVec2(0, 160), true);
    CardTitle("ESP DRAWING OPTIONS");
    
    static bool enableEsp = false;
    static bool drawSnaplines = false;
    static bool drawBox2D = false;
    static bool drawBox3D = false;
    static bool showDistance = false;
    static bool showCount = false;
    static bool showName = false;
    static bool coverCheck = false;

    ImGui::Checkbox("Enable ESP", &enableEsp); ImGui::SameLine(160.0f);
    ImGui::Checkbox("Snaplines", &drawSnaplines); ImGui::SameLine(310.0f);
    ImGui::Checkbox("2D Box", &drawBox2D);

    ImGui::Checkbox("3D Box", &drawBox3D); ImGui::SameLine(160.0f);
    ImGui::Checkbox("Distance", &showDistance); ImGui::SameLine(310.0f);
    ImGui::Checkbox("Object Count", &showCount);

    ImGui::Checkbox("Object Name", &showName); ImGui::SameLine(160.0f);
    ImGui::Checkbox("Cover Check", &coverCheck);
    ImGui::EndChild();

    ImGui::Spacing();

    // إدارة الكائنات وتحديدها (مرتبطة بدوال المحرك الخلفي)
    ImGui::BeginChild("object_management", ImVec2(0, 100), true);
    CardTitle("OBJECT MANAGEMENT");
    if (ImGui::Button("Select All", ImVec2(120, 26))) {
        NexDevEngine::SetAllObjectsSelected(true);
    }
    ImGui::SameLine();
    if (ImGui::Button("Deselect All", ImVec2(120, 26))) {
        NexDevEngine::SetAllObjectsSelected(false);
    }
    ImGui::SameLine();
    if (ImGui::Button("Refresh Classes", ImVec2(120, 26))) {
        NexDevEngine::RefreshSceneObjects();
    }
    ImGui::EndChild();
}

// 2. تبويب الإعدادات (Settings)
inline void SettingsPage() {
    ImGui::TextColored(kAccent, "Settings & Game Info");
    ImGui::TextColored(kMuted, "Essential game and Unity details");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("settings_essential", ImVec2(0, 160), true);
    CardTitle("ESSENTIAL INFO");
    Row("Game Name", g_GameInfo.name.c_str());
    Row("Package", g_GameInfo.packageName.c_str());
    Row("Version", g_GameInfo.versionName.c_str());
    Row("Unity Version", g_UnityInfo.version.c_str(), kGood);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::BeginChild("settings_appearance", ImVec2(0, 110), true);
    CardTitle("APPEARANCE");
    ImGui::Checkbox("Show Watermark", &g_ShowWatermark);
    ImGui::SliderFloat("Window Opacity", &g_MenuAlpha, 0.70f, 1.0f, "%.2f");
    ImGui::EndChild();
}

inline void RenderWatermark() {
    if (!g_ShowWatermark) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12.0f, 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##watermark", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
    ImGui::TextColored(ImVec4(0.35f, 0.78f, 1.0f, 0.82f), "NEXDEVTOOL");
    ImGui::End();
}

} // namespace ToolUI

inline void RenderImGuiContent() {
    ToolUI::RenderWatermark();
    if (!g_ShowMenu) return;

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = g_FontScale;
    const float width = io.DisplaySize.x > 700.0f ? 780.0f : io.DisplaySize.x * 0.92f;
    const float height = io.DisplaySize.y > 700.0f ? 540.0f : io.DisplaySize.y * 0.78f;
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(g_MenuAlpha);
    
    ImGui::Begin("NexDevTool v1.0", &g_ShowMenu, ImGuiWindowFlags_NoCollapse);

    if (ImGui::BeginTabBar("##main_tabs")) {
        if (ImGui::BeginTabItem("Features")) { ToolUI::FeaturesPage(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Settings")) { ToolUI::SettingsPage(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
