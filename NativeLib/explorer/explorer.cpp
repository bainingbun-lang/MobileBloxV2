#include "explorer/explorer.hpp"
#include "ui/theme.hpp"
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <iomanip>

namespace mobileblox::explorer {

Explorer::Explorer() {
    state_.root_item = std::make_unique<ExplorerItem>("Project", "", ItemType::Folder, true);
    state_.root_item->expanded = true;
}

Explorer::~Explorer() = default;

bool Explorer::SetProjectRoot(const fs::path& path) {
    if (!fs::exists(path)) {
        if (!fs::create_directories(path)) return false;
    }
    state_.root_path = fs::absolute(path);
    state_.root_item->name = state_.root_path.filename().string();
    state_.root_item->path = state_.root_path;
    Refresh();
    return true;
}

bool Explorer::LoadProject(const fs::path& project_file) {
    // TODO: Load .mbproj JSON file
    return SetProjectRoot(project_file.parent_path());
}

bool Explorer::SaveProject(const fs::path& project_file) {
    // TODO: Save .mbproj JSON file
    return true;
}

void Explorer::Refresh() {
    if (!fs::exists(state_.root_path)) return;
    state_.root_item = BuildTree(state_.root_path, nullptr);
    state_.root_item->expanded = true;
    state_.selected_items.clear();
}

ExplorerItem* Explorer::FindItem(const fs::path& path) {
    auto abs_path = fs::absolute(path);
    std::function<ExplorerItem*(ExplorerItem*)> search = [&](ExplorerItem* item) -> ExplorerItem* {
        if (fs::absolute(item->path) == abs_path) return item;
        for (auto& child : item->children) {
            if (auto found = search(child.get())) return found;
        }
        return nullptr;
    };
    return search(state_.root_item.get());
}

void Explorer::SelectItem(ExplorerItem* item, bool multi_select) {
    if (!item) return;
    
    if (!multi_select) {
        // Clear previous selection
        for (auto* sel : state_.selected_items) {
            sel->selected = false;
        }
        state_.selected_items.clear();
    }
    
    item->selected = true;
    state_.selected_items.push_back(item);
    
    if (item_selected_cb_) item_selected_cb_(item);
}

void Explorer::ClearSelection() {
    for (auto* item : state_.selected_items) {
        item->selected = false;
    }
    state_.selected_items.clear();
}

bool Explorer::CreateFolder(ExplorerItem* parent, const std::string& name) {
    if (!parent || !parent->is_folder) return false;
    
    fs::path new_path = parent->path / name;
    if (fs::exists(new_path)) return false;
    
    if (!fs::create_directory(new_path)) return false;
    
    auto child = std::make_unique<ExplorerItem>(name, new_path, ItemType::Folder, true);
    child->parent = parent;
    parent->children.push_back(std::move(child));
    parent->expanded = true;
    
    if (file_created_cb_) file_created_cb_(new_path);
    return true;
}

bool Explorer::CreateScript(ExplorerItem* parent, const std::string& name, ItemType type) {
    if (!parent || !parent->is_folder) return false;
    
    std::string ext;
    switch (type) {
        case ItemType::Script: ext = ".server.lua"; break;
        case ItemType::LocalScript: ext = ".client.lua"; break;
        case ItemType::ModuleScript: ext = ".module.lua"; break;
        default: ext = ".lua"; break;
    }
    
    fs::path new_path = parent->path / (name + ext);
    if (fs::exists(new_path)) return false;
    
    // Create with basic template
    std::ofstream file(new_path);
    if (!file.is_open()) return false;
    
    if (type == ItemType::ModuleScript) {
        file << "local Module = {}\n\nfunction Module.Init()\n    print(\"Module initialized\")\nend\n\nreturn Module\n";
    } else {
        file << "-- " << name << "\n\nprint(\"Hello from " << name << "\")\n";
    }
    
    auto child = std::make_unique<ExplorerItem>(name + ext, new_path, type, false);
    child->parent = parent;
    parent->children.push_back(std::move(child));
    parent->expanded = true;
    
    if (file_created_cb_) file_created_cb_(new_path);
    return true;
}

bool Explorer::DeleteItem(ExplorerItem* item) {
    if (!item || !item->parent) return false;
    
    // Remove from filesystem
    if (fs::exists(item->path)) {
        if (item->is_folder) {
            fs::remove_all(item->path);
        } else {
            fs::remove(item->path);
        }
    }
    
    // Remove from tree
    auto& siblings = item->parent->children;
    auto it = std::find_if(siblings.begin(), siblings.end(), 
        [item](const std::unique_ptr<ExplorerItem>& ptr) { return ptr.get() == item; });
    if (it != siblings.end()) {
        // Clear selection if deleted item was selected
        if (item->selected) {
            item->selected = false;
            state_.selected_items.erase(
                std::remove(state_.selected_items.begin(), state_.selected_items.end(), item),
                state_.selected_items.end()
            );
        }
        siblings.erase(it);
        return true;
    }
    return false;
}

bool Explorer::RenameItem(ExplorerItem* item, const std::string& new_name) {
    if (!item || !item->parent) return false;
    
    fs::path new_path = item->parent->path / new_name;
    if (fs::exists(new_path)) return false;
    
    try {
        fs::rename(item->path, new_path);
        item->name = new_name;
        item->path = new_path;
        // Update children paths recursively
        std::function<void(ExplorerItem*)> update_paths = [&](ExplorerItem* child) {
            child->path = new_path / child->name;
            for (auto& grandchild : child->children) {
                update_paths(grandchild.get());
            }
        };
        for (auto& child : item->children) {
            update_paths(child.get());
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool Explorer::MoveItem(ExplorerItem* item, ExplorerItem* new_parent) {
    if (!item || !item->parent || !new_parent || !new_parent->is_folder) return false;
    if (item == new_parent) return false;
    
    // Check if new_parent is a descendant of item
    ExplorerItem* check = new_parent;
    while (check) {
        if (check == item) return false;
        check = check->parent;
    }
    
    fs::path new_path = new_parent->path / item->name;
    if (fs::exists(new_path)) return false;
    
    try {
        fs::rename(item->path, new_path);
        
        // Remove from old parent
        auto& old_siblings = item->parent->children;
        old_siblings.erase(
            std::remove_if(old_siblings.begin(), old_siblings.end(),
                [item](const std::unique_ptr<ExplorerItem>& ptr) { return ptr.get() == item; }),
            old_siblings.end()
        );
        
        // Add to new parent
        item->parent = new_parent;
        item->path = new_path;
        new_parent->children.push_back(std::unique_ptr<ExplorerItem>(item));
        new_parent->expanded = true;
        
        // Update children paths
        std::function<void(ExplorerItem*)> update_paths = [&](ExplorerItem* child) {
            child->path = new_path / child->name;
            for (auto& grandchild : child->children) {
                update_paths(grandchild.get());
            }
        };
        for (auto& child : item->children) {
            update_paths(child.get());
        }
        return true;
    } catch (...) {
        return false;
    }
}

bool Explorer::DuplicateItem(ExplorerItem* item) {
    if (!item || !item->parent) return false;
    
    std::string base_name = item->name;
    size_t dot_pos = base_name.rfind('.');
    std::string name_part = (dot_pos == std::string::npos) ? base_name : base_name.substr(0, dot_pos);
    std::string ext = (dot_pos == std::string::npos) ? "" : base_name.substr(dot_pos);
    
    int counter = 1;
    std::string new_name;
    do {
        new_name = name_part + " Copy" + (counter > 1 ? std::to_string(counter) : "") + ext;
        counter++;
    } while (fs::exists(item->parent->path / new_name));
    
    fs::path new_path = item->parent->path / new_name;
    fs::copy(item->path, new_path, fs::copy_options::recursive);
    
    auto child = std::make_unique<ExplorerItem>(new_name, new_path, item->type, item->is_folder);
    child->parent = item->parent;
    item->parent->children.push_back(std::move(child));
    item->parent->expanded = true;
    
    return true;
}

Explorer::ItemType Explorer::GetItemType(const fs::path& path) {
    std::string ext = path.extension().string();
    std::string name = path.filename().string();
    
    if (ext == ".server.lua" || name.find(".server.") != std::string::npos) return ItemType::Script;
    if (ext == ".client.lua" || name.find(".client.") != std::string::npos) return ItemType::LocalScript;
    if (ext == ".module.lua" || name.find(".module.") != std::string::npos) return ItemType::ModuleScript;
    if (ext == ".lua") return ItemType::Script;
    if (ext == ".txt" || ext == ".md") return ItemType::TextFile;
    if (ext == ".json") return ItemType::JsonFile;
    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp") return ItemType::ImageFile;
    return ItemType::Other;
}

std::unique_ptr<ExplorerItem> Explorer::BuildTree(const fs::path& path, ExplorerItem* parent) {
    std::string name = path.filename().string();
    if (name.empty()) name = path.string();
    
    bool is_folder = fs::is_directory(path);
    ItemType type = is_folder ? ItemType::Folder : GetItemType(path);
    
    auto item = std::make_unique<ExplorerItem>(name, path, type, is_folder);
    item->parent = parent;
    
    if (is_folder) {
        item->expanded = false;
        try {
            for (const auto& entry : fs::directory_iterator(path)) {
                if (!state_.show_hidden && entry.path().filename().string().starts_with(".")) continue;
                
                auto child = BuildTree(entry.path(), item.get());
                if (child) item->children.push_back(std::move(child));
            }
        } catch (...) {
            // Permission denied or other error
        }
        
        // Sort: folders first, then alphabetically
        std::sort(item->children.begin(), item->children.end(),
            [](const std::unique_ptr<ExplorerItem>& a, const std::unique_ptr<ExplorerItem>& b) {
                if (a->is_folder != b->is_folder) return a->is_folder > b->is_folder;
                return a->name < b->name;
            });
    } else {
        // File properties
        try {
            auto ftime = fs::last_write_time(path);
            auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
            );
            std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M");
            item->modified_time = ss.str();
            item->size = fs::file_size(path);
        } catch (...) {}
    }
    
    return item;
}

void Explorer::ApplyFilter(ExplorerItem* item, const std::string& filter, bool& any_match) {
    if (filter.empty()) {
        item->expanded = true;
        any_match = true;
        return;
    }
    
    std::string lower_name = item->name;
    std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
    std::string lower_filter = filter;
    std::transform(lower_filter.begin(), lower_filter.end(), lower_filter.begin(), ::tolower);
    
    bool matches = lower_name.find(lower_filter) != std::string::npos;
    bool child_matches = false;
    
    for (auto& child : item->children) {
        bool child_any = false;
        ApplyFilter(child.get(), filter, child_any);
        if (child_any) child_matches = true;
    }
    
    any_match = matches || child_matches;
    if (child_matches) item->expanded = true;
}

void Explorer::RenderToolbar() {
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6, 4));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 0));
    
    if (ImGui::Button(ICON_FA_FOLDER_PLUS " New Folder")) {
        if (!state_.selected_items.empty() && state_.selected_items[0]->is_folder) {
            // TODO: Show input dialog for name
        } else if (state_.root_item) {
            // TODO: Show input dialog for name
        }
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_FILE_CODE " New Script")) {
        // TODO: Show script type selector
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_SYNC " Refresh")) {
        Refresh();
    }
    ImGui::SameLine();
    ImGui::Dummy(ImVec2(10, 0));
    ImGui::SameLine();
    
    // Expand/Collapse all
    if (ImGui::Button(ICON_FA_EXPAND " Expand All")) {
        std::function<void(ExplorerItem*)> expand = [&](ExplorerItem* item) {
            if (item->is_folder) {
                item->expanded = true;
                for (auto& child : item->children) expand(child.get());
            }
        };
        expand(state_.root_item.get());
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_COMPRESS " Collapse All")) {
        std::function<void(ExplorerItem*)> collapse = [&](ExplorerItem* item) {
            if (item->is_folder) {
                item->expanded = false;
                for (auto& child : item->children) collapse(child.get());
            }
        };
        collapse(state_.root_item.get());
    }
    
    ImGui::PopStyleVar(2);
}

