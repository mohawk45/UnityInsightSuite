#pragma once

#include "imgui.h"
#include "Globals.h"
#include "UnityDetector.h"

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
    s.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
    s.WindowRounding = 9.0f;
    s.ChildRounding = 7.0f;
    s.FrameRounding = 5.0f;
    s.PopupRounding = 6.0f;
    s.ScrollbarRounding = 6.0f;
    s.GrabRounding = 5.0f;
    s.TabRounding = 5.0f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg] = ImVec4(0.035f, 0.055f, 0.080f, 0.98f);
    c[ImGuiCol_ChildBg] = ImVec4(0.055f, 0.080f, 0.110f, 0.92f);
    c[ImGuiCol_PopupBg] = ImVec4(0.045f, 0.065f, 0.095f, 0.99f);
    c[ImGuiCol_Border] = ImVec4(0.15f, 0.28f, 0.40f, 0.75f);
    c[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.13f, 0.18f, 1.00f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(0.12f, 0.22f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.30f, 0.40f, 1.00f);
    c[ImGuiCol_TitleBg] = ImVec4(0.025f, 0.045f, 0.070f, 1.00f);
    c[ImGuiCol_TitleBgActive] = ImVec4(0.04f, 0.10f, 0.16f, 1.00f);
    c[ImGuiCol_Button] = ImVec4(0.08f, 0.18f, 0.27f, 1.00f);
    c[ImGuiCol_ButtonHovered] = ImVec4(0.12f, 0.34f, 0.48f, 1.00f);
    c[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.44f, 0.58f, 1.00f);
    c[ImGuiCol_Header] = ImVec4(0.08f, 0.24f, 0.35f, 1.00f);
    c[ImGuiCol_HeaderHovered] = ImVec4(0.11f, 0.34f, 0.46f, 1.00f);
    c[ImGuiCol_Tab] = ImVec4(0.05f, 0.13f, 0.20f, 1.00f);
    c[ImGuiCol_TabHovered] = ImVec4(0.12f, 0.34f, 0.46f, 1.00f);
    c[ImGuiCol_TabActive] = ImVec4(0.08f, 0.25f, 0.36f, 1.00f);
    c[ImGuiCol_CheckMark] = kAccent;
    c[ImGuiCol_SliderGrab] = kAccent;
    c[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.85f, 1.00f, 1.00f);
}

inline const char* RuntimeLabel() {
    switch (g_UnityInfo.state) {
        case UnityTool::RuntimeState::Scanning: return "Scanning";
        case UnityTool::RuntimeState::Detected: return "Detected";
        case UnityTool::RuntimeState::NotDetected: return "Not detected";
        case UnityTool::RuntimeState::Unsupported: return "Unsupported";
        default: return "Waiting";
    }
}

inline ImVec4 RuntimeColor() {
    return g_UnityInfo.state == UnityTool::RuntimeState::Detected ? kGood : kWarn;
}

inline void Row(const char* label, const char* value, const ImVec4& valueColor = ImVec4(0.86f, 0.90f, 0.96f, 1.0f)) {
    ImGui::TextColored(kMuted, "%s", label);
    ImGui::SameLine(165.0f);
    ImGui::TextColored(valueColor, "%s", value ? value : "Unknown");
}

inline void CardTitle(const char* title) {
    ImGui::TextColored(kAccent, "%s", title);
    ImGui::Separator();
    ImGui::Spacing();
}

inline void DashboardPage() {
    ImGui::TextColored(kAccent, "Overview");
    ImGui::SameLine();
    ImGui::TextColored(kMuted, "Unity diagnostic workspace");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("status_card", ImVec2(0, 108), true);
    CardTitle("RUNTIME STATUS");
    Row("Engine", g_UnityInfo.runtime.c_str(), RuntimeColor());
    Row("Status", RuntimeLabel(), RuntimeColor());
    Row("Version", g_UnityInfo.version.c_str(), (g_UnityInfo.confidence == "Verified" || g_UnityInfo.confidence == "High") ? kGood : kWarn);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::BeginChild("quick_card", ImVec2(0, 132), true);
    CardTitle("QUICK INFORMATION");
    Row("Backend", g_UnityInfo.backend.c_str());
    Row("Architecture", g_UnityInfo.architecture.c_str());
    Row("Graphics", g_UnityInfo.graphicsApi.c_str());
    Row("Profile", g_UnityInfo.profile.c_str(), kMuted);
    ImGui::EndChild();

    ImGui::Spacing();
    if (ImGui::Button("Scan runtime", ImVec2(150, 34))) {
        UnityDetector::ScanLoadedRuntime(nullptr, nullptr, true);
    }
    ImGui::SameLine();
    ImGui::TextColored(kMuted, "No guessed version or profile is enabled.");
}

inline void DebugOverlayPage() {
    ImGui::TextColored(kAccent, "Debug Overlay");
    ImGui::TextColored(kMuted, "Visualization controls are prepared for the next phase.");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginChild("overlay_plan", ImVec2(0, 210), true);
    CardTitle("PLANNED MODULES");
    ImGui::BulletText("Camera and frustum visualization");
    ImGui::BulletText("Scene object names and bounds");
    ImGui::BulletText("Collider and physics debug shapes");
    ImGui::BulletText("Distance and layer filters");
    ImGui::BulletText("Performance overlay");
    ImGui::Spacing();
    ImGui::TextColored(kWarn, "Waiting for verified Unity data provider");
    ImGui::EndChild();
}

