#include "ui/ui_manager.hpp"
#include "ui/theme.hpp"
#include <imgui.h>
#include <imgui_impl_android.h>
#include <imgui_impl_opengl3.h>
#include <android/log.h>

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "MobileBloxUI", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "MobileBloxUI", __VA_ARGS__)

namespace mobileblox::ui {

UIManager::UIManager() {
    editor_ = std::make_unique<mobileblox::editor::LuauEditor>();
    explorer_ = std::make_unique<mobileblox::explorer::Explorer>();
    ai_assistant_ = std::make_unique<mobileblox::ai::AIAssistant>();
}

UIManager::~UIManager() {
    Shutdown();
}

bool UIManager::Initialize(void* native_window, void* egl_display) {
    // Create ImGui context
    imgui_context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(imgui_context_);
    
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
    io.IniFilename = "imgui.ini";
    
    // Load default font (monospace for editor)
    // TODO: Load custom font from assets
    io.Fonts->AddFontDefault();
    
    // Apply theme
    ApplyTheme();
    
    // Initialize backends
    // Note: On Android, we use the native activity window
    if (!ImGui_ImplAndroid_Init(native_window)) {
        LOGE("Failed to initialize ImGui Android backend");
        return false;
    }
    
    // OpenGL ES 3 backend
    const char* glsl_version = "#version 300 es";
    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        LOGE("Failed to initialize ImGui OpenGL3 backend");
        return false;
    }
    
    // Initialize components
    // Set up project root for explorer
    explorer_->SetProjectRoot("/sdcard/MobileBloxProjects");
    
    // Set up callbacks
    editor_->SetExecuteCallback([this](const std::string& script) {
        // TODO: Send to native execution queue
        LOGI("Execute script: %s", script.substr(0, 100).c_str());
    });
    
    explorer_->SetItemDoubleClickedCallback([this](mobileblox::explorer::ExplorerItem* item) {
        if (!item->is_folder) {
            // Open in editor
            editor_->OpenFile(item->path.string());
        }
    });
    
    ai_assistant_->SetMessageCallback([this](const std::string& message) {
        LOGI("AI Message: %s", message.c_str());
    });
    
    LOGI("UIManager initialized successfully");
    return true;
}

void UIManager::Shutdown() {
    if (imgui_context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
    }
}

void UIManager::NewFrame() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    
    if (first_frame_) {
        SetupDockingLayout();
        first_frame_ = false;
    }
}

void UIManager::Render() {
    // Main menu bar
    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New File", "Ctrl+N")) editor_->NewFile();
            if (ImGui::MenuItem("Open File...", "Ctrl+O")) { /* TODO */ }
            if (ImGui::MenuItem("Save", "Ctrl+S")) editor_->SaveFile();
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Open Project...")) { /* TODO */ }
            if (ImGui::MenuItem("Save Project")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit")) { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) { /* TODO */ }
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "Ctrl+X")) { /* TODO */ }
            if (ImGui::MenuItem("Copy", "Ctrl+C")) { /* TODO */ }
            if (ImGui::MenuItem("Paste", "Ctrl+V")) { /* TODO */ }
            ImGui::Separator();
            if (ImGui::MenuItem("Find", "Ctrl+F")) { /* TODO */ }
            if (ImGui::MenuItem("Replace", "Ctrl+H")) { /* TODO */ }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Explorer", nullptr, explorer_->GetState().root_item != nullptr);
            ImGui::MenuItem("AI Assistant", nullptr, ai_assistant_->IsVisible());
            ImGui::MenuItem("Console", nullptr, false); // TODO
            ImGui::MenuItem("Output", nullptr, false); // TODO
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("AI Configuration")) ai_assistant_->GetConfig();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Help")) {
            if (ImGui::MenuItem("About")) { /* TODO */ }
            ImGui::EndMenu();
        }
        
        // Logo on right side
        ImGui::SameLine(ImGui::GetWindowWidth() - 60);
        mobileblox::ui::RenderLogo();
        
        ImGui::EndMainMenuBar();
    }
    
    // Docking space
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->Pos);
    ImGui::SetNextWindowSize(viewport->Size);
    ImGui::SetNextWindowViewport(viewport->ID);
    
    ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
    window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
    
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    
    ImGui::Begin("##dockspace", nullptr, window_flags);
    
    ImGuiID dockspace_id = ImGui::GetID("MobileBloxDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_PassthruCentralNode);
    
    ImGui::End();
    ImGui::PopStyleVar(3);
    
    // Render components
    editor_->Render();
    explorer_->Render();
    ai_assistant_->Render();
    
    // Console window (placeholder)
    if (ImGui::Begin("Console")) {
        ImGui::Text("Console output will appear here...");
        ImGui::Separator();
        ImGui::TextColored(mobileblox::ui::COL_ACCENT, "> Ready");
    }
    ImGui::End();
}

void UIManager::EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    // Handle multi-viewport
    ImGuiIO& io = ImGui::GetIO();
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }
}

void UIManager::ApplyTheme() {
    mobileblox::ui::ApplyTheme();
}

void UIManager::SetupDockingLayout() {
    ImGuiID dockspace_id = ImGui::GetID("MobileBloxDockspace");
    
    // Clear existing layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(window_width_, window_height_));
    
    // Split layout: Left (Explorer), Center (Editor), Right (AI), Bottom (Console)
    ImGuiID dock_main = dockspace_id;
    ImGuiID dock_left = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.25f, nullptr, &dock_main);
    ImGuiID dock_right = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.30f, nullptr, &dock_main);
    ImGuiID dock_bottom = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.20f, nullptr, &dock_main);
    
    // Dock windows
    ImGui::DockBuilderDockWindow("Explorer", dock_left);
    ImGui::DockBuilderDockWindow("AI Assistant", dock_right);
    ImGui::DockBuilderDockWindow("Console", dock_bottom);
    ImGui::DockBuilderDockWindow("Editor", dock_main); // Central node
    
    ImGui::DockBuilderFinish(dockspace_id);
}

} // namespace mobileblox::ui