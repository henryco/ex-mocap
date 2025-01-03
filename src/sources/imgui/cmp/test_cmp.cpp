//
// Created by xd on 03/01/25.
//

#include "../../../headers/imgui/cmp/test_cmp.h"
#include "imgui.h"

namespace ex::gui::test {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;
}

namespace ex::gui::test::one {
    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const auto c_bullet = nested ? 0 : tree_bull_flags;
        return ImGui::TreeNodeEx(name, c_flag | c_bullet);
    }
}

namespace ex::gui::test::two {
    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const auto c_bullet = nested ? 0 : tree_bull_flags;
        return ImGui::TreeNodeEx(name, c_flag | c_bullet);
    }
}