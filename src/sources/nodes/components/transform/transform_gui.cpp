//
// Created by xd on 18/05/25.
//

#include "../../../../headers/nodes/components/transform/transform_gui.h"
#include "../../../../headers/nodes/components/transform/transform_logic.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>
#include "imgui.h"

namespace ex::gui::node::transform {

    namespace {
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
        namespace lnt  = logic::node::transform;
        auto *trs_node = dynamic_cast<lnt::TransformNode *>(
                ex::logic::state::get_node(window->node->id_node));
        if (trs_node == nullptr || trs_node->get_node_id() != window->node->id_node)
            return true;

        static struct {
            lnt::config_t config;
            unsigned      node;
            double        dt;
        } dt_state; // TODO

        {
            dt_state.config = trs_node->config;
        }

        {
            ImGui::SeparatorText(strings::node_component_transform_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", trs_node->get_image_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            if (ImGui::InputText("###TransformId", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                trs_node->set_image_id(::atoi(buff));
            }

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
        }

        ImGui::NewLine();
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Region", ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));
            { // x0
                static char buff[8] = "";
                if (ImGui::InputText("x0##r_xo", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }

            { // y0
                static char buff[8] = "";
                if (ImGui::InputText("y0##r_y0", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }

            { // w
                static char buff[8] = "";
                if (ImGui::InputText("width##r_w", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }

            { // h
                static char buff[8] = "";
                if (ImGui::InputText("height##r_h", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }
            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Resize", ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));
            { // w
                static char buff[8] = "";
                if (ImGui::InputText("width##e_w", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }

            { // h
                static char buff[8] = "";
                if (ImGui::InputText("height##e_h", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {

                }
            }

            ImGui::Checkbox("Linear interpolation", &dt_state.config.resize.linear);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Mirror", ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            ImGui::Checkbox("Horizontally", &dt_state.config.flip.flip_h);

            ImGui::Checkbox("Vertically", &dt_state.config.flip.flip_v);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader("Rotate", ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            int op = static_cast<int>(dt_state.config.rotate);

            ImGui::RadioButton("None", &op, static_cast<int>(lnt::rotate_e::NONE));
            ImGui::RadioButton("Clockwise 90º", &op, static_cast<int>(lnt::rotate_e::CW_1));

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
