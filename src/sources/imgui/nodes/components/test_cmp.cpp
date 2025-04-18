//
// Created by xd on 03/01/25.
//

#include "../../../../headers/imgui/nodes/components/test_cmp.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"
#include "imgui.h"

#include <IconsLucide.h>
#include <string>

namespace ex::gui::test::one {
    const char *get_node_name() {
        return "Component One";
    }

    const char *get_node_preview() {
        return ICON_LC_COMPONENT " Component One";
    }

    bool draw_tree_node(TreeNode *node, const bool nested, const bool selected, const bool collapse, const bool expand) {
        const std::string text  = std::string(ICON_LC_COMPONENT) + " 1 " + std::string(node->name);
        if (expand)   ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        if (collapse) ImGui::SetNextItemOpen(false, ImGuiCond_Always);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiNodeWindow *node) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("This window 1: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::graph::BaseNode *instance(TreeNode *from) {
        const auto node = new NodeOne();
        node->set_id(from->id_local);
        return node;
    }

    limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

    void destroy(ex::TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void disable(TreeNode *, limbo::graph::BaseNode *) {
    }

    void enable(TreeNode *, limbo::graph::BaseNode *) {
    }
} // namespace ex::gui::test::one

namespace ex::gui::test::two {
    const char *get_node_name() {
        return "Component Two";
    }

    const char *get_node_preview() {
        return ICON_LC_WAND_SPARKLES " Component Two";
    }

    bool draw_tree_node(TreeNode *node, const bool nested, const bool selected, const bool collapse, const bool expand) {
        const std::string text = std::string(ICON_LC_WAND_SPARKLES) + " 2 " + std::string(node->name);
        if (expand)   ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        if (collapse) ImGui::SetNextItemOpen(false, ImGuiCond_Always);
        return generic_node(text.c_str(), false, selected, nested);
    }

    void draw_tree_menu(TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiNodeWindow *node) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("This window 2: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::graph::BaseNode *instance(TreeNode *from) {
        const auto node = new NodeTwo();
        node->set_id(from->id_local);
        return node;
    }

    limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

    void destroy(ex::TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void disable(TreeNode *, limbo::graph::BaseNode *) {
    }

    void enable(TreeNode *, limbo::graph::BaseNode *) {
    }
}

namespace ex::gui::test::tri {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    const char *get_node_name() {
        return "Component Three";
    }

    const char *get_node_preview() {
        return "Component Three";
    }

    bool draw_tree_node(TreeNode *node, const bool nested, const bool selected, const bool collapse, const bool expand) {
        if (expand)   ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        if (collapse) ImGui::SetNextItemOpen(false, ImGuiCond_Always);
        return plain_node(node->name, true, selected, nested);
    }

    void draw_tree_menu(TreeNode *node, const bool , const bool ) {
        ImGui::Separator();
        if (ImGui::MenuItem(node->name)) {
            // Do something TODO
        }
        ImGui::Separator();
    }

    bool draw_node_window(GuiNodeWindow *node) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("This window 3: %d | %d | %d", node->docked, node->pinned, gui.view.docking);
        return true;
    }

    limbo::graph::BaseNode *instance(TreeNode *from) {
        const auto node = new NodeTri();
        node->set_id(from->id_local);
        return node;
    }

    limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node) {
        std::cout << "relocate: " << tree->id_local << '\n';
        return node;
    }

    void destroy(ex::TreeNode *, limbo::graph::BaseNode *node) {
        node->detach_all();
        delete node;
    }

    void disable(TreeNode *, limbo::graph::BaseNode *) {
    }

    void enable(TreeNode *, limbo::graph::BaseNode *) {
    }
}