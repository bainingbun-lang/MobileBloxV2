#include "ai/ai_assistant.hpp"
#include "ui/theme.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <sstream>
#include <iostream>

namespace mobileblox::ai {

using json = nlohmann::json;

// =============================================================================
// HTTP Adapter Base (for OpenAI-compatible APIs)
// =============================================================================
class HTTPAIAdapter : public AIAdapter {
public:
    bool Initialize(const AIConfig& config) override {
        config_ = config;
        state_ = ConnectionState::Connecting;
        
        // Test connection
        std::string error;
        if (TestConnection(error)) {
            state_ = ConnectionState::Connected;
            return true;
        } else {
            state_ = ConnectionState::Error;
            last_error_ = error;
            return false;
        }
    }
    
    void Shutdown() override {
        state_ = ConnectionState::Disconnected;
    }
    
    bool SendRequest(const AIRequest& request, AIResponse& response) override {
        json payload = BuildPayload(request);
        payload["stream"] = false;
        
        cpr::Response r = cpr::Post(
            cpr::Url{config_.endpoint_url},
            cpr::Header{
                {"Content-Type", "application/json"},
                {"Authorization", "Bearer " + config_.api_key}
            },
            cpr::Body{payload.dump()},
            cpr::Timeout{config_.timeout_ms}
        );
        
        return ParseResponse(r, response);
    }
    
    bool SendStreamingRequest(const AIRequest& request, StreamCallback callback) override {
        json payload = BuildPayload(request);
        payload["stream"] = true;
        
        // Use cpr with callback for streaming
        // Note: cpr doesn't have native streaming, we'll simulate with chunked response
        // For real streaming, we'd need a lower-level HTTP client
        // This is a simplified implementation
        
        cpr::Response r = cpr::Post(
            cpr::Url{config_.endpoint_url},
            cpr::Header{
                {"Content-Type", "application/json"},
                {"Authorization", "Bearer " + config_.api_key},
                {"Accept", "text/event-stream"}
            },
            cpr::Body{payload.dump()},
            cpr::Timeout{config_.timeout_ms}
        );
        
        // Parse SSE stream
        return ParseStreamingResponse(r.text, callback);
    }
    
    bool TestConnection(std::string& error_out) override {
        // Try a simple models list request or minimal chat
        AIRequest test_request;
        test_request.messages.push_back({ChatMessage::Role::User, "Hi"});
        test_request.model = config_.model;
        test_request.max_tokens = 5;
        
        AIResponse response;
        if (SendRequest(test_request, response)) {
            if (response.success) return true;
            error_out = response.error;
        } else {
            error_out = "Connection failed";
        }
        return false;
    }
    
    AIProvider GetProvider() const override { return config_.provider; }
    
    std::vector<std::string> GetAvailableModels() override {
        // Try to fetch models list
        std::vector<std::string> models;
        cpr::Response r = cpr::Get(
            cpr::Url{config_.endpoint_url.replace(config_.endpoint_url.find("/chat"), 5, "/models")},
            cpr::Header{{"Authorization", "Bearer " + config_.api_key}},
            cpr::Timeout{10000}
        );
        
        if (r.status_code == 200) {
            try {
                json j = json::parse(r.text);
                if (j.contains("data") && j["data"].is_array()) {
                    for (const auto& m : j["data"]) {
                        if (m.contains("id")) models.push_back(m["id"]);
                    }
                }
            } catch (...) {}
        }
        
        if (models.empty()) models = {"codellama", "llama2", "mistral", "gpt-3.5-turbo"};
        return models;
    }
    
    std::string GetDefaultEndpoint() const override {
        switch (config_.provider) {
            case AIProvider::Ollama: return "http://localhost:11434/api/chat";
            case AIProvider::OpenAI: return "https://api.openai.com/v1/chat/completions";
            case AIProvider::Anthropic: return "https://api.anthropic.com/v1/messages";
            case AIProvider::ClaudeCode: return "http://localhost:8080/api/chat"; // Example proxy
            default: return "http://localhost:11434/api/chat";
        }
    }
    
    void SetConfig(const AIConfig& config) override { config_ = config; }
    AIConfig GetConfig() const override { return config_; }

protected:
    virtual json BuildPayload(const AIRequest& request) {
        json messages_json = json::array();
        for (const auto& msg : request.messages) {
            json m;
            m["role"] = RoleToString(msg.role);
            m["content"] = msg.content;
            messages_json.push_back(m);
        }
        
        json payload;
        payload["model"] = request.model.empty() ? config_.model : request.model;
        payload["messages"] = messages_json;
        payload["temperature"] = request.temperature;
        payload["max_tokens"] = request.max_tokens;
        
        // Merge extra params
        for (auto& [key, val] : request.extra_params.items()) {
            payload[key] = val;
        }
        
        return payload;
    }
    
