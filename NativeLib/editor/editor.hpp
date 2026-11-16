#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <imgui.h>

namespace mobileblox::editor {

// Forward declaration for TextEditor (using imgui-text-editor or custom)
struct TextEditor;

struct EditorFile {
    std::string path;
    std::string name;
    std::string content;
    bool modified = false;
    bool is_new = false;
    size_t cursor_line = 1;
    size_t cursor_col = 1;
};

struct EditorState {
    std::vector<std::unique_ptr<EditorFile>> files;
    int active_file_idx = -1;
    bool show_whitespace = false;
    bool show_line_numbers = true;
    bool auto_indent = true;
    int tab_size = 4;
    bool insert_spaces = true;
};

class LuauEditor {
public:
    LuauEditor();
    ~LuauEditor();

    // File operations
    bool NewFile(const std::string& path = "");
    bool OpenFile(const std::string& path);
    bool SaveFile(int index = -1);
    bool SaveFileAs(int index, const std::string& path);
    void CloseFile(int index);
    void CloseAllFiles();

    // Editor rendering
    void Render();
    void RenderFileTabs();
    void RenderEditorPane();
    void RenderStatusBar();

    // Content access
    std::string GetContent(int index = -1) const;
    void SetContent(int index, const std::string& content);
    bool IsModified(int index = -1) const;

    // Execution
    using ExecuteCallback = std::function<void(const std::string& script)>;
    void SetExecuteCallback(ExecuteCallback cb) { execute_callback_ = cb; }

    // State
    EditorState& GetState() { return state_; }
    const EditorState& GetState() const { return state_; }

    // Syntax highlighting
    void SetSyntaxHighlighting(bool enabled) { syntax_highlighting_ = enabled; }
    bool GetSyntaxHighlighting() const { return syntax_highlighting_; }

    // Diagnostics
    struct Diagnostic {
        enum class Severity { Error, Warning, Info, Hint };
        size_t line;
        size_t col;
        size_t end_line;
        size_t end_col;
        std::string message;
        Severity severity;
    };
    void SetDiagnostics(int file_idx, const std::vector<Diagnostic>& diagnostics);
    const std::vector<Diagnostic>& GetDiagnostics(int file_idx) const;

private:
    EditorState state_;
    ExecuteCallback execute_callback_;
    bool syntax_highlighting_ = true;
    std::unique_ptr<TextEditor> text_editor_;
    
    // Luau keywords for highlighting
    static const char* luau_keywords[];
    static const char* luau_types[];
    static const char* luau_globals[];
};

// Luau syntax highlighting helper
struct LuauSyntaxHighlighter {
    enum class TokenType {
        None,
        Keyword,
        Type,
        Global,
        String,
        Number,
        Comment,
        Operator,
        Identifier,
        Preprocessor
    };

    struct Token {
        TokenType type;
        size_t start;
        size_t length;
    };

    static std::vector<Token> Tokenize(const std::string& code);
    static ImVec4 GetColor(TokenType type);
};

} // namespace mobileblox::editor