void Explorer::RenderSearchBar() {
    ImGui::PushItemWidth(-1);
    ImGui::InputTextWithHint("##search", ICON_FA_SEARCH " Search...", &state_.search_filter);
    ImGui::PopItemWidth();
}

void Explorer::RenderTree(ExplorerItem* item, int depth) {
    if (!item) return;
    
    // Apply filter
    if (!state_.search_filter.empty()) {
        bool any_match = false;
        ApplyFilter(item, state_.search_filter, any_match);
        if (!any_match) return;
    }
    
    RenderItem(item, depth);
    
    if (item->expanded && item->is_folder) {
        for (auto& child : item->children) {
            RenderTree(child.get(), depth + 1);
        }
    }
}

void Explorer::RenderItem(ExplorerItem* item, int depth) {
    ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick |
                               ImGuiTreeNodeFlags_SpanAvailWidth;
    if (!item->is_folder) flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    if (item->selected) flags |= ImGuiTreeNodeFlags_Selected;
    
    // Indent
    ImGui::Indent(depth * 16.0f);
    
    // Icon
    const char* icon = "";
    ImVec4 icon_color = mobileblox::ui::COL_TEXT;
    switch (item->type) {
        case ItemType::Folder: icon = ICON_FA_FOLDER; icon_color = mobileblox::ui::COL_ACCENT; break;
        case ItemType::Script: icon = ICON_FA_FILE_CODE; icon_color = ImVec4(0.4f, 0.8f, 1.0f, 1.0f); break;
        case ItemType::LocalScript: icon = ICON_FA_FILE_CODE; icon_color = ImVec4(0.4f, 1.0f, 0.6f, 1.0f); break;
        case ItemType::ModuleScript: icon = ICON_FA_CUBE; icon_color = ImVec4(1.0f, 0.8f, 0.4f, 1.0f); break;
        case ItemType::TextFile: icon = ICON_FA_FILE_ALT; icon_color = mobileblox::ui::COL_TEXT; break;
        case ItemType::JsonFile: icon = ICON_FA_FILE_CODE; icon_color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f); break;
        case ItemType::ImageFile: icon = ICON_FA_FILE_IMAGE; icon_color = ImVec4(1.0f, 0.6f, 1.0f, 1.0f); break;
        default: icon = ICON_FA_FILE; icon_color = mobileblox::ui::COL_TEXT; break;
    }
    
    ImGui::PushStyleColor(ImGuiCol_Text, icon_color);
    ImGui::Text("%s", icon);
    ImGui::PopStyleColor();
    ImGui::SameLine(0, 4);
    
    bool opened = ImGui::TreeNodeEx(item->name.c_str(), flags, "%s", item->name.c_str());
    
    // Drag and drop
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
        ImGui::SetDragDropPayload("EXPLORER_ITEM", &item, sizeof(ExplorerItem*));
        ImGui::Text("%s %s", icon, item->name.c_str());
        ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER_ITEM")) {
            ExplorerItem** src = (ExplorerItem**)payload->Data;
            if (*src != item && item->is_folder) {
                MoveItem(*src, item);
            }
        }
        ImGui::EndDragDropTarget();
    }
    
    // Context menu
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
        OpenContextMenu(item, ImGui::GetMousePos());
    }
    
    // Double click
    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left)) {
        if (item_double_clicked_cb_) item_double_clicked_cb_(item);
        if (!item->is_folder) {
            // Open in editor
        }
    }
    
    // Selection
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        bool multi = ImGui::GetIO().KeyCtrl || ImGui::GetIO().KeyShift;
        SelectItem(item, multi);
    }
    
    if (opened && item->is_folder) {
        ImGui::TreePop();
    }
    
    ImGui::Unindent(depth * 16.0f);
}

