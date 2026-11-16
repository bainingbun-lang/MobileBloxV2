#pragma once

#include <imgui.h>

namespace mobileblox::ui {

// MobileBlox V2 Color Palette
// Background: Black #000000
// Panels: Dark Gray #1a1a1a
// Buttons: Black #0d0d0d
// Text: White #ffffff
// Accent: Electric Blue #00d4ff

inline constexpr ImVec4 COL_BACKGROUND      = ImVec4(0.00f, 0.00f, 0.00f, 1.00f); // #000000
inline constexpr ImVec4 COL_PANEL           = ImVec4(0.10f, 0.10f, 0.10f, 1.00f); // #1a1a1a
inline constexpr ImVec4 COL_PANEL_HOVER     = ImVec4(0.13f, 0.13f, 0.13f, 1.00f); // #212121
inline constexpr ImVec4 COL_PANEL_ACTIVE    = ImVec4(0.16f, 0.16f, 0.16f, 1.00f); // #292929
inline constexpr ImVec4 COL_BUTTON          = ImVec4(0.05f, 0.05f, 0.05f, 1.00f); // #0d0d0d
inline constexpr ImVec4 COL_BUTTON_HOVER    = ImVec4(0.08f, 0.08f, 0.08f, 1.00f); // #141414
inline constexpr ImVec4 COL_BUTTON_ACTIVE   = ImVec4(0.11f, 0.11f, 0.11f, 1.00f); // #1c1c1c
inline constexpr ImVec4 COL_TEXT            = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // #ffffff
inline constexpr ImVec4 COL_TEXT_DISABLED   = ImVec4(0.50f, 0.50f, 0.50f, 1.00f); // #808080
inline constexpr ImVec4 COL_ACCENT          = ImVec4(0.00f, 0.83f, 1.00f, 1.00f); // #00d4ff
inline constexpr ImVec4 COL_ACCENT_HOVER    = ImVec4(0.00f, 0.67f, 0.80f, 1.00f); // #00aacc
inline constexpr ImVec4 COL_ACCENT_ACTIVE   = ImVec4(0.00f, 0.50f, 0.60f, 1.00f); // #008099
inline constexpr ImVec4 COL_BORDER          = ImVec4(0.20f, 0.20f, 0.20f, 1.00f); // #333333
inline constexpr ImVec4 COL_SEPARATOR       = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
inline constexpr ImVec4 COL_ERROR           = ImVec4(1.00f, 0.27f, 0.27f, 1.00f); // #ff4444
inline constexpr ImVec4 COL_SUCCESS         = ImVec4(0.27f, 0.80f, 0.27f, 1.00f); // #44cc44
inline constexpr ImVec4 COL_WARNING         = ImVec4(1.00f, 0.67f, 0.00f, 1.00f); // #ffaa00

// Apply MobileBlox V2 theme to ImGui style
inline void ApplyTheme(ImGuiStyle& style = ImGui::GetStyle()) {
    ImVec4* colors = style.Colors;

    // Core
    colors[ImGuiCol_Text]                   = COL_TEXT;
    colors[ImGuiCol_TextDisabled]           = COL_TEXT_DISABLED;
    colors[ImGuiCol_WindowBg]               = COL_BACKGROUND;
    colors[ImGuiCol_ChildBg]                = COL_PANEL;
    colors[ImGuiCol_PopupBg]                = COL_PANEL;
    colors[ImGuiCol_Border]                 = COL_BORDER;
    colors[ImGuiCol_BorderShadow]           = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg]                = COL_BUTTON;
    colors[ImGuiCol_FrameBgHovered]         = COL_BUTTON_HOVER;
    colors[ImGuiCol_FrameBgActive]          = COL_BUTTON_ACTIVE;
    colors[ImGuiCol_TitleBg]                = COL_PANEL;
    colors[ImGuiCol_TitleBgActive]          = COL_PANEL_ACTIVE;
    colors[ImGuiCol_TitleBgCollapsed]       = COL_PANEL;
    colors[ImGuiCol_MenuBarBg]              = COL_PANEL;
    colors[ImGuiCol_ScrollbarBg]            = COL_BACKGROUND;
    colors[ImGuiCol_ScrollbarGrab]          = COL_ACCENT;
    colors[ImGuiCol_ScrollbarGrabHovered]   = COL_ACCENT_HOVER;
    colors[ImGuiCol_ScrollbarGrabActive]    = COL_ACCENT_ACTIVE;
    colors[ImGuiCol_CheckMark]              = COL_ACCENT;
    colors[ImGuiCol_SliderGrab]             = COL_ACCENT;
    colors[ImGuiCol_SliderGrabActive]       = COL_ACCENT_ACTIVE;
    colors[ImGuiCol_Button]                 = COL_BUTTON;
    colors[ImGuiCol_ButtonHovered]          = COL_BUTTON_HOVER;
    colors[ImGuiCol_ButtonActive]           = COL_BUTTON_ACTIVE;
    colors[ImGuiCol_Header]                 = COL_ACCENT;
    colors[ImGuiCol_HeaderHovered]          = COL_ACCENT_HOVER;
    colors[ImGuiCol_HeaderActive]           = COL_ACCENT_ACTIVE;
    colors[ImGuiCol_Separator]              = COL_SEPARATOR;
    colors[ImGuiCol_SeparatorHovered]       = COL_ACCENT;
    colors[ImGuiCol_SeparatorActive]        = COL_ACCENT;
    colors[ImGuiCol_ResizeGrip]             = COL_ACCENT;
    colors[ImGuiCol_ResizeGripHovered]      = COL_ACCENT_HOVER;
    colors[ImGuiCol_ResizeGripActive]       = COL_ACCENT_ACTIVE;
    colors[ImGuiCol_Tab]                    = COL_BUTTON;
    colors[ImGuiCol_TabHovered]             = COL_BUTTON_HOVER;
    colors[ImGuiCol_TabActive]              = COL_ACCENT;
    colors[ImGuiCol_TabUnfocused]           = COL_BUTTON;
    colors[ImGuiCol_TabUnfocusedActive]     = COL_BUTTON_HOVER;
    colors[ImGuiCol_DockingPreview]         = COL_ACCENT;
    colors[ImGuiCol_DockingEmptyBg]         = COL_BACKGROUND;
    colors[ImGuiCol_PlotLines]              = COL_ACCENT;
    colors[ImGuiCol_PlotLinesHovered]       = COL_ACCENT_HOVER;
    colors[ImGuiCol_PlotHistogram]          = COL_ACCENT;
    colors[ImGuiCol_PlotHistogramHovered]   = COL_ACCENT_HOVER;
    colors[ImGuiCol_TableHeaderBg]          = COL_PANEL;
    colors[ImGuiCol_TableBorderStrong]      = COL_BORDER;
    colors[ImGuiCol_TableBorderLight]       = COL_SEPARATOR;
    colors[ImGuiCol_TableRowBg]             = COL_BACKGROUND;
    colors[ImGuiCol_TableRowBgAlt]          = COL_PANEL;
    colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.00f, 0.83f, 1.00f, 0.35f);
    colors[ImGuiCol_DragDropTarget]         = COL_ACCENT;
    colors[ImGuiCol_NavHighlight]           = COL_ACCENT;
    colors[ImGuiCol_NavWindowingHighlight]  = COL_ACCENT;
    colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);
    colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);

    // Style
    style.WindowRounding        = 4.0f;
    style.ChildRounding         = 4.0f;
    style.FrameRounding         = 3.0f;
    style.PopupRounding         = 4.0f;
    style.ScrollbarRounding     = 3.0f;
    style.GrabRounding          = 3.0f;
    style.TabRounding           = 3.0f;
    style.WindowBorderSize      = 1.0f;
    style.FrameBorderSize       = 1.0f;
    style.PopupBorderSize       = 1.0f;
    style.TabBorderSize         = 1.0f;
    style.WindowPadding         = ImVec2(8, 8);
    style.FramePadding          = ImVec2(6, 4);
    style.ItemSpacing           = ImVec2(6, 4);
    style.ItemInnerSpacing      = ImVec2(4, 4);
    style.IndentSpacing         = 16.0f;
    style.ScrollbarSize         = 10.0f;
    style.GrabMinSize           = 12.0f;
    style.WindowTitleAlign      = ImVec2(0.0f, 0.5f);
    style.ButtonTextAlign       = ImVec2(0.5f, 0.5f);
    style.DisplayWindowPadding  = ImVec2(0, 0);
    style.DisplaySafeAreaPadding = ImVec2(4, 4);
}

// Render logo "_" in electric blue on black button
inline void RenderLogo(const char* id = "##logo", ImVec2 size = ImVec2(40, 32)) {
    ImGui::PushStyleColor(ImGuiCol_Button, COL_BUTTON);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_BUTTON);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, COL_BUTTON);
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0,0,0,0));
    
    if (ImGui::Button(id, size)) {
        // Logo click - could toggle something
    }
    
    // Draw "_" in electric blue centered
    ImVec2 pos = ImGui::GetItemRectMin();
    ImVec2 center = ImVec2(pos.x + size.x * 0.5f, pos.y + size.y * 0.5f);
    ImGui::GetWindowDrawList()->AddText(
        ImGui::GetFont(), 
        20.0f, 
        ImVec2(center.x - 4, center.y - 10), 
        ImColor(COL_ACCENT), 
        "_"
    );
    
    ImGui::PopStyleColor(4);
}

} // namespace mobileblox::ui