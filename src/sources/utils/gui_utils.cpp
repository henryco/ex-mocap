//
// Created by xd on 27/11/24.
//

#include "../../headers/utils/gui_utils.h"

#include <imgui_internal.h>
#include <IconsLucide.h>

namespace ex::gui {

    void next_auto_hide(bool header) {
        ImGuiWindowClass window_class;
        window_class.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_None | ImGuiDockNodeFlags_AutoHideTabBar;
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
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 7.0f);
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
}
