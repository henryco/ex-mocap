//
// Created by xd on 29/05/25.
//


#include "../../../../headers/nodes/components/blur/blur_gui.h"
#include "../../../../headers/nodes/components/blur/blur_logic.h"

#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include "../../../../../extra/data/struct/time_stack.h"

#include <IconsLucide.h>
#include "imgui.h"


namespace ex::gui::node::blur {

    namespace {
        namespace lnb = logic::node::blur;
        using blur_node_t = lnb::BlurNode;

        void fn_apply(const unsigned node, const lnb::config_t &data) {
            if (blur_node_t *instance = dynamic_cast<blur_node_t *>(logic::state::get_node(node)))
                instance->set_config(data);
        }

        // up to 1KB debounce queue
        data::time_stack<23, id_type_32, lnb::config_t, &fn_apply> time_stack;

        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_WAVES, strings::node_component_blur_name);
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
        return blur_node_t::get_static_type_id();
    }

    const char *get_node_name() {
        return strings::node_component_blur_name;
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_WAVES, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const id_type_32 node_id = window->node->id_node;

        auto *base = dynamic_cast<blur_node_t *>(logic::state::get_node(node_id));
        if (base == nullptr || base->get_node_id() != node_id)
            return true;

        const double time_now = ImGui::GetTime();

        lnb::config_t config;
        if (const lnb::config_t *state = time_stack.retrieve_consume(node_id, time_now, 1.f))
            ::memcpy(&config, state, sizeof(lnb::config_t));
        else
            ::memcpy(&config, &base->config, sizeof(lnb::config_t));


        static const ex::State &state = ex::logic::globals::state();

        {
            ImGui::SeparatorText(strings::node_component_blur_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", base->get_image_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            ImGui::BeginDisabled(true);
            if (ImGui::InputText("###BlurId", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                base->set_image_id(::atoi(buff));
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
            ImGui::TextDisabled("Image  W: %u", base->get_image_w());
            ImGui::TextDisabled("Image  H: %u", base->get_image_h());
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_blur_kernel, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            {// Kernel half-size
                static char buff[9] = "";
                ::sprintf(buff, "%hhu", config.kernel_size);
                if (ImGui::InputText(strings::node_component_blur_kernel_size, buff, 9, ImGuiInputTextFlags_CharsDecimal)) {
                    const int value    = ::atoi(buff);
                    config.kernel_size = value;
                    if (value < 0)
                        config.kernel_size = 0;
                    if (value > 20)
                        config.kernel_size = 20;
                    time_stack.push(time_now, node_id, config);
                }
            }

            {// Sigma
                static char buff[9] = "";
                ::sprintf(buff, "%f", config.sigma);
                if (ImGui::InputText(strings::node_component_blur_kernel_sigma, buff, 9, ImGuiInputTextFlags_CharsScientific)) {
                    const double value = ::atof(buff);
                    config.sigma = value;
                    if (value <= 0)
                        config.sigma = 1.f;
                    time_stack.push(time_now, node_id, config);
                }
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_blur_mode, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            // Single pass
            if (ImGui::Checkbox(strings::node_component_blur_mode_single, &config.single_pass))
                time_stack.push(time_now, node_id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        return true;
    }

    GuiNodeParcel *store(const TreeNode *node) {

        const blur_node_t *base = dynamic_cast<blur_node_t *>(ex::logic::state::get_node(node->id_node));
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

        if (auto *trs_node = dynamic_cast<blur_node_t *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (auto *trs_node = dynamic_cast<blur_node_t *>(instance)) {
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
            if (const auto base = dynamic_cast<blur_node_t *>(logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }
        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<blur_node_t *>(logic::state::get_node(after->id_node)))
                base->enable();
        }
    }
}