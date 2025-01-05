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
        const std::string text = std::string(ICON_LC_COMPONENT) + " " + std::string(name);
        const char *text_2 = text.c_str();
        const auto c_bullet = nested ? 0 : tree_bull_flags;

        if (!nested) ImGui::Unindent();
        const bool open = ImGui::TreeNodeEx(text_2, c_flag | c_bullet);
        if (!nested) ImGui::Indent();
        return open;
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node, const bool first_time) {
        if (!node->show)
            return false;

        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool *ptr = (!node->docked || gui.view.docking) ? &node->show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, (gui.view.reset || first_time) ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d", node->docked, gui.view.docking);
        ImGui::End();

        return node->show;
    }
}

namespace ex::gui::test::two {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const std::string text = std::string(ICON_LC_WAND_SPARKLES) + " " + std::string(name);
        const char *text_2 = text.c_str();
        const auto c_bullet = nested ? 0 : tree_bull_flags;

        if (!nested) ImGui::Unindent();
        const bool open = ImGui::TreeNodeEx(text_2, c_flag | c_bullet);
        if (!nested) ImGui::Indent();
        return open;
    }

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node, const bool first_time) {
        if (!node->show)
            return false;

        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool *ptr = (!node->docked || gui.view.docking) ? &node->show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, (gui.view.reset || first_time) ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d", node->docked, gui.view.docking);
        ImGui::End();

        return node->show;
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

    bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *node, const bool first_time) {
        if (!node->show)
            return false;

        if (node->visible != nullptr && !(*node->visible))
            return false;

        bool *ptr = (!node->docked || gui.view.docking) ? &node->show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, (gui.view.reset || first_time) ? ImGuiCond_Always : ImGuiCond_Once);

        ImGui::Begin(node->uuid, ptr, reset_flag_(gui, true) | ImGuiWindowFlags_NoCollapse);
        node->docked = ex::gui::window_is_docked();
        node->visible = watch_parent_dock_(gui);
        ImGui::Text("This window: %d | %d", node->docked, gui.view.docking);
        ImGui::End();

        return node->show;
    }
}