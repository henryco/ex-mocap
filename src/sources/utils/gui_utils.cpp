//
// Created by xd on 27/11/24.
//

#include "../../headers/utils/gui_utils.h"

#include <imgui_internal.h>
#include <IconsLucide.h>

namespace ex::gui {

    inline int window_reset_flags(const bool reset, const bool standalone) {
        return reset ? (ImGuiWindowFlags_NoSavedSettings | (standalone ? ImGuiWindowFlags_AlwaysAutoResize : 0)) : 0;
    }

    bool begin_docked_window(const char *name, bool *p_open, const bool reset, const bool standalone,
                             const ImGuiWindowFlags window_flags) {
        return ImGui::Begin(name, p_open,
                            window_reset_flags(reset, standalone) | ImGuiWindowFlags_NoCollapse |
                                    ImGuiWindowFlags_NoFocusOnAppearing | window_flags);
    }

    void next_no_close() {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        ImGui::SetNextWindowClass(&window_class);
    }

    void next_auto_hide(const bool header) {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_None  | ImGuiDockNodeFlags_AutoHideTabBar;
        window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoCloseButton;
        if (!header)
            window_class.DockNodeFlagsOverrideSet |= ImGuiDockNodeFlags_NoTabBar;
        ImGui::SetNextWindowClass(&window_class);
    }

    bool window_withing_dock(const ImGuiID &id) {
        auto node = ImGui::GetWindowDockNode();
        while (true) {
            if (node == nullptr)
                return false;
            if (node->ID == id)
                return true;
            node = node->ParentNode;
        }
    }

    bool window_is_docked() {
        return ImGui::GetWindowDockID() != 0;
    }

    bool window_is_multi_docked() {
        const auto node = ImGui::GetWindowDockNode();
        return node != nullptr && node->Windows.size() > 1;
    }

    bool is_docking_empty(const ImGuiID &id) {
        const auto node = ImGui::DockBuilderGetNode(id);
        if (!node)
            return true;
        return (node->Windows.empty() && !node->ChildNodes[0] && !node->ChildNodes[1]);
    }

