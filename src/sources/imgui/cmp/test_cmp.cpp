//
// Created by xd on 03/01/25.
//

#include "../../../headers/imgui/cmp/test_cmp.h"
#include "../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <string>

#include "imgui.h"

namespace ex::gui::test {
    inline int reset_flag_(GuiState &gui, const bool standalone = false) {
        return gui.view.reset ? (ImGuiWindowFlags_NoSavedSettings
                                 | (standalone ? ImGuiWindowFlags_AlwaysAutoResize : 0))
                              : 0;
    }

    inline bool *watch_parent_dock_(GuiState &gui) {
        if (ex::gui::window_withing_dock(gui.docking.dock_left_id))
            return &gui.view.show_left;
        if (ex::gui::window_withing_dock(gui.docking.dock_right_id))
            return &gui.view.show_right;
        return nullptr;
    }
}

namespace ex::gui::test::one {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const std::string text = std::string(ICON_LC_COMPONENT) + " 1 " + std::string(name);
        const char *text_2 = text.c_str();
        const auto c_bullet = nested ? 0 : tree_bull_flags;

        if (!nested) ImGui::Unindent();
        const bool open = ImGui::TreeNodeEx(text_2, c_flag | c_bullet);
        if (!nested) ImGui::Indent();
        return open;
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        ImGui::End();

        return show;
    }

    void from_base(State &state, nodes::BaseNode *from, TreeNode *to) {

    }

    void from_tree(State &state, TreeNode *from, nodes::BaseNode *to) {

    }

    nodes::BaseNode *re_instance(State &state, TreeNode *from, nodes::BaseNode *src) {
        return src;
    }

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeOne();
        node->set_id(from->id_local);
        return node;
    }

    void cleanup(void *data) {

    }
}

namespace ex::gui::test::two {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const std::string text = std::string(ICON_LC_WAND_SPARKLES) + " 2 " + std::string(name);
        const char *text_2 = text.c_str();
        const auto c_bullet = nested ? 0 : tree_bull_flags;

        if (!nested) ImGui::Unindent();
        const bool open = ImGui::TreeNodeEx(text_2, c_flag | c_bullet);
        if (!nested) ImGui::Indent();
        return open;
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        ImGui::End();

        return show;
    }

    void from_base(State &state, nodes::BaseNode *from, TreeNode *to) {

    }

    void from_tree(State &state, TreeNode *from, nodes::BaseNode *to) {

    }

    nodes::BaseNode *re_instance(State &state, TreeNode *from, nodes::BaseNode *src) {
        return src;
    }

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeTwo();
        node->set_id(from->id_local);
        return node;
    }

    void cleanup(void *data) {

    }
}

namespace ex::gui::test::tri {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const auto c_bullet = nested ? 0 : tree_bull_flags;
        return ImGui::TreeNodeEx(name, c_flag | c_bullet);
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        ImGui::End();

        return show;
    }

    void from_base(State &state, nodes::BaseNode *from, TreeNode *to) {

    }

    void from_tree(State &state, TreeNode *from, nodes::BaseNode *to) {

    }

    nodes::BaseNode *re_instance(State &state, TreeNode *from, nodes::BaseNode *src) {
        return src;
    }

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeTri();
        node->set_id(from->id_local);
        return node;
    }

    void cleanup(void *data) {

    }
}