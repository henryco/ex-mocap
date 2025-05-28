//
// Created by xd on 18/05/25.
//

#include "../../../../../extra/data/struct/time_stack.h"
#include "../../../../headers/nodes/components/transform/transform_gui.h"
#include "../../../../headers/nodes/components/transform/transform_logic.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include "imgui.h"

namespace ex::gui::node::transform {

    namespace {
        namespace lnt = logic::node::transform;

        void fn_apply(const unsigned node, const lnt::config_t &data) {
            if (auto *instance = dynamic_cast<lnt::TransformNode *>(logic::state::get_node(node)))
                instance->set_config(data);
        }

        // 1KB debounce queue
        data::time_stack<23, id_type_32, lnt::config_t, &fn_apply> time_stack;

        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_SCALING, strings::node_component_transform_name);
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
        return ex::logic::node::transform::TransformNode::get_static_type_id();
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    const char *get_node_name() {
        return strings::node_component_transform_name;
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_SCALING, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const id_type_32 node_id = window->node->id_node;

        auto *trs_node = dynamic_cast<lnt::TransformNode *>(logic::state::get_node(node_id));
        if (trs_node == nullptr || trs_node->get_node_id() != node_id)
            return true;

        const double time_now = ImGui::GetTime();

        lnt::config_t config;
        if (const lnt::config_t *state = time_stack.retrieve_consume(node_id, time_now, 1.f))
            ::memcpy(&config, state, sizeof(lnt::config_t));
        else
            ::memcpy(&config, &trs_node->config, sizeof(lnt::config_t));


        static const ex::State &state = ex::logic::globals::state();

        {
            ImGui::SeparatorText(strings::node_component_transform_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", trs_node->get_image_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            ImGui::BeginDisabled(true);
            if (ImGui::InputText("###TransformId", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                trs_node->set_image_id(::atoi(buff));
            }
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
            ImGui::TextDisabled("Output W: %u", trs_node->get_output_w());
            ImGui::TextDisabled("Output H: %u", trs_node->get_output_h());
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_transform_region, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            { // x0
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.crop.start_x);
                if (ImGui::InputText(strings::node_component_transform_region_x0, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                    const int value     = ::atoi(buff);
                    config.crop.start_x = value;
                    if (value < 0 || value >= trs_node->get_input_w())
                        config.crop.start_x = 0;
                    time_stack.push(time_now, node_id, config);
                }
            }

            { // y0
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.crop.start_y);
                if (ImGui::InputText(strings::node_component_transform_region_y0, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                    const int value     = ::atoi(buff);
                    config.crop.start_y = value;
                    if (value < 0 || value >= trs_node->get_input_h())
                        config.crop.start_y = 0;
                    time_stack.push(time_now, node_id, config);
                }
            }

            { // w
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.crop.end_x > 0 ? config.crop.end_x : trs_node->get_input_w());
                if (ImGui::InputText(strings::node_component_transform_region_x1, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                    const int value   = ::atoi(buff);
                    config.crop.end_x = value;

                    if (value < 0 || value < config.crop.start_x)
                        config.crop.end_x = 0;

                    if (config.crop.end_x > trs_node->get_input_w())
                        config.crop.end_x = 0;

                    time_stack.push(time_now, node_id, config);
                }
            }

            { // h
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.crop.end_y > 0 ? config.crop.end_y : trs_node->get_input_h());
                if (ImGui::InputText(strings::node_component_transform_region_y1, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                    const int value   = ::atoi(buff);
                    config.crop.end_y = value;

                    if (value < 0 || value < config.crop.start_y)
                        config.crop.end_y = 0;

                    if (config.crop.end_y > trs_node->get_input_h())
                        config.crop.end_y = 0;

                    time_stack.push(time_now, node_id, config);
                }
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_transform_resize, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            { // w
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.resize.width > 0 ? config.resize.width : trs_node->get_input_w());
                if (ImGui::InputText(strings::node_component_transform_resize_w, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                    const int value     = ::atoi(buff);
                    config.resize.width = value;

                    if (value < 0)
                        config.resize.width = 0;

                    if (config.resize.width > state.cl.tex_cap_w)
                        config.resize.width = 0;

                    if (config.resize.fixed && config.resize.width) {
                        const float ratio_w_to_h = static_cast<float>(trs_node->get_input_w()) / static_cast<float>(trs_node->get_input_h());
                        config.resize.height = static_cast<float>(config.resize.width) / ratio_w_to_h;
                    }

                    else if (config.resize.fixed) {
                        config.resize.height = 0;
                    }

                    time_stack.push(time_now, node_id, config);
                }
            }

            { // h
                static char buff[8] = "";
                ::sprintf(buff, "%hu", config.resize.height > 0 ? config.resize.height : trs_node->get_input_h());
                if (ImGui::InputText(strings::node_component_transform_resize_h, buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                    const int value      = ::atoi(buff);
                    config.resize.height = value;

                    if (value < 0)
                        config.resize.height = 0;

                    if (config.resize.height > state.cl.tex_cap_h)
                        config.resize.height = 0;

                    if (config.resize.fixed && config.resize.height) {
                        const float ratio_w_to_h = static_cast<float>(trs_node->get_input_w()) / static_cast<float>(trs_node->get_input_h());
                        config.resize.width = static_cast<float>(config.resize.height) * ratio_w_to_h;
                    }

                    else if (config.resize.fixed) {
                        config.resize.width = 0;
                    }

                    time_stack.push(time_now, node_id, config);
                }
            }

            if (ImGui::Checkbox(strings::node_component_transform_resize_p, &config.resize.fixed))
                time_stack.push(time_now, node_id, config);

            if (ImGui::Checkbox(strings::node_component_transform_resize_l, &config.resize.linear))
                time_stack.push(time_now, node_id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_transform_mirror, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            if (ImGui::Checkbox(strings::node_component_transform_mirror_h, &config.flip.flip_h))
                time_stack.push(time_now, node_id, config);

            if (ImGui::Checkbox(strings::node_component_transform_mirror_v, &config.flip.flip_v))
                time_stack.push(time_now, node_id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_transform_rotate, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            int op = static_cast<int>(config.rotate);

            if (ImGui::RadioButton(strings::node_component_transform_rotate_0, &op, static_cast<int>(lnt::rotate_e::NONE))) {
                config.rotate = static_cast<lnt::rotate_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            if (ImGui::RadioButton(strings::node_component_transform_rotate_90, &op, static_cast<int>(lnt::rotate_e::CW_1))) {
                config.rotate = static_cast<lnt::rotate_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        return true;
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        if (!(before->state >>= node_state::disabled) &&
            (after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<logic::node::transform::TransformNode *>(logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }

        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<logic::node::transform::TransformNode *>(logic::state::get_node(after->id_node)))
                base->enable();
        }
    }

    GuiNodeParcel *store(const TreeNode *node) {
        auto *trs_node = dynamic_cast<logic::node::transform::TransformNode *>(ex::logic::state::get_node(node->id_node));
        if (trs_node == nullptr)
            return nullptr;

        io::byte_array data = trs_node->serialize();

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

        if (auto *trs_node = dynamic_cast<logic::node::transform::TransformNode *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (auto *trs_node = dynamic_cast<logic::node::transform::TransformNode *>(instance)) {
            ex::logic::state::store_node(node->id_node, instance);

            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        abort_("Invalid node mapper type");
    }
}
