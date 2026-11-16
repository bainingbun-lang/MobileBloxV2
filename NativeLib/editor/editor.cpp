#include "editor/editor.hpp"
#include "ui/theme.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace mobileblox::editor {

// Simple TextEditor implementation using ImGui
struct TextEditor {
    std::string text;
    std::vector<std::string> lines;
    size_t cursor_line = 0;
    size_t cursor_col = 0;
    size_t selection_start_line = 0;
    size_t selection_start_col = 0;
    size_t selection_end_line = 0;
    size_t selection_end_col = 0;
    bool has_selection = false;
    ImVec2 scroll = ImVec2(0, 0);
    float line_height = 0;
    float char_width = 0;
    int tab_size = 4;
    bool insert_spaces = true;
    bool show_line_numbers = true;
    std::vector<Diagnostic> diagnostics;
    
    void UpdateLines() {
        lines.clear();
        std::istringstream iss(text);
        std::string line;
        while (std::getline(iss, line)) {
            lines.push_back(line);
        }
        if (lines.empty()) lines.push_back("");
    }
    
    void EnsureCursorValid() {
        if (cursor_line >= lines.size()) cursor_line = lines.size() - 1;
        if (cursor_col > lines[cursor_line].size()) cursor_col = lines[cursor_line].size();
    }
};

LuauEditor::LuauEditor() {
    text_editor_ = std::make_unique<TextEditor>();
    text_editor_->tab_size = state_.tab_size;
    text_editor_->insert_spaces = state_.insert_spaces;
    text_editor_->show_line_numbers = state_.show_line_numbers;
}

LuauEditor::~LuauEditor() = default;

bool LuauEditor::NewFile(const std::string& path) {
    auto file = std::make_unique<EditorFile>();
    if (path.empty()) {
        file->name = "untitled.lua";
        file->is_new = true;
    } else {
        file->path = path;
        size_t pos = path.find_last_of('/');
        file->name = (pos == std::string::npos) ? path : path.substr(pos + 1);
        file->is_new = false;
    }
    file->content = "-- MobileBlox V2 Script\n\n";
    
    int idx = state_.files.size();
    state_.files.push_back(std::move(file));
    state_.active_file_idx = idx;
    
    text_editor_->text = state_.files[idx]->content;
    text_editor_->UpdateLines();
    text_editor_->cursor_line = 0;
    text_editor_->cursor_col = 0;
    
    return true;
}

bool LuauEditor::OpenFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    
    auto editor_file = std::make_unique<EditorFile>();
    editor_file->path = path;
    size_t pos = path.find_last_of('/');
    editor_file->name = (pos == std::string::npos) ? path : path.substr(pos + 1);
    editor_file->content = content;
    editor_file->is_new = false;
    
    int idx = state_.files.size();
    state_.files.push_back(std::move(editor_file));
    state_.active_file_idx = idx;
    
    text_editor_->text = state_.files[idx]->content;
    text_editor_->UpdateLines();
    text_editor_->cursor_line = 0;
    text_editor_->cursor_col = 0;
    
    return true;
}

bool LuauEditor::SaveFile(int index) {
    if (index == -1) index = state_.active_file_idx;
    if (index < 0 || index >= (int)state_.files.size()) return false;
    
    auto& file = state_.files[index];
    if (file->path.empty() && file->is_new) {
        // Need Save As
        return false;
    }
    
    std::ofstream out(file->path);
    if (!out.is_open()) return false;
    
    out << file->content;
    file->modified = false;
    return true;
}

bool LuauEditor::SaveFileAs(int index, const std::string& path) {
    if (index < 0 || index >= (int)state_.files.size()) return false;
    
    auto& file = state_.files[index];
    std::ofstream out(path);
    if (!out.is_open()) return false;
    
    out << file->content;
    file->path = path;
    file->name = path.substr(path.find_last_of('/') + 1);
    file->is_new = false;
    file->modified = false;
    return true;
}

void LuauEditor::CloseFile(int index) {
    if (index < 0 || index >= (int)state_.files.size()) return;
    
    if (state_.files[index]->modified) {
        // TODO: Prompt to save
    }
    
    state_.files.erase(state_.files.begin() + index);
    
    if (state_.active_file_idx >= index) {
        state_.active_file_idx--;
    }
    if (state_.active_file_idx >= (int)state_.files.size()) {
        state_.active_file_idx = state_.files.empty() ? -1 : state_.files.size() - 1;
    }
    
    if (state_.active_file_idx >= 0) {
        text_editor_->text = state_.files[state_.active_file_idx]->content;
        text_editor_->UpdateLines();
    } else {
        text_editor_->text.clear();
        text_editor_->lines.clear();
        text_editor_->lines.push_back("");
    }
}