void Explorer::RenderContextMenu() {
    if (!state_.show_context_menu || !state_.context_item) return;
    
    if (ImGui::BeginPopupContextWindow("##explorer_context")) {
        auto* item = state_.context_item;
        
        if (ImGui::MenuItem(ICON_FA_FILE_CODE " New Script", nullptr, false, item->is_folder)) {
            // TODO: Show script type dialog
        }
        if (ImGui::MenuItem(ICON_FA_FOLDER_PLUS " New Folder", nullptr, false, item->is_folder)) {
            // TODO: Show name input
        }
        if (ImGui::MenuItem(ICON_FA_COPY " Duplicate", nullptr, false, true)) {
            DuplicateItem(item);
        }
        if (ImGui::MenuItem(ICON_FA_EDIT " Rename", nullptr, false, true)) {
            // TODO: Show rename input
        }
        if (ImGui::MenuItem(ICON_FA_TRASH " Delete", nullptr, false, true)) {
            DeleteItem(item);
        }
        ImGui::Separator();
        if (ImGui::MenuItem(ICON_FA_COPY " Copy Path")) {
            ImGui::SetClipboardText(item->path.string().c_str());
        }
        if (ImGui::MenuItem(ICON_FA_EXTERNAL_LINK_ALT " Open in Explorer")) {
            // TODO: Open system file explorer
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Properties")) {
            // Show properties in inspector
        }
        
        ImGui::EndPopup();
    }
}

