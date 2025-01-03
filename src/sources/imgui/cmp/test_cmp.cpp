//
// Created by xd on 03/01/25.
//

#include "../../../headers/imgui/cmp/test_cmp.h"

#include <IconsLucide.h>
#include <string>

#include "imgui.h"

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
}

namespace ex::gui::test::tri {
    constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
    constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;

    bool draw_tree_node(GuiState &gui, const char *name, const bool nested, const bool selected) {
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);
        const auto c_bullet = nested ? 0 : tree_bull_flags;
        return ImGui::TreeNodeEx(name, c_flag | c_bullet);
    }
}