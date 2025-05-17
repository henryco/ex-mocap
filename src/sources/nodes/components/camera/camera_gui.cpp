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

    namespace {
        const char *buffering_strings[] = {
            strings::node_component_camera_buffering_0,
            strings::node_component_camera_buffering_1,
            strings::node_component_camera_buffering_2,
            strings::node_component_camera_buffering_3,
        };

        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_CAMERA, strings::node_component_camera_name);
            return out_name;
        }

        void parcel_free_(GuiNodeParcel *parcel) {
            if (parcel == nullptr)
                return;
            if (parcel->data != nullptr)
                ::free(parcel->data);
            ::free(parcel);
        }

        void push_fps_color(const float fps) {
            if (static_cast<int>(fps) > 25) {
                ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 127, 0, 127));
                return;
            }
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(127, 0, 0, 127));
        }

        void pop_fps_color() {
            ImGui::PopStyleColor();
        }
    }

    void update_tree_node(const TreeNode *o, TreeNode *n) {
        if (!(o->state >>= node_state::disabled) &&
            (n->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<ex::logic::node::camera::CameraNode *>(ex::logic::state::get_node(n->id_node)))
                base->close();
            return;
        }
        if ((o->state >>= node_state::disabled) &&
            !(n->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<ex::logic::node::camera::CameraNode *>(ex::logic::state::get_node(n->id_node)))
                base->open();
        }
    }

    unsigned long int get_node_type_id() {
        return ex::logic::node::camera::CameraNode::get_static_type_id();
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
        if (cam_node == nullptr || cam_node->get_node_id() != window->node->id_node)
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
                const auto &list = ex::logic::node::camera::CameraNode::list_camera_devices(closed);
                static char buff_full[192];
                for (int i = 0; i < list.size; ++i) {
                    const auto &item = list[i];
                    const bool is_selected = ::strcmp(device.device.uuid, item.uuid) == 0;

                    ::memset(buff_full, '\0', 192);
                    ::snprintf(buff_full, 191, "%s [%s]", item.name, item.uuid);
                    buff_full[191] = '\0';

                    ImGui::PushID(i);
                    if (ImGui::Selectable(buff_full, is_selected)) {
                        cam_node->close();
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
            const short state_buffering = state.buffering;
            ImGui::SeparatorText(strings::node_component_camera_buffering);

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);

            if (ImGui::BeginCombo("##cam_buffering", buffering_strings[state_buffering], ImGuiComboFlags_HeightRegular)) {

                for (int i = 0; i < IM_ARRAYSIZE(buffering_strings); ++i) {
                    const bool is_selected = state_buffering == i;
                    ImGui::PushID(i);

                    if (ImGui::Selectable(buffering_strings[i], is_selected)) {
                        cam_node->close();
                        cam_node->set_device_buff(i);
                        cam_node->open();
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
                static char buff_form[5];
                for (int i = 0; i < caps.size; ++i) {
                    const auto &item = caps[i];
                    const bool is_selected = mode.mode.uuid == item.uuid;

                    ::memset(buff_full, '\0', 192);
                    ::memset(buff_form, '\0', 5);
                    ::memcpy(buff_form, item.format_4cc, 4);
                    ::snprintf(buff_full, 191, "%s (%s) - %d fps [%d x %d]", item.description, buff_form,
                               item.refresh_rate, item.width, item.height);

                    buff_full[191] = '\0';

                    ImGui::PushID(i);
                    if (ImGui::Selectable(buff_full, is_selected)) {
                        cam_node->close();
                        cam_node->set_device_mode(item);
                        cam_node->open();
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
            constexpr int off = 10;
            static const float one_size = ImGui::CalcTextSize(strings::node_component_camera_reset_all).x + off;
            static const float two_size = ImGui::CalcTextSize(ICON_LC_REFRESH_CCW_DOT).x;

            const float total_w = ImGui::GetContentRegionAvail().x;
            const float pox = total_w - two_size;
            const float av_w = one_size < (pox - off) ? one_size : (pox - off);

            if (ImGui::Button(strings::node_component_camera_reset_all, ImVec2(av_w, ImGui::GetFrameHeight()))) {
                cam_node->reset_controls();
                cam_node->list_controls(true);
                prev_change.dt = -1;
                prev_change.nd = -1;
                prev_change.id = -1;
            }

            ImGui::SameLine();
            push_fps_color(cam_node->timings.frame_rate);
            ImGui::TextWrapped("%.4f FPS", cam_node->timings.frame_rate);
            pop_fps_color();

            ImGui::SameLine(pox);
            if (ImGui::Button(ICON_LC_REFRESH_CCW_DOT)) {
                cam_node->reset();
                prev_change.dt = -1;
                prev_change.nd = -1;
                prev_change.id = -1;
            }
            ex::gui::plain_tooltip(strings::node_component_camera_refresh);
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
                if (ImGui::IsItemHovered()) {
                    ImGui::BeginTooltip();
                    ImGui::Text("%d", item.id);
                    ImGui::EndTooltip();
                }

                constexpr float    button_off  = 5;
                static const float button_size = ImGui::CalcTextSize(ICON_LC_ROTATE_CCW).x;
                static const float button_pad  = button_size + button_off;

                const float total_w = ImGui::GetContentRegionAvail().x;

                { // VALUE SLIDER
                    const auto input_pos  = ImGui::GetCursorScreenPos();
                    const auto input_size = ImVec2(total_w - button_pad - button_off, ImGui::GetFrameHeight());
                    ImGui::SetNextItemWidth(input_size.x);

                    if (cam_node->get_node_id() == prev_change.nd && item.id == prev_change.id)
                        value = prev_change.val;

                    ImGui::SliderInt("##drag_value", &value, item.min, item.max);

                    if (value != item.value) {

                        if (cam_node->get_node_id() == prev_change.nd) {
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

                        prev_change.nd  = cam_node->get_node_id();
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
                    ex::gui::plain_tooltip(strings::node_component_camera_reset);

                    if (value == item.default_value)
                        ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }
        }

        return true;
    }

    GuiNodeParcel *store(const TreeNode *node) {
        auto *img_node = dynamic_cast<logic::node::camera::CameraNode *>(ex::logic::state::get_node(node->id_node));
        if (img_node == nullptr)
            return nullptr;

        io::byte_array data = img_node->serialize();

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
        if (auto *cam_node = dynamic_cast<logic::node::camera::CameraNode *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            cam_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (auto *cam_node = dynamic_cast<logic::node::camera::CameraNode *>(instance)) {
            ex::logic::state::store_node(node->id_node, instance);

            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            cam_node->deserialize(byte_data);
            return;
        }

        abort_("Invalid node mapper type");
    }
}

namespace ex::gui::node::sync {

    namespace {
        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_COMBINE, strings::node_component_sync_name);
            return out_name;
        }
    }

    unsigned long int get_node_type_id() {
        return ex::logic::node::sync::SyncNode::get_static_type_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_COMBINE, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    const char *get_node_name() {
        return strings::node_component_sync_name;
    }

    void update_tree_node(const TreeNode *o, TreeNode *n) {
        if (!(o->state >>= node_state::disabled) &&
            (n->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<ex::logic::node::sync::SyncNode *>(ex::logic::state::get_node(n->id_node)))
                base->disable();
            return;
        }
        if ((o->state >>= node_state::disabled) &&
            !(n->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<ex::logic::node::sync::SyncNode *>(ex::logic::state::get_node(n->id_node)))
                base->enable();
        }
    }

}