//
// Created by xd on 4/9/25.
//

#include "../../../../headers/imgui/nodes/components/image.h"
#include "../../../../headers/logic/nodes/components/image.h"

#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"

#include "imgui.h"
#include <IconsLucide.h>

namespace ex::gui::node::cmp::image {
    unsigned long int get_node_type_id() {
        return logic::node::cmp::image::ImageNode::get_node_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_IMAGE, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(TreeNode *node) {
        if (node == nullptr)
            return;
        // TODO
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("Image: %d | %d | %d", window->docked, window->pinned, gui.view.docking);
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

}