void Explorer::OpenContextMenu(ExplorerItem* item, ImVec2 pos) {
    state_.context_item = item;
    state_.context_pos = pos;
    state_.show_context_menu = true;
    ImGui::OpenPopup("##explorer_context");
}

void Explorer::RenderInspector() {
    if (state_.selected_items.empty()) {
        ImGui::TextDisabled("No item selected");
        return;
    }
    
    auto* item = state_.selected_items[0];
    
    ImGui::Separator();
    ImGui::Text("Properties");
    ImGui::Separator();
    
    ImGui::BeginTable("##properties", 2, ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_SizingStretchProp);
    ImGui::TableSetupColumn("Property", ImGuiTableColumnFlags_WidthFixed, 120);
    ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
    
    auto prop_row = [](const char* name, const char* value) {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::TextDisabled("%s", name);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%s", value);
    };
    
    prop_row("Name", item->name.c_str());
    prop_row("Type", item->is_folder ? "Folder" : "File");
    prop_row("Path", item->path.string().c_str());
    if (!item->modified_time.empty()) prop_row("Modified", item->modified_time.c_str());
    if (item->size > 0) {
        char buf[32];
        if (item->size < 1024) snprintf(buf, sizeof(buf), "%zu B", item->size);
        else if (item->size < 1024*1024) snprintf(buf, sizeof(buf), "%.1f KB", item->size/1024.0);
        else snprintf(buf, sizeof(buf), "%.1f MB", item->size/1024.0/1024.0);
        prop_row("Size", buf);
    }
    
    ImGui::EndTable();
}