void LuauEditor::CloseAllFiles() {
    state_.files.clear();
    state_.active_file_idx = -1;
    text_editor_->text.clear();
    text_editor_->lines.clear();
    text_editor_->lines.push_back("");
}

std::string LuauEditor::GetContent(int index) const {
    if (index == -1) index = state_.active_file_idx;
    if (index < 0 || index >= (int)state_.files.size()) return "";
    return state_.files[index]->content;
}

void LuauEditor::SetContent(int index, const std::string& content) {
    if (index < 0 || index >= (int)state_.files.size()) return;
    state_.files[index]->content = content;
    state_.files[index]->modified = true;
    if (index == state_.active_file_idx) {
        text_editor_->text = content;
        text_editor_->UpdateLines();
    }
}

bool LuauEditor::IsModified(int index) const {
    if (index == -1) index = state_.active_file_idx;
    if (index < 0 || index >= (int)state_.files.size()) return false;
    return state_.files[index]->modified;
}

void LuauEditor::SetDiagnostics(int file_idx, const std::vector<Diagnostic>& diagnostics) {
    if (file_idx < 0 || file_idx >= (int)state_.files.size()) return;
    text_editor_->diagnostics = diagnostics;
}

const std::vector<Diagnostic>& LuauEditor::GetDiagnostics(int file_idx) const {
    static std::vector<Diagnostic> empty;
    if (file_idx < 0 || file_idx >= (int)state_.files.size()) return empty;
    return text_editor_->diagnostics;
}

void LuauEditor::RenderFileTabs() {
    if (state_.files.empty()) {
        ImGui::Button("+", ImVec2(30, 24));
        if (ImGui::IsItemClicked()) NewFile();
        return;
    }
    
    ImGuiTabBarFlags tab_flags = ImGuiTabBarFlags_Reorderable | ImGuiTabBarFlags_FittingPolicyScroll;
    if (ImGui::BeginTabBar("##editor_tabs", tab_flags)) {
        for (int i = 0; i < (int)state_.files.size(); ++i) {
            auto& file = state_.files[i];
            ImGuiTabItemFlags flags = 0;
            if (file->modified) flags |= ImGuiTabItemFlags_UnsavedDocument;
            
            std::string label = file->name + (file->modified ? "*" : "");
            bool open = true;
            if (ImGui::BeginTabItem(label.c_str(), &open, flags)) {
                state_.active_file_idx = i;
                text_editor_->text = file->content;
                text_editor_->UpdateLines();
                ImGui::EndTabItem();
            }
            if (!open) {
                CloseFile(i);
                break;
            }
        }
        
        // New file button as tab
        ImGuiTabItemFlags add_flags = ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip;
        if (ImGui::TabItemButton("+", add_flags)) {
            NewFile();
        }
        
        ImGui::EndTabBar();
    }
}

void LuauEditor::RenderEditorPane() {
    if (state_.active_file_idx < 0) {
        ImGui::TextDisabled("No file open. Click '+' to create a new file.");
        return;
    }
    
    auto& file = state_.files[state_.active_file_idx];
    auto& editor = *text_editor_;
    
    // Calculate layout
    ImVec2 avail = ImGui::GetContentRegionAvail();
    float line_num_width = editor.show_line_numbers ? 50.0f : 0.0f;
    
    // Line numbers column
    if (editor.show_line_numbers) {
        ImGui::BeginChild("##line_numbers", ImVec2(line_num_width, avail.y), false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMove);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Text, mobileblox::ui::COL_TEXT_DISABLED);
        
        for (size_t i = 0; i < editor.lines.size(); ++i) {
            bool is_current = (i == editor.cursor_line);
            if (is_current) {
                ImGui::PushStyleColor(ImGuiCol_Text, mobileblox::ui::COL_ACCENT);
            }
            
            char buf[16];
            snprintf(buf, sizeof(buf), "%4zu", i + 1);
            ImGui::Text("%s", buf);
            
            if (is_current) ImGui::PopStyleColor();
            
            // Diagnostic markers
            for (const auto& diag : editor.diagnostics) {
                if (diag.line == i + 1) {
                    ImGui::SameLine();
                    ImVec4 color = (diag.severity == Diagnostic::Severity::Error) ? mobileblox::ui::COL_ERROR :
                                   (diag.severity == Diagnostic::Severity::Warning) ? mobileblox::ui::COL_WARNING :
                                   mobileblox::ui::COL_ACCENT;
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::Text("●");
                    ImGui::PopStyleColor();
                    break;
                }
            }
        }
        
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::SameLine(0, 0);
    }
    
    // Editor area
    ImGui::BeginChild("##editor", ImVec2(avail.x - line_num_width, avail.y), false, ImGuiWindowFlags_HorizontalScrollbar);
    
    // Font
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]); // Monospace font should be first
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    
    // InputTextMultiline for editing
    ImGuiInputTextFlags flags = ImGuiInputTextFlags_AllowTabInput | 
                                ImGuiInputTextFlags_CtrlEnterForNewLine;
    
    // We need to handle the text buffer manually for syntax highlighting
    // For now, use a simple approach
    std::string& text = file->content;
    
    // Calculate cursor position for scrolling
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
    // Simple syntax highlighted rendering (read-only display with overlay input)
    // This is a simplified version - a full editor would use a custom widget
    
    if (ImGui::InputTextMultiline("##source", &text, ImVec2(-FLT_MIN, -FLT_MIN), flags)) {
        file->modified = true;
        editor.text = text;
        editor.UpdateLines();
    }
    
    // Get cursor position from InputText
    // Note: This is approximate since we can't easily get cursor from InputTextMultiline
    
    ImGui::PopStyleVar(2);
    ImGui::PopFont();
    ImGui::EndChild();
}

