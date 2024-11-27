//
// Created by xd on 27/11/24.
//

#include "../../headers/utils/gui_utils.h"
#include "imgui.h"

namespace ex::gui {

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
