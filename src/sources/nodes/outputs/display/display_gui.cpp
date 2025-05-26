//
// Created by xd on 2/25/25.
//

#include "../../../../../extra/data/struct/time_stack.h"
#include "../../../../headers/nodes/outputs/display/display_logic.h"
#include "../../../../headers/nodes/outputs/display/display_gui.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"
#include "imgui.h"

#include <IconsLucide.h>

namespace ex::gui::node::out::display {

    namespace elno = ex::logic::node::out;

    namespace {
        struct config_t {
            bool direct;
        };

        void fn_apply(const unsigned node, const config_t &data) {
            if (auto *instance = dynamic_cast<elno::DisplayOutputNode *>(logic::state::get_node(node)))
                instance->set_direct(data.direct);
        }

        // debounce queue
        data::time_stack<32, id_type_32, config_t, &fn_apply> time_stack;
    }


    unsigned long int get_node_type_id() {
        return elno::DisplayOutputNode::get_static_type_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_PICTURE_IN_PICTURE_2, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        if (before == nullptr || after == nullptr)
            return;

        if (!(before->state >>= node_state::disabled) &&
            (after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }

        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node)))
                base->enable();
            return;
        }

        if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node))) {
            base->rename_image(after->name);
        }
    }

    bool draw_node_window(GuiNodeWindow *window) {

        const id_type_32 id = window->node->id_node;
        auto *trs_node = dynamic_cast<elno::DisplayOutputNode *>(logic::state::get_node(id));
        if (trs_node == nullptr || trs_node->get_node_id() != id)
            return true;

        const double time_now = ImGui::GetTime();

        config_t config;
        if (const config_t *state = time_stack.retrieve_consume(id, time_now, .5))
            ::memcpy(&config, state, sizeof(config_t));
        else {
            ::memset(&config, 0, sizeof(config_t));
            config.direct = trs_node->is_direct();
        }

        {
            ImGui::SeparatorText(strings::node_output_display_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%lu", trs_node->get_node_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            ImGui::BeginDisabled(true);
            ImGui::InputText("###DisplayId", buff, 8, ImGuiInputTextFlags_CharsDecimal);
            ImGui::EndDisabled();

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
            ImGui::NewLine();
        }

        {
            ImGui::TextDisabled("Input ID: %u", trs_node->get_input_id());
            ImGui::TextDisabled("Input  W: %u", trs_node->get_input_w());
            ImGui::TextDisabled("Input  H: %u", trs_node->get_input_h());
            ImGui::NewLine();
        }

        {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            if (ImGui::Checkbox(strings::node_output_display_direct, &config.direct))
                time_stack.push(time_now, id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        return true;
    }

    const char *get_node_preview() {
        static char  out_name[128];
        static char *name = nullptr;
        if (name == nullptr) {
            name = out_name;
            sprintf(name, "%s %s", ICON_LC_PICTURE_IN_PICTURE_2, strings::node_output_display_name);
        }
        return name;
    }

    const char *get_node_name() {
        return strings::node_output_display_name;
    }

} // namespace ex::gui::node::out::display
