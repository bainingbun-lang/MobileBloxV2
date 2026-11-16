#pragma once

#include <memory>
#include "editor/editor.hpp"
#include "explorer/explorer.hpp"
#include "ai/ai_assistant.hpp"

namespace mobileblox::ui {

class UIManager {
public:
    UIManager();
    ~UIManager();
    
    // Initialize ImGui context and backends
    bool Initialize(void* native_window, void* egl_display);
    void Shutdown();
    
    // Main render loop
    void NewFrame();
    void Render();
    void EndFrame();
    
    // Access to components
    mobileblox::editor::LuauEditor& GetEditor() { return *editor_; }
    mobileblox::explorer::Explorer& GetExplorer() { return *explorer_; }
    mobileblox::ai::AIAssistant& GetAIAssistant() { return *ai_assistant_; }
    
    // Window state
    void SetWindowSize(int width, int height) { window_width_ = width; window_height_ = height; }
    
    // Theme
    void ApplyTheme();
    
private:
    int window_width_ = 1280;
    int window_height_ = 720;
    
    std::unique_ptr<mobileblox::editor::LuauEditor> editor_;
    std::unique_ptr<mobileblox::explorer::Explorer> explorer_;
    std::unique_ptr<mobileblox::ai::AIAssistant> ai_assistant_;
    
    // ImGui context
    ImGuiContext* imgui_context_ = nullptr;
    
    // Docking layout
    bool first_frame_ = true;
    void SetupDockingLayout();
};

} // namespace mobileblox::ui