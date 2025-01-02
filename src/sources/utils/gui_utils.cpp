//
// Created by xd on 27/11/24.
//

#include "../../headers/utils/gui_utils.h"

#include <imgui_internal.h>
#include <IconsLucide.h>

namespace ex::gui {

    void next_no_close() {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        ImGui::SetNextWindowClass(&window_class);
    }

    void next_auto_hide(bool header) {
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

    void brackets_end(const ImVec2 &start, const float indent) {
        constexpr auto color = IM_COL32(0, 0, 0, 120);
        constexpr auto thickness = 1.2f;

        const ImVec2 inner_end = ImGui::GetCursorScreenPos();

        const auto p1 = ImVec2(start.x, start.y + 2.f);
        const auto p2 = ImVec2(start.x, inner_end.y - 6.f);

        const auto p3 = ImVec2(p1.x + 5.f, p1.y);
        const auto p4 = ImVec2(p2.x + 5.f, p2.y);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(p1, p2, color, thickness);
        draw_list->AddLine(p1, p3, color, thickness);
        draw_list->AddLine(p2, p4, color, thickness);

        ImGui::Unindent(13.f);
        ImGui::Unindent(indent <= 0 ? 7.f : indent);
    }

    void brackets_end_half(const ImVec2 &start, const float indent) {
        const float off = indent <= 0 ? 13.f : indent;

        constexpr auto color = IM_COL32(0, 0, 0, 120);
        constexpr auto thickness = 1.2f;

        const ImVec2 inner_end = ImGui::GetCursorScreenPos();

        const auto p1 = ImVec2(start.x - off, start.y + 2.f);
        const auto p2 = ImVec2(start.x - off, inner_end.y - 6.f);

        const auto p4 = ImVec2(p2.x + 5.f, p2.y);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(p1, p2, color, thickness);
        draw_list->AddLine(p2, p4, color, thickness);
    }

} // namespace ex::gui
