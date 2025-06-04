//
// Created by xd on 04/06/25.
//

#include "../../../../headers/nodes/components/calibration/calibration_gui.h"
#include "../../../../headers/nodes/components/calibration/calibration_logic.h"

#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include "../../../../../extra/data/struct/time_stack.h"

#include <IconsLucide.h>
#include "imgui.h"

namespace ex::gui::node::calibration {

    namespace {
        namespace lnc = logic::node::calibration;
        using calib_node_t = lnc::CalibNode;

        void fn_apply(const unsigned node, const lnc::config_t &data) {
            if (calib_node_t *instance = dynamic_cast<calib_node_t *>(logic::state::get_node(node)))
                instance->set_config(data);
        }

        // up to 1KB debounce queue
        data::time_stack<23, id_type_32, lnc::config_t, &fn_apply> time_stack;

        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_APERTURE, strings::node_component_calibration_name);
            return out_name;
        }

        void parcel_free_(GuiNodeParcel *parcel) {
            if (parcel == nullptr)
                return;
            if (parcel->data != nullptr)
                ::free(parcel->data);
            ::free(parcel);
        }
    }

    unsigned long int get_node_type_id() {
        return calib_node_t::get_static_type_id();
    }

    const char *get_node_name() {
        return strings::node_component_calibration_name;
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_APERTURE, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const id_type_32 node_id = window->node->id_node;

        auto *base = dynamic_cast<calib_node_t *>(logic::state::get_node(node_id));
        if (base == nullptr || base->get_node_id() != node_id)
            return true;

        const double time_now = ImGui::GetTime();

        lnc::config_t config;
        if (const lnc::config_t *state = time_stack.retrieve_consume(node_id, time_now, 1.f))
            ::memcpy(&config, state, sizeof(lnc::config_t));
        else
            ::memcpy(&config, &base->config, sizeof(lnc::config_t));


        static const ex::State &state = ex::logic::globals::state();

        {
            ImGui::SeparatorText(strings::node_component_calibration_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%lu", base->get_node_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            ImGui::BeginDisabled(true);
            if (ImGui::InputText("###CalibId", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
            }
            ImGui::EndDisabled();

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
            ImGui::NewLine();
        }

        {
            ImGui::TextDisabled("Input ID: %u", base->get_input_id());
            ImGui::TextDisabled("Image  W: %u", base->get_input_w());
            ImGui::TextDisabled("Image  H: %u", base->get_input_h());
            ImGui::NewLine();
        }

        // TODO

        return true;
    }

    GuiNodeParcel *store(const TreeNode *node) {

        const calib_node_t *base = dynamic_cast<calib_node_t *>(ex::logic::state::get_node(node->id_node));
        if (base == nullptr)
            return nullptr;

        io::byte_array data = base->serialize();

        GuiNodeParcel *parcel = static_cast<GuiNodeParcel *>(::malloc(sizeof(GuiNodeParcel)));
        parcel->free          = &parcel_free_;
        parcel->size          = data.size;
        parcel->data          = nullptr;

        if (!data.empty()) {
            parcel->data = static_cast<unsigned char *>(::malloc(data.size));
            ::memcpy(parcel->data, data.data, data.size);
        }

        io::byte_array::free(data);
        return parcel;
    }

    void restore(TreeNode *node, GuiNodeParcel *parcel) {
        if (parcel == nullptr)
            return;

        if (auto *trs_node = dynamic_cast<calib_node_t *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (auto *trs_node = dynamic_cast<calib_node_t *>(instance)) {
            ex::logic::state::store_node(node->id_node, instance);

            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        abort_("Invalid node mapper type");
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        if (!(before->state >>= node_state::disabled) &&
            (after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<calib_node_t *>(logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }
        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<calib_node_t *>(logic::state::get_node(after->id_node)))
                base->enable();
        }
    }

}