    virtual std::string RoleToString(ChatMessage::Role role) {
        switch (role) {
            case ChatMessage::Role::System: return "system";
            case ChatMessage::Role::User: return "user";
            case ChatMessage::Role::Assistant: return "assistant";
            case ChatMessage::Role::Tool: return "tool";
        }
        return "user";
    }
    
    virtual bool ParseResponse(const cpr::Response& r, AIResponse& response) {
        response.success = false;
        
        if (r.status_code >= 400) {
            response.error = "HTTP " + std::to_string(r.status_code) + ": " + r.text;
            return false;
        }
        
        try {
            json j = json::parse(r.text);
            
            // OpenAI format
            if (j.contains("choices") && j["choices"].is_array() && !j["choices"].empty()) {
                auto& choice = j["choices"][0];
                if (choice.contains("message") && choice["message"].contains("content")) {
                    response.content = choice["message"]["content"];
                    response.success = true;
                }
            }
            // Ollama format
            else if (j.contains("message") && j["message"].contains("content")) {
                response.content = j["message"]["content"];
                response.success = true;
            }
            // Anthropic format
            else if (j.contains("content") && j["content"].is_array()) {
                for (const auto& c : j["content"]) {
                    if (c.contains("text")) response.content += c["text"];
                }
                response.success = true;
            }
            
            if (j.contains("model")) response.model = j["model"];
            if (j.contains("usage")) {
                response.prompt_tokens = j["usage"].value("prompt_tokens", 0);
                response.completion_tokens = j["usage"].value("completion_tokens", 0);
            }
            
        } catch (const std::exception& e) {
            response.error = "Parse error: " + std::string(e.what());
            return false;
        }
        
        return response.success;
    }
    
    virtual bool ParseStreamingResponse(const std::string& text, StreamCallback callback) {
        // Parse Server-Sent Events (SSE)
        std::istringstream stream(text);
        std::string line;
        
        while (std::getline(stream, line)) {
            if (line.empty() || line[0] == ':') continue; // Comments
            
            if (line.rfind("data: ", 0) == 0) {
                std::string data = line.substr(6);
                if (data == "[DONE]") {
                    callback("", true);
                    return true;
                }
                
                try {
                    json j = json::parse(data);
                    std::string chunk;
                    
                    // OpenAI format
                    if (j.contains("choices") && !j["choices"].empty()) {
                        auto& choice = j["choices"][0];
                        if (choice.contains("delta") && choice["delta"].contains("content")) {
                            chunk = choice["delta"]["content"];
                        }
                        if (choice.contains("finish_reason") && !choice["finish_reason"].is_null()) {
                            callback(chunk, true);
                            return true;
                        }
                    }
                    // Ollama format
                    else if (j.contains("message") && j["message"].contains("content")) {
                        chunk = j["message"]["content"];
                        if (j.value("done", false)) {
                            callback(chunk, true);
                            return true;
                        }
                    }
                    
                    if (!chunk.empty()) {
                        callback(chunk, false);
                    }
                } catch (...) {}
            }
        }
        return true;
    }
};

// =============================================================================
// Ollama Adapter (extends HTTPAIAdapter with Ollama-specific endpoints)
// =============================================================================
class OllamaAdapter : public HTTPAIAdapter {
public:
    AIProvider GetProvider() const override { return AIProvider::Ollama; }
    std::string GetDefaultEndpoint() const override { return "http://localhost:11434/api/chat"; }
    