inline void PerformancePage() {
    ImGui::TextColored(kAccent, "Performance");
    ImGui::TextColored(kMuted, "Telemetry will be connected after the runtime foundation.");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::BeginChild("performance_card", ImVec2(0, 170), true);
    CardTitle("SESSION METRICS");
    Row("FPS", "--");
    Row("Frame time", "-- ms");
    Row("Memory", "-- MB");
    Row("Objects", "--");
    Row("Draw calls", "--");
    ImGui::EndChild();
}

inline void ConsolePage() {
    ImGui::TextColored(kAccent, "Console");
    ImGui::SameLine(ImGui::GetContentRegionAvail().x - 78.0f);
    ImGui::Button("Clear", ImVec2(68, 28));
    ImGui::Separator();
    ImGui::BeginChild("console_output", ImVec2(0, 0), true);
    ImGui::TextColored(kGood, "[INFO] Phase 1 UI initialized");
    ImGui::TextColored(kGood, "[INFO] Passive Unity runtime scan ready");
    ImGui::TextColored(kMuted, "[WAIT] Unity version source is not verified");
    ImGui::TextColored(kMuted, "[WAIT] Profile selection is disabled safely");
    ImGui::EndChild();
}

inline void SettingsPage() {
    ImGui::TextColored(kAccent, "Settings");
    ImGui::TextColored(kMuted, "Game, Unity, device and appearance information");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::BeginChild("settings_game", ImVec2(0, 230), true);
    CardTitle("GAME INFORMATION");
    Row("Game name", g_GameInfo.name.c_str());
    Row("Package", g_GameInfo.packageName.c_str());
    Row("Version name", g_GameInfo.versionName.c_str());
    Row("Version code", g_GameInfo.versionCode.c_str());
    Row("Activity", g_GameInfo.activityClass.c_str());
    Row("APK", g_GameInfo.apkPath.c_str());
    Row("Split APK", g_GameInfo.splitApkPaths.c_str());
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::BeginChild("settings_unity", ImVec2(0, 198), true);
    CardTitle("UNITY INFORMATION");
    Row("Runtime", g_UnityInfo.runtime.c_str(), RuntimeColor());
    Row("Unity version", g_UnityInfo.version.c_str(), (g_UnityInfo.confidence == "Verified" || g_UnityInfo.confidence == "High") ? kGood : kWarn);
    Row("Backend", g_UnityInfo.backend.c_str());
    Row("Module", g_UnityInfo.module.c_str());
    Row("Architecture", g_UnityInfo.architecture.c_str());
    Row("Detection source", g_UnityInfo.source.c_str());
    Row("Source package", g_UnityInfo.sourcePackage.c_str());
    Row("Confidence", g_UnityInfo.confidence.c_str());
    Row("Profile", g_UnityInfo.profile.c_str(), kMuted);
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::BeginChild("settings_device", ImVec2(0, 138), true);
    CardTitle("DEVICE INFORMATION");
    Row("Android", g_DeviceInfo.androidVersion.c_str());
    Row("Model", g_DeviceInfo.model.c_str());
    Row("ABI", g_DeviceInfo.abi.c_str());
    Row("Resolution", g_DeviceInfo.resolution.c_str());
    ImGui::EndChild();

    ImGui::Spacing();
    ImGui::BeginChild("settings_appearance", ImVec2(0, 122), true);
    CardTitle("APPEARANCE");
    ImGui::Checkbox("Compact layout", &g_CompactMode);
    ImGui::Checkbox("Show watermark", &g_ShowWatermark);
    ImGui::SliderFloat("Window opacity", &g_MenuAlpha, 0.70f, 1.0f, "%.2f");
    ImGui::SliderFloat("Font scale", &g_FontScale, 0.85f, 1.20f, "%.2f");
    ImGui::EndChild();
}

inline void RenderWatermark() {
    if (!g_ShowWatermark) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x - 12.0f, 12.0f), ImGuiCond_Always, ImVec2(1.0f, 0.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);
    ImGui::Begin("##watermark", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBackground);
    ImGui::TextColored(ImVec4(0.35f, 0.78f, 1.0f, 0.82f), "UNITY TOOL");
    ImGui::End();
}

} // namespace ToolUI

inline void RenderImGuiContent() {
    ToolUI::RenderWatermark();
    if (!g_ShowMenu) return;

    ImGuiIO& io = ImGui::GetIO();
    io.FontGlobalScale = g_FontScale;
    const float width = io.DisplaySize.x > 700.0f ? 760.0f : io.DisplaySize.x * 0.92f;
    const float height = io.DisplaySize.y > 700.0f ? 520.0f : io.DisplaySize.y * 0.78f;
    ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowBgAlpha(g_MenuAlpha);
    ImGui::Begin("Unity Insight Suite", &g_ShowMenu, ImGuiWindowFlags_NoCollapse);

    ImGui::TextColored(ToolUI::kAccent, "UNITY INSIGHT SUITE");
    ImGui::SameLine();
    ImGui::TextColored(ToolUI::kMuted, "  Phase 1");
    ImGui::Separator();

    if (ImGui::BeginTabBar("##main_tabs")) {
        if (ImGui::BeginTabItem("Dashboard")) { ToolUI::DashboardPage(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Debug Overlay")) { ToolUI::DebugOverlayPage(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Performance")) { ToolUI::PerformancePage(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Console")) { ToolUI::ConsolePage(); ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("Settings")) { ToolUI::SettingsPage(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }
    ImGui::End();
}