void Explorer::Render() {
    if (!visible_) return;
    
    mobileblox::ui::ApplyTheme();
    
    if (!ImGui::Begin("Explorer", &visible_, ImGuiWindowFlags_MenuBar)) {
        ImGui::End();
        return;
    }
    
    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New Folder", "Ctrl+Shift+N")) {}
            if (ImGui::MenuItem("New Script", "Ctrl+N")) {}
            if (ImGui::MenuItem("Open Project...", "Ctrl+O")) {}
            if (ImGui::MenuItem("Save Project", "Ctrl+S")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Close Project")) {}
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Expand All", nullptr, false, true);
            ImGui::MenuItem("Collapse All", nullptr, false, true);
            ImGui::Separator();
            ImGui::MenuItem("Show Hidden Files", nullptr, &state_.show_hidden);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    
    // Toolbar
    RenderToolbar();
    ImGui::Separator();
    
    // Search
    RenderSearchBar();
    ImGui::Separator();
    
    // Tree
    ImGui::BeginChild("##explorer_tree", ImVec2(0, -100), false, ImGuiWindowFlags_HorizontalScrollbar);
    if (state_.root_item) {
        RenderTree(state_.root_item.get());
    } else {
        ImGui::TextDisabled("No project open. Use File > Open Project...");
    }
    ImGui::EndChild();
    
    // Inspector
    ImGui::Separator();
    ImGui::BeginChild("##inspector", ImVec2(0, 100), false);
    RenderInspector();
    ImGui::EndChild();
    
    // Context menu
    RenderContextMenu();
    
    ImGui::End();
}

} // namespace mobileblox::explorer