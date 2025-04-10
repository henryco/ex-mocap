//
// Created by xd on 4/9/25.
//

#include "../../../../headers/imgui/nodes/components/image.h"
#include "../../../../headers/logic/nodes/components/image.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>
#include "imgui.h"


namespace ex::gui::node::cmp::image {

    namespace imgn = ex::logic::node::cmp::image;

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

        void show_open_file_picker_(bool *open, const char *def_path, imgn::image_state_ *state);
    }

    unsigned long int get_node_type_id() {
        return logic::node::cmp::image::ImageNode::get_node_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_IMAGE, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(TreeNode *node) {
        if (node == nullptr)
            return;
        // TODO
    }

    bool draw_node_window(GuiNodeWindow *window) {
        static ex::State &state = ex::logic::globals::state();
        static GuiState  &gui   = ex::gui::state();

        static imgn::ImageNode *img_node = nullptr;
        if (img_node == nullptr || img_node->get_id() != window->node->id_local) {
            imgn::ImageNode *ptr = dynamic_cast<imgn::ImageNode *>(ex::logic::state::get_node(window->node->id_local));
            if (ptr == nullptr || ptr->get_id() != window->node->id_local)
                return true;
            img_node = ptr;
        }

        imgn::image_state_ *state_ = &img_node->state;

        ImGui::SeparatorText("Source not selected");
        ImGui::NewLine();

        const char* text = " Select ";
        ImVec2 textSize = ImGui::CalcTextSize(text);
        float posX = (ImGui::GetContentRegionAvail().x - textSize.x) * 0.5f;
        ImGui::SetCursorPosX(posX);

        static bool open_ = false;
        if (ImGui::Button(text)) {
            open_ = true;
        }

        internal_::show_open_file_picker_(&open_, state.project.project_path, state_);

        // TODO

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

    void internal_::show_open_file_picker_(bool *open, const char *def_path, imgn::image_state_ *state) {
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

                    ::strncpy(state->source, file, sizeof(state->source));
                    state->source[sizeof(state->source) - 1] = '\0';
                    state->selected = true;

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