void LuauEditor::RenderStatusBar() {
    ImGui::Separator();
    ImGui::BeginChild("##statusbar", ImVec2(0, 24), false, ImGuiWindowFlags_NoScrollbar);
    
    if (state_.active_file_idx >= 0) {
        auto& file = state_.files[state_.active_file_idx];
        auto& editor = *text_editor_;
        
        ImGui::Text("Ln %zu, Col %zu", editor.cursor_line + 1, editor.cursor_col + 1);
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("%s", file->is_new ? "New File" : file->path.c_str());
        ImGui::SameLine();
        if (file->modified) {
            ImGui::TextColored(mobileblox::ui::COL_WARNING, "● Modified");
            ImGui::SameLine();
        }
        ImGui::Text("Luau");
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("UTF-8");
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        ImGui::Text("LF");
    } else {
        ImGui::Text("No file open");
    }
    
    ImGui::EndChild();
}

void LuauEditor::Render() {
    // Apply theme
    mobileblox::ui::ApplyTheme();
    
    // Toolbar
    if (ImGui::BeginToolbar()) {
        if (ImGui::Button("New")) NewFile();
        ImGui::SameLine();
        if (ImGui::Button("Open")) { /* TODO: file dialog */ }
        ImGui::SameLine();
        if (ImGui::Button("Save")) SaveFile();
        ImGui::SameLine();
        if (ImGui::Button("Save As")) { /* TODO: file dialog */ }
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        if (ImGui::Button("Run") && execute_callback_ && state_.active_file_idx >= 0) {
            execute_callback_(state_.files[state_.active_file_idx]->content);
        }
        ImGui::SameLine();
        if (ImGui::Button("Format")) { /* TODO: luau fmt */ }
        
        ImGui::EndToolbar();
    }
    
    // File tabs
    RenderFileTabs();
    
    // Editor pane
    RenderEditorPane();
    
    // Status bar
    RenderStatusBar();
}

// Luau keywords
const char* LuauEditor::luau_keywords[] = {
    "and", "break", "do", "else", "elseif", "end", "false", "for", "function",
    "goto", "if", "in", "local", "nil", "not", "or", "repeat", "return",
    "then", "true", "until", "while", "continue", "export", "type",
    nullptr
};

const char* LuauEditor::luau_types[] = {
    "string", "number", "boolean", "table", "function", "thread", "userdata",
    "any", "never", "unknown", "nil",
    nullptr
};

const char* LuauEditor::luau_globals[] = {
    "print", "warn", "error", "assert", "type", "typeof", "next", "pairs", "ipairs",
    "select", "tonumber", "tostring", "rawget", "rawset", "rawlen", "rawequal",
    "setmetatable", "getmetatable", "pcall", "xpcall", "load", "loadfile", "dofile",
    "require", "module", "package", "string", "table", "math", "coroutine", "debug",
    "os", "io", "bit32", "utf8", "buffer", "vector", "CFrame", "Color3", "UDim2",
    "Instance", "game", "workspace", "script", "plugin",
    nullptr
};

