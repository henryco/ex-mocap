//
// Created by xd on 4/9/25.
//

#include "../../../../headers/nodes/components/image/image_gui.h"
#include "../../../../headers/nodes/components/image/image_logic.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>
#include "imgui.h"

namespace ex::gui::node::image {

    namespace imgn = ex::logic::node::image;

    namespace internal_ {

        constexpr auto EXTENSION_DEFAULT = "*.png";
        constexpr auto EXTENSION_JPEG    = "*.jpeg";
        constexpr auto EXTENSION_JPG     = "*.jpg";
        constexpr auto EXTENSION_BMP     = "*.bmp";
        constexpr auto EXTENSION_GIF     = "*.gif";
        constexpr auto EXTENSION_ALL     = "*";

        static const char *filters_[] = {
            EXTENSION_DEFAULT,
            EXTENSION_JPEG,
            EXTENSION_JPG,
            EXTENSION_BMP,
            EXTENSION_GIF,
            EXTENSION_ALL,
            nullptr
        };

        void show_open_file_picker_(bool *open, const char *def_path, imgn::ImageNode *img_node);

        void parcel_free_(GuiNodeParcel *parcel) {
            if (parcel == nullptr)
                return;
            if (parcel->data != nullptr)
                ::free(parcel->data);
            ::free(parcel);
        }
    }

    unsigned long int get_node_type_id() {
        return logic::node::image::ImageNode::get_static_type_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_IMAGE, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(const TreeNode *, TreeNode *) {
    }

    GuiNodeParcel *store(const TreeNode *node) {
        imgn::ImageNode *img_node = dynamic_cast<imgn::ImageNode *>(ex::logic::state::get_node(node->id_node));
        if (img_node == nullptr)
            return nullptr;

        io::byte_array data = img_node->serialize();

        GuiNodeParcel *parcel = static_cast<GuiNodeParcel *>(::malloc(sizeof(GuiNodeParcel)));
        parcel->free          = &internal_::parcel_free_;
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
        if (imgn::ImageNode *img_node = dynamic_cast<imgn::ImageNode *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            img_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (imgn::ImageNode *img_node = dynamic_cast<imgn::ImageNode *>(instance)) {
            ex::logic::state::store_node(node->id_node, instance);

            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            img_node->deserialize(byte_data);
            return;
        }

        abort_("Invalid node mapper type");
    }

    bool draw_node_window(GuiNodeWindow *window) {
        static ex::State &state = ex::logic::globals::state();

        imgn::ImageNode *img_node = dynamic_cast<imgn::ImageNode *>(ex::logic::state::get_node(window->node->id_node));
        if (img_node == nullptr || img_node->get_node_id() != window->node->id_node)
            return true;

        {
            ImGui::SeparatorText(strings::node_component_image_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", img_node->state.image_id);

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            if (ImGui::InputText("##ImageID", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                img_node->set_image_id(::atoi(buff));
            }

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
        }

        ImGui::NewLine();

        const char *source_file = nullptr;
        if (img_node->state.selected) {
            source_file = img_node->state.source;
            ImGui::SeparatorText(strings::node_component_image_source);
            ImGui::TextWrapped("%s", source_file);
        } else {
            source_file = state.project.project_path;
            ImGui::SeparatorText(strings::node_component_image_sourceless);
        }

        ImGui::NewLine();

        const ImVec2 text_size = ImGui::CalcTextSize(strings::node_component_image_select);
        const float pos_x = (ImGui::GetContentRegionAvail().x - text_size.x) * 0.5f;
        ImGui::SetCursorPosX(pos_x);

        static bool open_ = false;
        if (ImGui::Button(strings::node_component_image_select)) {
            open_ = true;
        }

        internal_::show_open_file_picker_(&open_, source_file, img_node);

        if (!img_node->state.selected)
            return true;

        return true;
    }

    const char *get_node_preview() {
        static char  out_name[128];
        static char *name = nullptr;
        if (name == nullptr) {
            name = out_name;
            sprintf(name, "%s %s", ICON_LC_IMAGE, strings::node_component_image_name);
        }
        return name;
    }

    const char *get_node_name() {
        return strings::node_component_image_name;
    }

    void internal_::show_open_file_picker_(bool *open, const char *def_path, imgn::ImageNode *img_node) {
        if (open == nullptr || !*open)
            return;

        constexpr auto min_size  = ImVec2(900, 500);
        static constexpr simpfp::Labels labels {
            .main_accept = strings::gui_dialogs_project_open_accept
        };

        if (!simpfp::FileDialogOpen())
            simpfp::OpenFileDialog(strings::gui_dialogs_project_open, def_path, filters_, &labels);

        if (simpfp::ShowFileDialog(strings::gui_dialogs_project_open, open, min_size, true)) {
            if (static char file[256]; simpfp::FileAccepted(file, sizeof(file))) {
                const auto path = std::filesystem::path(file);
                if (std::filesystem::exists(path) && std::filesystem::is_regular_file(path)) {
                    img_node->set_image(file);
                    *open = false;
                    simpfp::CloseFileDialog();
                } else {
                    ImGui::OpenPopup(strings::gui_dialogs_project_open_error);
                }

                show_alert_dialog(strings::gui_dialogs_alert_error, strings::gui_dialogs_project_open_error_msg,
                                  strings::gui_dialogs_project_open_error_ok);
            }
            simpfp::EndFileDialog();
        }
    }

}