    std::vector<std::string> GetAvailableModels() override {
        std::vector<std::string> models;
        cpr::Response r = cpr::Get(
            cpr::Url{"http://localhost:11434/api/tags"},
            cpr::Timeout{10000}
        );
        
        if (r.status_code == 200) {
            try {
                json j = json::parse(r.text);
                if (j.contains("models") && j["models"].is_array()) {
                    for (const auto& m : j["models"]) {
                        if (m.contains("name")) models.push_back(m["name"]);
                    }
                }
            } catch (...) {}
        }
        
        if (models.empty()) models = {"codellama", "llama2", "mistral", "phi"};
        return models;
    }
    
protected:
    json BuildPayload(const AIRequest& request) override {
        json payload = HTTPAIAdapter::BuildPayload(request);
        // Ollama specific options
        payload["options"] = {
            {"temperature", request.temperature},
            {"num_predict", request.max_tokens}
        };
        return payload;
    }
};

// =============================================================================
// OpenAI Adapter
// =============================================================================
class OpenAIAdapter : public HTTPAIAdapter {
public:
    AIProvider GetProvider() const override { return AIProvider::OpenAI; }
    std::string GetDefaultEndpoint() const override { return "https://api.openai.com/v1/chat/completions"; }
};

// =============================================================================
// Anthropic Adapter (different API format)
// =============================================================================
class AnthropicAdapter : public HTTPAIAdapter {
public:
    AIProvider GetProvider() const override { return AIProvider::Anthropic; }
    std::string GetDefaultEndpoint() const override { return "https://api.anthropic.com/v1/messages"; }
    
protected:
    json BuildPayload(const AIRequest& request) override {
        json messages_json = json::array();
        std::string system_prompt;
        
        for (const auto& msg : request.messages) {
            if (msg.role == ChatMessage::Role::System) {
                system_prompt = msg.content;
            } else {
                json m;
                m["role"] = (msg.role == ChatMessage::Role::User) ? "user" : "assistant";
                m["content"] = msg.content;
                messages_json.push_back(m);
            }
        }
        
        json payload;
        payload["model"] = request.model.empty() ? config_.model : request.model;
        payload["messages"] = messages_json;
        payload["max_tokens"] = request.max_tokens;
        payload["temperature"] = request.temperature;
        
        if (!system_prompt.empty()) {
            payload["system"] = system_prompt;
        }
        
        return payload;
    }
    