// LuauSyntaxHighlighter implementation
std::vector<LuauSyntaxHighlighter::Token> LuauSyntaxHighlighter::Tokenize(const std::string& code) {
    std::vector<Token> tokens;
    size_t i = 0;
    while (i < code.size()) {
        char c = code[i];
        
        // Whitespace
        if (std::isspace(c)) {
            size_t start = i;
            while (i < code.size() && std::isspace(code[i])) i++;
            tokens.push_back({TokenType::None, start, i - start});
            continue;
        }
        
        // Comments
        if (c == '-' && i + 1 < code.size() && code[i+1] == '-') {
            size_t start = i;
            if (i + 3 < code.size() && code[i+2] == '[' && code[i+3] == '[') {
                // Long comment
                i += 4;
                while (i + 1 < code.size() && !(code[i] == ']' && code[i+1] == ']')) i++;
                if (i + 1 < code.size()) i += 2;
            } else {
                // Line comment
                while (i < code.size() && code[i] != '\n') i++;
            }
            tokens.push_back({TokenType::Comment, start, i - start});
            continue;
        }
        
        // Strings
        if (c == '"' || c == '\'' || (c == '[' && i + 1 < code.size() && (code[i+1] == '[' || code[i+1] == '='))) {
            size_t start = i;
            char quote = c;
            if (quote == '[') {
                // Long string
                size_t eq_count = 0;
                while (i + 1 + eq_count < code.size() && code[i+1+eq_count] == '=') eq_count++;
                if (i + 1 + eq_count < code.size() && code[i+1+eq_count] == '[') {
                    i += 2 + eq_count;
                    std::string end_marker = "]" + std::string(eq_count, '=') + "]";
                    while (i + end_marker.size() <= code.size()) {
                        if (code.substr(i, end_marker.size()) == end_marker) {
                            i += end_marker.size();
                            break;
                        }
                        i++;
                    }
                }
            } else {
                // Regular string
                i++;
                while (i < code.size() && code[i] != quote) {
                    if (code[i] == '\\' && i + 1 < code.size()) i += 2;
                    else i++;
                }
                if (i < code.size()) i++;
            }
            tokens.push_back({TokenType::String, start, i - start});
            continue;
        }
        
        // Numbers
        if (std::isdigit(c) || (c == '.' && i + 1 < code.size() && std::isdigit(code[i+1]))) {
            size_t start = i;
            while (i < code.size() && (std::isdigit(code[i]) || code[i] == '.' || code[i] == 'e' || code[i] == 'E' || code[i] == 'x' || code[i] == 'X' || (code[i] >= 'a' && code[i] <= 'f') || (code[i] >= 'A' && code[i] <= 'F'))) i++;
            tokens.push_back({TokenType::Number, start, i - start});
            continue;
        }
        
        // Identifiers/Keywords
        if (std::isalpha(c) || c == '_') {
            size_t start = i;
            while (i < code.size() && (std::isalnum(code[i]) || code[i] == '_')) i++;
            std::string word = code.substr(start, i - start);
            
            // Check keyword
            bool is_keyword = false;
            for (const char** kw = luau_keywords; *kw; ++kw) {
                if (word == *kw) { is_keyword = true; break; }
            }
            if (!is_keyword) {
                for (const char** kw = luau_types; *kw; ++kw) {
                    if (word == *kw) { is_keyword = true; break; }
                }
            }
            if (!is_keyword) {
                for (const char** kw = luau_globals; *kw; ++kw) {
                    if (word == *kw) { is_keyword = true; break; }
                }
            }
            
            tokens.push_back({is_keyword ? TokenType::Keyword : TokenType::Identifier, start, i - start});
            continue;
        }
        
        // Operators/Punctuation
        size_t start = i;
        // Multi-char operators
        if ((c == '=' || c == '~' || c == '<' || c == '>') && i + 1 < code.size() && code[i+1] == '=') i += 2;
        else if (c == '.' && i + 1 < code.size() && code[i+1] == '.') {
            if (i + 2 < code.size() && code[i+2] == '.') i += 3;
            else i += 2;
        }
        else if (c == ':' && i + 1 < code.size() && code[i+1] == ':') i += 2;
        else i++;
        tokens.push_back({TokenType::Operator, start, i - start});
    }
    return tokens;
}

ImVec4 LuauSyntaxHighlighter::GetColor(TokenType type) {
    switch (type) {
        case TokenType::Keyword:   return mobileblox::ui::COL_ACCENT;
        case TokenType::Type:      return ImVec4(0.6f, 0.8f, 1.0f, 1.0f);
        case TokenType::Global:    return ImVec4(0.8f, 1.0f, 0.6f, 1.0f);
        case TokenType::String:    return ImVec4(0.9f, 0.7f, 0.4f, 1.0f);
        case TokenType::Number:    return ImVec4(1.0f, 0.8f, 0.4f, 1.0f);
        case TokenType::Comment:   return mobileblox::ui::COL_TEXT_DISABLED;
        case TokenType::Operator:  return mobileblox::ui::COL_ACCENT_HOVER;
        case TokenType::Preprocessor: return ImVec4(1.0f, 0.5f, 1.0f, 1.0f);
        default:                   return mobileblox::ui::COL_TEXT;
    }
}

} // namespace mobileblox::editor