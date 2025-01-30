//
// Created by xd on 03/01/25.
//

#include "../../../headers/imgui/cmp/test_cmp.h"
#include "../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <string>

#include "imgui.h"

namespace ex::gui::test {
    inline bool *watch_parent_dock_(GuiState &gui) {
        if (ex::gui::window_withing_dock(gui.docking.dock_left_id))
            return &gui.view.show_left;
        if (ex::gui::window_withing_dock(gui.docking.dock_right_id))
            return &gui.view.show_right;
        return nullptr;
    }
}

namespace ex::gui::test::one {
    const char *get_node_name(GuiState &, ex::State &) {
        return ICON_LC_COMPONENT " Component One";
    }

    bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        const std::string text = std::string(ICON_LC_COMPONENT) + " 1 " + std::string(node->name);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ex::gui::begin_docked_window(node->uuid, ptr, gui.view.reset, true);
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

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeOne();
        node->set_id(from->id_local);
        return node;
    }

    nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

}

namespace ex::gui::test::two {
    const char *get_node_name(GuiState &, ex::State &) {
        return ICON_LC_WAND_SPARKLES " Component Two";
    }

    bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        const std::string text = std::string(ICON_LC_WAND_SPARKLES) + " 2 " + std::string(node->name);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ex::gui::begin_docked_window(node->uuid, ptr, gui.view.reset, true);
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

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeTwo();
        node->set_id(from->id_local);
        return node;
    }

    nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

}

namespace ex::gui::test::tri {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    const char *get_node_name(GuiState &, ex::State &) {
        return "Component Three";
    }

    bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        return plain_node(node->name, true, selected, nested);
    }

    void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, const bool nested, const bool selected) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node) {
        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool show = true;
        bool *ptr = (node->pinned || !node->docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        ex::gui::begin_docked_window(node->uuid, ptr, gui.view.reset, true);
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

    nodes::BaseNode *instance(const State &state, TreeNode *from) {
        const auto node = new NodeTri();
        node->set_id(from->id_local);
        return node;
    }

    nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }
}