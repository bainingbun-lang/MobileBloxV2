#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <chrono>
#include <imgui.h>
#include <nlohmann/json.hpp>

namespace mobileblox::ai {

using json = nlohmann::json;

enum class AIProvider {
    Custom,
    Ollama,
    OpenAI,
    Anthropic,
    ClaudeCode
};

enum class ConnectionState {
    Disconnected,
    Connecting,
    Connected,
    Error
};

struct AIConfig {
    AIProvider provider = AIProvider::Custom;
    std::string endpoint_url = "http://localhost:11434/api/chat"; // Ollama default
    std::string model = "codellama";
    std::string api_key;
    int timeout_ms = 30000;
    bool streaming = true;
    double temperature = 0.7;
    int max_tokens = 4096;
    std::string system_prompt = "You are a helpful coding assistant for Luau/Roblox development.";
};

struct ChatMessage {
    enum class Role { System, User, Assistant, Tool };
    Role role;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    bool is_streaming = false;
    std::string model; // Which model generated this
};

struct AIRequest {
    std::vector<ChatMessage> messages;
    std::string model;
    double temperature = 0.7;
    int max_tokens = 4096;
    bool stream = true;
    json extra_params;
};

struct AIResponse {
    bool success = false;
    std::string content;
    std::string error;
    std::string model;
    int prompt_tokens = 0;
    int completion_tokens = 0;
    bool done = true;
};

// Base adapter interface
class AIAdapter {
public:
    virtual ~AIAdapter() = default;
    
    virtual bool Initialize(const AIConfig& config) = 0;
    virtual void Shutdown() = 0;
    
    // Send request (blocking for non-streaming, or starts streaming)
    virtual bool SendRequest(const AIRequest& request, AIResponse& response) = 0;
    
    // Streaming: call callback for each chunk
    using StreamCallback = std::function<void(const std::string& chunk, bool done)>;
    virtual bool SendStreamingRequest(const AIRequest& request, StreamCallback callback) = 0;
    
    // Test connection
    virtual bool TestConnection(std::string& error_out) = 0;
    
    // Get provider info
    virtual AIProvider GetProvider() const = 0;
    virtual std::vector<std::string> GetAvailableModels() = 0;
    virtual std::string GetDefaultEndpoint() const = 0;
    
    // Configuration
    virtual void SetConfig(const AIConfig& config) = 0;
    virtual AIConfig GetConfig() const = 0;
    
    // Connection state
    ConnectionState GetState() const { return state_; }
    const std::string& GetLastError() const { return last_error_; }

protected:
    ConnectionState state_ = ConnectionState::Disconnected;
    std::string last_error_;
    AIConfig config_;
};

// Factory for creating adapters
class AIAdapterFactory {
public:
    static std::unique_ptr<AIAdapter> CreateAdapter(AIProvider provider);
    static std::vector<AIProvider> GetSupportedProviders();
    static std::string ProviderToString(AIProvider provider);
    static AIProvider StringToProvider(const std::string& str);
};

// Main AI Assistant class
class AIAssistant {
public:
    AIAssistant();
    ~AIAssistant();
    
    // Configuration
    bool Configure(const AIConfig& config);
    AIConfig GetConfig() const;
    bool TestConnection(std::string& error_out);
    
    // Chat
    void SendMessage(const std::string& message);
    void SendMessageWithContext(const std::string& message, const std::string& file_context);
    void SendErrorForAnalysis(const std::string& error, const std::string& code_context);
    void RequestExplanation(const std::string& code);
    void RequestFix(const std::string& code, const std::string& error);
    
    // Rendering
    void Render();
    void RenderConfigPanel();
    void RenderChatPanel();
    void RenderConnectionStatus();
    
    // State
    const std::vector<ChatMessage>& GetMessages() const { return messages_; }
    ConnectionState GetConnectionState() const { return connection_state_; }
    void SetVisible(bool visible) { visible_ = visible; }
    bool IsVisible() const { return visible_; }
    void ToggleVisible() { visible_ = !visible_; }
    
    // Callbacks
    using MessageCallback = std::function<void(const std::string& message)>;
    void SetMessageCallback(MessageCallback cb) { message_cb_ = cb; }
    
    // File context
    void SetCurrentFileContext(const std::string& file_path, const std::string& content, size_t cursor_line);
    void ClearFileContext();

private:
    AIConfig config_;
    std::unique_ptr<AIAdapter> adapter_;
    std::vector<ChatMessage> messages_;
    ConnectionState connection_state_ = ConnectionState::Disconnected;
    std::string last_error_;
    bool visible_ = true;
    bool config_open_ = false;
    bool streaming_ = false;
    std::string streaming_buffer_;
    
    // File context
    struct FileContext {
        std::string path;
        std::string content;
        size_t cursor_line = 0;
        size_t selection_start = 0;
        size_t selection_end = 0;
    };
    std::optional<FileContext> file_context_;
    
    MessageCallback message_cb_;
    
    // Internal
    void AddMessage(ChatMessage::Role role, const std::string& content);
    void ProcessStreamingResponse(const std::string& chunk, bool done);
    void HandleStreamingComplete();
    std::string BuildSystemPrompt() const;
    std::vector<json> BuildMessagesJSON() const;
    void SaveConfig();
    void LoadConfig();
};

// Predefined prompts for common tasks
namespace Prompts {
    inline const std::string CODE_EXPLANATION = 
        "Explain the following Luau code in detail. Focus on what it does, "
        "how it works, and any Roblox-specific patterns used.";
    
    inline const std::string CODE_FIX = 
        "Fix the following Luau code. The error is: {error}\n\n"
        "Provide only the corrected code with minimal changes.";
    
    inline const std::string CODE_REVIEW = 
        "Review the following Luau code for best practices, performance, "
        "and potential bugs. Suggest improvements.";
    
    inline const std::string ERROR_ANALYSIS = 
        "Analyze this compiler/runtime error and explain what caused it "
        "and how to fix it. Error: {error}\n\nCode context:\n{code}";
    
    inline const std::string OPTIMIZATION = 
        "Optimize the following Luau code for performance. "
        "Focus on Roblox Luau specific optimizations.";
}

} // namespace mobileblox::ai