    bool plain_close_button() {
        ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(ICON_LC_X).x - ImGui::GetStyle().ItemSpacing.x);
        const auto closed = ex::gui::plain_button(ICON_LC_X);
        ex::gui::plain_tooltip("Close");
        return closed;
    }

    bool plain_button(const char *label, const bool &state) {
        bool new_state = state;
        const int v = state ? 255 : 0;
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(v, v, v, 255));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));

        if (ImGui::Button(label)) {
            new_state = !state;
        }

        ImGui::PopStyleColor(2);

        return new_state;
    }

    bool plain_button(const char *label) {
        return plain_button(label, false);
    }

    bool toggle_button(const char *label, const bool &state) {
        bool new_state = state;
        const int v = state ? 255 : 0;

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(v, v, v, 255));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, state ? 127 : 0));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 11.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

        if (ImGui::Button(label)) {
            new_state = !state;
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

        return new_state;
    }

    bool toggle_button(const char *label) {
        return toggle_button(label, false);
    }

    bool plain_tooltip(const char *label) {
        if (ImGui::IsItemHovered()) {
            ImGui::BeginTooltip();
            ImGui::Text("%s", label);
            ImGui::EndTooltip();
            return true;
        }
        return false;
    }

    ImVec2 brackets_start(const float indent) {
        ImGui::Indent(indent <= 0 ? 7.f : indent);
        const auto pos = ImGui::GetCursorScreenPos();
        ImGui::Indent(13.f);
        return pos;
    }

    void brackets_end(const ImVec2 &start, const float indent, const float alpha) {
        const auto color = IM_COL32(0, 0, 0, 150 * alpha);

        constexpr float thickness = 1.25f;
        constexpr float width     = 2.8f;
        constexpr float height    = 2.8f;
        constexpr float offset    = 8.f;

        const ImVec2 inner_end = ImGui::GetCursorScreenPos();

        const auto p1 = ImVec2(start.x, start.y + 2.f);
        const auto p2 = ImVec2(start.x, inner_end.y - 6.f);

        const auto p3 = ImVec2(p1.x + 7.f, p1.y);
        const auto p4 = ImVec2(p2.x + 7.f, p2.y);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(p1, p2, color, thickness);
        draw_list->AddLine(p1, p3, color, thickness);
        draw_list->AddLine(p2, p4, color, thickness);

        const auto p5 = ImVec2(p1.x - 4.f, p2.y - 0.f);
        const auto p6 = ImVec2(p5.x - 6.f, p2.y - 0.f);
        const auto p7 = ImVec2(p6.x, p6.y - offset + height);

        draw_list->AddLine(p5, p6, color, thickness);
        draw_list->AddLine(p6, p7, color, thickness);

        const auto p0 = ImVec2(p7.x + 0.5, p7.y - height);
        const auto p8 = ImVec2(p0.x - width, p0.y + height);
        const auto p9 = ImVec2(p0.x + width, p0.y + height);
        draw_list->AddTriangleFilled(p0, p8, p9, color);

        ImGui::Unindent(13.f);
        ImGui::Unindent(indent <= 0 ? 7.f : indent);
    }

    void brackets_end_half(const ImVec2 &start, const float indent, const float alpha) {
        const auto color_1 = IM_COL32(0, 0, 0, 150 * alpha);

        const float off = indent <= 0 ? 13.f : indent;

        constexpr float thickness = 1.25f;
        constexpr float width     = 3.f;
        constexpr float height    = 3.f;
        constexpr float offset    = 1.f;

        const ImVec2 inner_end = ImGui::GetCursorScreenPos();

        const auto p2 = ImVec2(start.x - off, inner_end.y - 6.f);
        const auto p4 = ImVec2(p2.x + 7.f, p2.y);
        const auto p1 = ImVec2(start.x - off, start.y + offset + height);

        ImDrawList *draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(p4, p2, color_1, thickness);
        draw_list->AddLine(p2, p1, color_1, thickness);

        const auto p7 = ImVec2(p1.x + 0.5f, p1.y - height);
        const auto p5 = ImVec2(p7.x - width, p1.y);
        const auto p6 = ImVec2(p7.x + width, p1.y);

        draw_list->AddTriangleFilled(p7, p5, p6, color_1);
    }

    bool generic_node(const char *name, const bool bullet, const bool selected, const bool nested) {
        constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);

        auto c_bullet = nested ? 0 : tree_bull_flags;
        c_bullet |= (bullet && !nested) ? ImGuiTreeNodeFlags_Bullet : 0;

        if (!nested) ImGui::Unindent();
        const bool open = ImGui::TreeNodeEx(name, c_flag | c_bullet);
        if (!nested) ImGui::Indent();
        return open;
    }

    bool plain_node(const char *name, const bool bullet, const bool selected, const bool nested) {
        constexpr auto tree_bull_flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
        constexpr auto tree_node_flags = ImGuiTreeNodeFlags_SpanAllColumns;
        const auto c_flag = tree_node_flags | (selected ? ImGuiTreeNodeFlags_Selected : 0);

        auto c_bullet = nested ? 0 : tree_bull_flags;
        c_bullet |= (bullet && !nested) ? ImGuiTreeNodeFlags_Bullet : 0;

        const bool open = ImGui::TreeNodeEx(name, c_flag | c_bullet);
        return open;
    }

    bool show_yes_no_dialog_(const char *title, const char *msg, const char *yes, const char *no) {
        bool result = false;

        const auto offset     = ImGui::GetStyle().FramePadding.x * 2 + ImGui::GetStyle().ItemSpacing.x * 2;
        const auto title_size = ImGui::CalcTextSize(msg).x + offset * 4;

        const auto center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(title_size, 0));
        if (ImGui::BeginPopupModal(title, nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

            if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                return false;
            }

            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Text(" %s", msg);
            ImGui::Spacing();
            ImGui::Spacing();
            ImGui::Spacing();

            const float ok_size = ImGui::CalcTextSize(yes).x + offset;
            const float no_size = ImGui::CalcTextSize(no).x + offset;
            const float free_w  = ImGui::GetContentRegionMax().x;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;

            ImGui::SetCursorPosX(free_w - (ok_size + no_size + spacing));

            if (ImGui::Button(yes, ImVec2(ok_size, 0))) {
                ImGui::CloseCurrentPopup();
                result = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(no, ImVec2(no_size, 0))) {
                ImGui::CloseCurrentPopup();
                result = false;
            }

            ImGui::EndPopup();
        }

        return result;
    }

}
