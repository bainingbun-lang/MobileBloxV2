#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <filesystem>
#include <imgui.h>

namespace mobileblox::explorer {

namespace fs = std::filesystem;

enum class ItemType {
    Folder,
    Script,
    ModuleScript,
    LocalScript,
    TextFile,
    ImageFile,
    JsonFile,
    Other
};

struct ExplorerItem {
    std::string name;
    fs::path path;
    ItemType type = ItemType::Other;
    bool is_folder = false;
    bool expanded = false;
    bool selected = false;
    std::vector<std::unique_ptr<ExplorerItem>> children;
    ExplorerItem* parent = nullptr;
    
    // Properties for Inspector
    std::string class_name;
    std::string parent_path;
    size_t size = 0;
    std::string modified_time;
    
    ExplorerItem() = default;
    ExplorerItem(const std::string& n, const fs::path& p, ItemType t, bool folder)
        : name(n), path(p), type(t), is_folder(folder) {}
};

struct ExplorerState {
    fs::path root_path;
    std::unique_ptr<ExplorerItem> root_item;
    std::vector<ExplorerItem*> selected_items;
    std::string search_filter;
    bool show_hidden = false;
    bool show_file_types = true;
    ExplorerItem* context_item = nullptr;
    ImVec2 context_pos;
    bool show_context_menu = false;
};

class Explorer {
public:
    Explorer();
    ~Explorer();
    
    // Project management
    bool SetProjectRoot(const fs::path& path);
    bool LoadProject(const fs::path& project_file); // .mbproj
    bool SaveProject(const fs::path& project_file);
    void Refresh();
    
    // Tree operations
    void ExpandAll(ExplorerItem* item = nullptr);
    void CollapseAll(ExplorerItem* item = nullptr);
    ExplorerItem* FindItem(const fs::path& path);
    void SelectItem(ExplorerItem* item, bool multi_select = false);
    void ClearSelection();
    
    // File operations
    bool CreateFolder(ExplorerItem* parent, const std::string& name);
    bool CreateScript(ExplorerItem* parent, const std::string& name, ItemType type = ItemType::Script);
    bool DeleteItem(ExplorerItem* item);
    bool RenameItem(ExplorerItem* item, const std::string& new_name);
    bool MoveItem(ExplorerItem* item, ExplorerItem* new_parent);
    bool DuplicateItem(ExplorerItem* item);
    
    // Rendering
    void Render();
    void RenderTree(ExplorerItem* item, int depth = 0);
    void RenderContextMenu();
    void RenderToolbar();
    void RenderSearchBar();
    void RenderInspector();
    
    // State
    ExplorerState& GetState() { return state_; }
    const ExplorerState& GetState() const { return state_; }
    ExplorerItem* GetRootItem() { return state_.root_item.get(); }
    const std::vector<ExplorerItem*>& GetSelectedItems() const { return state_.selected_items; }
    
    // Callbacks
    using ItemSelectedCallback = std::function<void(ExplorerItem*)>;
    using ItemDoubleClickedCallback = std::function<void(ExplorerItem*)>;
    using FileCreatedCallback = std::function<void(const fs::path&)>;
    
    void SetItemSelectedCallback(ItemSelectedCallback cb) { item_selected_cb_ = cb; }
    void SetItemDoubleClickedCallback(ItemDoubleClickedCallback cb) { item_double_clicked_cb_ = cb; }
    void SetFileCreatedCallback(FileCreatedCallback cb) { file_created_cb_ = cb; }
    
    // Toggle visibility
    void SetVisible(bool visible) { visible_ = visible; }
    bool IsVisible() const { return visible_; }
    void ToggleVisible() { visible_ = !visible_; }

private:
    ExplorerState state_;
    bool visible_ = true;
    
    ItemSelectedCallback item_selected_cb_;
    ItemDoubleClickedCallback item_double_clicked_cb_;
    FileCreatedCallback file_created_cb_;
    
    // Helper functions
    ItemType GetItemType(const fs::path& path);
    std::unique_ptr<ExplorerItem> BuildTree(const fs::path& path, ExplorerItem* parent);
    void CollectChildren(ExplorerItem* item, std::vector<ExplorerItem*>& out);
    void ApplyFilter(ExplorerItem* item, const std::string& filter, bool& any_match);
    void RenderItem(ExplorerItem* item, int depth);
    void RenderItemProperties(ExplorerItem* item);
    void OpenContextMenu(ExplorerItem* item, ImVec2 pos);
};

} // namespace mobileblox::explorer