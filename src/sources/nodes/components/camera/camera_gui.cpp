//
// Created by xd on 4/30/25.
//

#include "../../../../headers/nodes/components/camera/camera_gui.h"
#include "../../../../headers/nodes/components/camera/camera_logic.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>
#include "imgui.h"

namespace ex::gui::node::camera {

    static const char *prev_name_() {
        static char out_name[128];
        sprintf(out_name, "%s %s", ICON_LC_CAMERA, strings::node_component_camera_name);
        return out_name;
    }

    void update_tree_node(const TreeNode *, TreeNode *) {
    }

    unsigned long int get_node_type_id() {
        return ex::logic::node::camera::CameraNode::get_node_id();
    }

    const char *get_node_name() {
        return strings::node_component_camera_name;
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_CAMERA, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    bool draw_node_window(GuiNodeWindow *window) {

        auto *cam_node = dynamic_cast<ex::logic::node::camera::CameraNode *>(ex::logic::state::get_node(window->node->id_node));
        if (cam_node == nullptr || cam_node->get_id() != window->node->id_node)
            return true;

        const auto &state = cam_node->state;

        static struct {
            double dt  = -1;
            uint   nd  = -1;
            uint   id  = -1;
            int    val = -1;
        } prev_change;

        {
            ImGui::SeparatorText(strings::node_component_camera_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", state.image_id);

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            if (ImGui::InputText("##CameraID", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                cam_node->set_image_id(::atoi(buff));
            }

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
        }

        ImGui::NewLine();

        {
            static bool closed = true;
            const auto &device = state.device;

            ImGui::SeparatorText(strings::node_component_camera_device);

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);

            if (ImGui::BeginCombo("##cam_device_list", device.device.name, ImGuiComboFlags_HeightRegular)) {
                const auto &list = ex::logic::node::camera::list_camera_devices(closed);
                static char buff_full[192];
                for (int i = 0; i < list.size; ++i) {
                    const auto &item = list[i];
                    const bool is_selected = ::strcmp(device.device.uuid, item.uuid) == 0;

                    ::memset(buff_full, '\0', 192);
                    ::snprintf(buff_full, 191, "%s [%s]", item.name, item.uuid);
                    buff_full[191] = '\0';

                    ImGui::PushID(i);
                    if (ImGui::Selectable(buff_full, is_selected)) {
                        cam_node->set_device(item);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }

                ImGui::EndCombo();

                closed = false;
            } else {
                closed = true;
            }

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));

            if (!state.device.selected)
                return true;
        }

        ImGui::NewLine();

        {
            const auto &mode = state.mode;

            ImGui::SeparatorText(strings::node_component_camera_mode);

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);

            if (ImGui::BeginCombo("##cam_cap_list", mode.description, ImGuiComboFlags_HeightRegular)) {

                const auto &caps = cam_node->list_capabilities();

                static char buff_full[192];
                for (int i = 0; i < caps.size; ++i) {
                    const auto &item = caps[i];
                    const bool is_selected = mode.mode.uuid == item.uuid;

                    ::memset(buff_full, '\0', 192);
                    ::snprintf(buff_full, 191, "%s (%s) - %d fps [%d x %d]", item.description, item.format_4cc,
                               item.refresh_rate, item.width, item.height);

                    buff_full[191] = '\0';

                    ImGui::PushID(i);
                    if (ImGui::Selectable(buff_full, is_selected)) {
                        cam_node->set_device_mode(item);
                    }

                    if (is_selected)
                        ImGui::SetItemDefaultFocus();
                    ImGui::PopID();
                }

                ImGui::EndCombo();
            }

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));

            if (!state.mode.selected)
                return true;
        }

        ImGui::NewLine();

        {
            if (ImGui::Button(strings::node_component_camera_reset)) {
                cam_node->reset_controls();
                cam_node->list_controls(true);
                prev_change.dt = -1;
                prev_change.nd = -1;
                prev_change.id = -1;
            }
        }

        ImGui::NewLine();

        {
            if (prev_change.id != -1 && prev_change.dt != -1) {
                const double dt = ImGui::GetTime() - prev_change.dt;
                if (dt >= 0.25) {
                    cam_node->set_control(prev_change.id, prev_change.val);
                    cam_node->list_controls(true);
                    prev_change.dt = -1;
                    prev_change.nd = -1;
                    prev_change.id = -1;
                }
            }

            const auto &controls = cam_node->list_controls();
            for (int i = 0; i < controls.size; ++i) {
                const auto &item = controls[i];
                if (item.step == 0 || item.min == item.max)
                    continue;

                int value = item.value;

                ImGui::NewLine();

                ImGui::PushID(i);
                ImGui::SeparatorText(item.name);

                constexpr float    button_off  = 5;
                static const float button_size = ImGui::CalcTextSize(ICON_LC_ROTATE_CCW).x;
                static const float button_pad  = button_size + button_off;

                const float total_w = ImGui::GetContentRegionAvail().x;

                { // VALUE SLIDER
                    const auto input_pos  = ImGui::GetCursorScreenPos();
                    const auto input_size = ImVec2(total_w - button_pad - button_off, ImGui::GetFrameHeight());
                    ImGui::SetNextItemWidth(input_size.x);

                    if (cam_node->get_id() == prev_change.nd && item.id == prev_change.id)
                        value = prev_change.val;

                    ImGui::SliderInt("##drag_value", &value, item.min, item.max);

                    if (value != item.value) {

                        if (cam_node->get_id() == prev_change.nd) {
                            if (item.id != prev_change.id) {
                                cam_node->set_control(prev_change.id, prev_change.val);
                                cam_node->list_controls(true);
                                prev_change.dt = ImGui::GetTime();
                            }

                            else if (value != prev_change.val) {
                                prev_change.dt = ImGui::GetTime();
                            }
                        }
                        else {
                            prev_change.dt = ImGui::GetTime();
                        }

                        prev_change.nd  = cam_node->get_id();
                        prev_change.id  = item.id;
                        prev_change.val = value;
                    }

                    constexpr float alpha = 1.f;
                    ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                        ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                        IM_COL32(0, 0, 0, 90 * alpha));
                }

                { // RESET BUTTON
                    const float pox = total_w - button_pad + button_off;
                    ImGui::SameLine(pox);

                    if (value == item.default_value)
                        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));

                    if (ImGui::Button(ICON_LC_ROTATE_CCW)) {
                        cam_node->set_control(item.id, item.default_value);
                        cam_node->list_controls(true);
                        prev_change.dt = -1;
                        prev_change.nd = -1;
                        prev_change.id = -1;
                    }

                    if (value == item.default_value)
                        ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }
        }

        return true;
    }

    GuiNodeParcel *store(const TreeNode *node) {
        // TODO
        return nullptr;
    }

    void restore(TreeNode *node, GuiNodeParcel *parcel) {
        if (parcel == nullptr)
            return;
        // TODO
    }
}