    bool ParseResponse(const cpr::Response& r, AIResponse& response) override {
        response.success = false;
        
        if (r.status_code >= 400) {
            response.error = "HTTP " + std::to_string(r.status_code) + ": " + r.text;
            return false;
        }
        
        try {
            json j = json::parse(r.text);
            if (j.contains("content") && j["content"].is_array()) {
                for (const auto& c : j["content"]) {
                    if (c.contains("text")) response.content += c["text"];
                }
                response.success = true;
            }
            if (j.contains("model")) response.model = j["model"];
            if (j.contains("usage")) {
                response.prompt_tokens = j["usage"].value("input_tokens", 0);
                response.completion_tokens = j["usage"].value("output_tokens", 0);
            }
        } catch (const std::exception& e) {
            response.error = "Parse error: " + std::string(e.what());
            return false;
        }
        
        return response.success;
    }
};

// =============================================================================
// Factory Implementation
// =============================================================================
std::unique_ptr<AIAdapter> AIAdapterFactory::CreateAdapter(AIProvider provider) {
    switch (provider) {
        case AIProvider::Ollama: return std::make_unique<OllamaAdapter>();
        case AIProvider::OpenAI: return std::make_unique<OpenAIAdapter>();
        case AIProvider::Anthropic: return std::make_unique<AnthropicAdapter>();
        case AIProvider::ClaudeCode: return std::make_unique<HTTPAIAdapter>(); // Custom endpoint
        case AIProvider::Custom: return std::make_unique<HTTPAIAdapter>();
        default: return std::make_unique<HTTPAIAdapter>();
    }
}

std::vector<AIProvider> AIAdapterFactory::GetSupportedProviders() {
    return {AIProvider::Ollama, AIProvider::OpenAI, AIProvider::Anthropic, AIProvider::ClaudeCode, AIProvider::Custom};
}

std::string AIAdapterFactory::ProviderToString(AIProvider provider) {
    switch (provider) {
        case AIProvider::Ollama: return "Ollama (Local)";
        case AIProvider::OpenAI: return "OpenAI";
        case AIProvider::Anthropic: return "Anthropic (Claude)";
        case AIProvider::ClaudeCode: return "Claude Code Proxy";
        case AIProvider::Custom: return "Custom (OpenAI Compatible)";
        default: return "Unknown";
    }
}

AIProvider AIAdapterFactory::StringToProvider(const std::string& str) {
    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
    if (lower.find("ollama") != std::string::npos) return AIProvider::Ollama;
    if (lower.find("openai") != std::string::npos) return AIProvider::OpenAI;
    if (lower.find("anthropic") != std::string::npos || lower.find("claude") != std::string::npos) return AIProvider::Anthropic;
    if (lower.find("claudecode") != std::string::npos || lower.find("claude-code") != std::string::npos) return AIProvider::ClaudeCode;
    return AIProvider::Custom;
}

// =============================================================================
// AIAssistant Implementation
// =============================================================================
AIAssistant::AIAssistant() {
    LoadConfig();
}

AIAssistant::~AIAssistant() {
    if (adapter_) adapter_->Shutdown();
    SaveConfig();
}

bool AIAssistant::Configure(const AIConfig& config) {
    config_ = config;
    adapter_ = AIAdapterFactory::CreateAdapter(config.provider);
    
    if (!adapter_->Initialize(config)) {
        connection_state_ = ConnectionState::Error;
        last_error_ = adapter_->GetLastError();
        return false;
    }
    
    connection_state_ = ConnectionState::Connected;
    return true;
}

AIConfig AIAssistant::GetConfig() const {
    return config_;
}

bool AIAssistant::TestConnection(std::string& error_out) {
    if (!adapter_) return false;
    connection_state_ = ConnectionState::Connecting;
    bool result = adapter_->TestConnection(error_out);
    connection_state_ = result ? ConnectionState::Connected : ConnectionState::Error;
    last_error_ = error_out;
    return result;
}

void AIAssistant::SendMessage(const std::string& message) {
    if (message.empty() || !adapter_ || connection_state_ != ConnectionState::Connected) return;
    
    AddMessage(ChatMessage::Role::User, message);
    
    AIRequest request;
    request.messages = messages_;
    request.model = config_.model;
    request.temperature = config_.temperature;
    request.max_tokens = config_.max_tokens;
    request.stream = config_.streaming;
    
    if (config_.streaming) {
        streaming_ = true;
        streaming_buffer_.clear();
        
        adapter_->SendStreamingRequest(request, [this](const std::string& chunk, bool done) {
            if (!chunk.empty()) {
                streaming_buffer_ += chunk;
                // Update last assistant message
                if (!messages_.empty() && messages_.back().role == ChatMessage::Role::Assistant) {
                    messages_.back().content = streaming_buffer_;
                    messages_.back().is_streaming = !done;
                } else {
                    // Create new assistant message
                    ChatMessage msg;
                    msg.role = ChatMessage::Role::Assistant;
                    msg.content = streaming_buffer_;
                    msg.timestamp = std::chrono::system_clock::now();
                    msg.is_streaming = !done;
                    msg.model = config_.model;
                    messages_.push_back(msg);
                }
            }
            if (done) {
                streaming_ = false;
                streaming_buffer_.clear();
                if (!messages_.empty()) messages_.back().is_streaming = false;
            }
        });
    } else {
        AIResponse response;
        if (adapter_->SendRequest(request, response)) {
            if (response.success) {
                AddMessage(ChatMessage::Role::Assistant, response.content);
            } else {
                AddMessage(ChatMessage::Role::Assistant, "Error: " + response.error);
            }
        }
    }
}

void AIAssistant::SendMessageWithContext(const std::string& message, const std::string& file_context) {
    std::string full_message = message;
    if (!file_context.empty()) {
        full_message = "File context:\n```luau\n" + file_context + "\n```\n\n" + message;
    }
    SendMessage(full_message);
}

void AIAssistant::SendErrorForAnalysis(const std::string& error, const std::string& code_context) {
    std::string prompt = Prompts::ERROR_ANALYSIS;
    size_t pos = prompt.find("{error}");
    if (pos != std::string::npos) prompt.replace(pos, 7, error);
    pos = prompt.find("{code}");
    if (pos != std::string::npos) prompt.replace(pos, 6, code_context);
    
    SendMessage(prompt);
}

void AIAssistant::RequestExplanation(const std::string& code) {
    SendMessage(Prompts::CODE_EXPLANATION + "\n\n```luau\n" + code + "\n```");
}

void AIAssistant::RequestFix(const std::string& code, const std::string& error) {
    std::string prompt = Prompts::CODE_FIX;
    size_t pos = prompt.find("{error}");
    if (pos != std::string::npos) prompt.replace(pos, 7, error);
    
    SendMessage(prompt + "\n\n```luau\n" + code + "\n```");
}

void AIAssistant::AddMessage(ChatMessage::Role role, const std::string& content) {
    ChatMessage msg;
    msg.role = role;
    msg.content = content;
    msg.timestamp = std::chrono::system_clock::now();
    msg.model = config_.model;
    messages_.push_back(msg);
}

std::string AIAssistant::BuildSystemPrompt() const {
    std::string prompt = config_.system_prompt;
    if (file_context_.has_value()) {
        prompt += "\n\nCurrent file: " + file_context_->path + " (line " + std::to_string(file_context_->cursor_line) + ")";
    }
    return prompt;
}

std::vector<json> AIAssistant::BuildMessagesJSON() const {
    std::vector<json> result;
    json sys_msg;
    sys_msg["role"] = "system";
    sys_msg["content"] = BuildSystemPrompt();
    result.push_back(sys_msg);
    
    for (const auto& msg : messages_) {
        json m;
        m["role"] = (msg.role == ChatMessage::Role::System) ? "system" :
                    (msg.role == ChatMessage::Role::User) ? "user" : "assistant";
        m["content"] = msg.content;
        result.push_back(m);
    }
    return result;
}

void AIAssistant::SetCurrentFileContext(const std::string& file_path, const std::string& content, size_t cursor_line) {
    file_context_ = FileContext{file_path, content, cursor_line};
}

void AIAssistant::ClearFileContext() {
    file_context_.reset();
}

void AIAssistant::SaveConfig() {
    // TODO: Save to file
}

void AIAssistant::LoadConfig() {
    // TODO: Load from file
    // Set defaults
    config_.provider = AIProvider::Ollama;
    config_.endpoint_url = "http://localhost:11434/api/chat";
    config_.model = "codellama";
    config_.streaming = true;
}

void AIAssistant::Render() {
    if (!visible_) return;
    
    mobileblox::ui::ApplyTheme();
    
    if (!ImGui::Begin("AI Assistant", &visible_, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Session")) {
            if (ImGui::MenuItem("New Chat")) { messages_.clear(); }
            if (ImGui::MenuItem("Save Chat")) {}
            if (ImGui::MenuItem("Load Chat")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Clear History")) { messages_.clear(); }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Tools")) {
            if (ImGui::MenuItem("Configure")) { config_open_ = true; }
            if (ImGui::MenuItem("Test Connection")) {
                std::string error;
                TestConnection(error);
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // Connection status
    RenderConnectionStatus();
    ImGui::Separator();
    
    // Chat area
    ImGui::BeginChild("##chat_area", ImVec2(0, -100), false, ImGuiWindowFlags_AlwaysVerticalScrollbar);
    for (const auto& msg : messages_) {
        RenderMessage(msg);
    }
    if (streaming_) {
        // Scroll to bottom
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::EndChild();
    
    ImGui::Separator();
    
    // Input area
    static char input_buf[4096] = "";
    ImGui::PushItemWidth(-1);
    bool enter_pressed = ImGui::InputTextMultiline("##input", input_buf, sizeof(input_buf), ImVec2(0, 60), ImGuiInputTextFlags_CtrlEnterForNewLine);
    ImGui::PopItemWidth();
    
    ImGui::SameLine();
    ImGui::BeginGroup();
    if (ImGui::Button("Send", ImVec2(60, 60)) || (enter_pressed && !ImGui::GetIO().KeyShift)) {
        if (strlen(input_buf) > 0) {
            SendMessage(input_buf);
            input_buf[0] = '\0';
        }
    }
    ImGui::EndGroup();
    
    // Config panel
    if (config_open_) {
        ImGui::OpenPopup("AI Configuration");
        config_open_ = false;
    }
    if (ImGui::BeginPopupModal("AI Configuration", &config_open_, ImGuiWindowFlags_AlwaysAutoResize)) {
        RenderConfigPanel();
        ImGui::EndPopup();
    }
    
    ImGui::End();
}

void AIAssistant::RenderMessage(const ChatMessage& msg) {
    const char* role_name = "";
    ImVec4 role_color = mobileblox::ui::COL_TEXT;
    ImVec4 bg_color = mobileblox::ui::COL_PANEL;
    
    switch (msg.role) {
        case ChatMessage::Role::User:
            role_name = "You";
            role_color = mobileblox::ui::COL_ACCENT;
            bg_color = ImVec4(0.05f, 0.15f, 0.25f, 1.0f);
            break;
        case ChatMessage::Role::Assistant:
            role_name = "Assistant";
            role_color = mobileblox::ui::COL_SUCCESS;
            bg_color = ImVec4(0.05f, 0.2f, 0.1f, 1.0f);
            break;
        case ChatMessage::Role::System:
            role_name = "System";
            role_color = mobileblox::ui::COL_WARNING;
            bg_color = ImVec4(0.2f, 0.15f, 0.05f, 1.0f);
            break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_ChildBg, bg_color);
    ImGui::BeginChild(("##msg_" + std::to_string(reinterpret_cast<uintptr_t>(&msg))).c_str(), ImVec2(0, 0), true);
    
    // Header
    ImGui::PushStyleColor(ImGuiCol_Text, role_color);
    ImGui::Text("%s", role_name);
    ImGui::PopStyleColor();
    ImGui::SameLine();
    
    // Timestamp
    auto time_t = std::chrono::system_clock::to_time_t(msg.timestamp);
    char time_buf[32];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", std::localtime(&time_t));
    ImGui::TextDisabled("%s", time_buf);
    
    if (msg.is_streaming) {
        ImGui::SameLine();
        ImGui::TextColored(mobileblox::ui::COL_ACCENT, "● Streaming");
    }
    
    ImGui::Separator();
    
    // Content
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 80.0f);
    ImGui::TextWrapped("%s", msg.content.c_str());
    ImGui::PopTextWrapPos();
    
    // Actions
    if (msg.role == ChatMessage::Role::Assistant) {
        ImGui::Separator();
        if (ImGui::SmallButton("Copy")) {
            ImGui::SetClipboardText(msg.content.c_str());
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Regenerate")) {
            // TODO: Regenerate last response
        }
    }
    
    ImGui::EndChild();
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

void AIAssistant::RenderConfigPanel() {
    ImGui::Text("Provider");
    ImGui::SameLine();
    static int provider_idx = static_cast<int>(config_.provider);
    const char* providers[] = {"Custom", "Ollama", "OpenAI", "Anthropic", "Claude Code"};
    if (ImGui::Combo("##provider", &provider_idx, providers, IM_ARRAYSIZE(providers))) {
        config_.provider = static_cast<AIProvider>(provider_idx);
        config_.endpoint_url = AIAdapterFactory::CreateAdapter(config_.provider)->GetDefaultEndpoint();
    }
    
    ImGui::InputText("Endpoint URL", &config_.endpoint_url);
    ImGui::InputText("Model", &config_.model);
    ImGui::InputText("API Key", &config_.api_key, ImGuiInputTextFlags_Password);
    ImGui::InputInt("Timeout (ms)", &config_.timeout_ms);
    ImGui::Checkbox("Streaming", &config_.streaming);
    ImGui::SliderDouble("Temperature", &config_.temperature, 0.0, 2.0);
    ImGui::InputInt("Max Tokens", &config_.max_tokens);
    ImGui::InputTextMultiline("System Prompt", &config_.system_prompt, ImVec2(0, 100));
    
    ImGui::Separator();
    
    if (ImGui::Button("Test Connection")) {
        std::string error;
        TestConnection(error);
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
        Configure(config_);
        config_open_ = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
        config_open_ = false;
    }
}

void AIAssistant::RenderConnectionStatus() {
    const char* status_text = "";
    ImVec4 status_color = mobileblox::ui::COL_TEXT_DISABLED;
    
    switch (connection_state_) {
        case ConnectionState::Disconnected:
            status_text = "Disconnected";
            status_color = mobileblox::ui::COL_TEXT_DISABLED;
            break;
        case ConnectionState::Connecting:
            status_text = "Connecting...";
            status_color = mobileblox::ui::COL_WARNING;
            break;
        case ConnectionState::Connected:
            status_text = "Connected";
            status_color = mobileblox::ui::COL_SUCCESS;
            break;
        case ConnectionState::Error:
            status_text = "Error";
            status_color = mobileblox::ui::COL_ERROR;
            break;
    }
    
    ImGui::Text("Status: ");
    ImGui::SameLine();
    ImGui::TextColored(status_color, "%s", status_text);
    
    if (connection_state_ == ConnectionState::Error && !last_error_.empty()) {
        ImGui::SameLine();
        ImGui::TextColored(mobileblox::ui::COL_ERROR, "(%s)", last_error_.c_str());
    }
    
    ImGui::SameLine(ImGui::GetWindowWidth() - 200);
    if (ImGui::SmallButton("Configure")) config_open_ = true;
    ImGui::SameLine();
    if (ImGui::SmallButton(connection_state_ == ConnectionState::Connected ? "Disconnect" : "Connect")) {
        if (connection_state_ == ConnectionState::Connected) {
            adapter_->Shutdown();
            connection_state_ = ConnectionState::Disconnected;
        } else {
            std::string error;
            TestConnection(error);
        }
    }
}

} // namespace mobileblox::ai