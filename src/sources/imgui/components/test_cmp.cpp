//
// Created by xd on 03/01/25.
//

#include "../../../headers/imgui/components/test_cmp.h"
#include "../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <string>

#include "imgui.h"

namespace ex::gui::test::one {
    const char *get_node_name(GuiState &, ex::State &) {
        return ICON_LC_COMPONENT " Component One";
    }

    bool draw_tree_node(GuiState &, ex::State &, TreeNode *node, const bool nested, const bool selected) {
        const std::string text = std::string(ICON_LC_COMPONENT) + " 1 " + std::string(node->name);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(GuiState &, ex::State &, TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &, GuiNodeWindow *node) {
        ImGui::Text("This window 1: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::nodes::BaseNode *instance(const State &, TreeNode *from) {
        const auto node = new NodeOne();
        node->set_id(from->id_local);
        return node;
    }

    limbo::nodes::BaseNode *relocate(const State &, TreeNode *tree, limbo::nodes::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

}

namespace ex::gui::test::two {
    const char *get_node_name(GuiState &, ex::State &) {
        return ICON_LC_WAND_SPARKLES " Component Two";
    }

    bool draw_tree_node(GuiState &, ex::State &, TreeNode *node, const bool nested, const bool selected) {
        const std::string text = std::string(ICON_LC_WAND_SPARKLES) + " 2 " + std::string(node->name);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(GuiState &, ex::State &, TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &, GuiNodeWindow *node) {
        ImGui::Text("This window 2: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::nodes::BaseNode *instance(const State &, TreeNode *from) {
        const auto node = new NodeTwo();
        node->set_id(from->id_local);
        return node;
    }

    limbo::nodes::BaseNode *relocate(const State &, TreeNode *tree, limbo::nodes::BaseNode *node) {
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

    bool draw_tree_node(GuiState &, ex::State &, TreeNode *node, const bool nested, const bool selected) {
        return plain_node(node->name, true, selected, nested);
    }

    void draw_tree_menu(GuiState &, ex::State &, TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiState &gui, ex::State &, GuiNodeWindow *node) {
        ImGui::Text("This window 3: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::nodes::BaseNode *instance(const State &, TreeNode *from) {
        const auto node = new NodeTri();
        node->set_id(from->id_local);
        return node;
    }

    limbo::nodes::BaseNode *relocate(const State &, TreeNode *tree, limbo::nodes::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

}