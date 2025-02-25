//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/imgui/nodes/outputs/display.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"
#include "imgui.h"

#include <IconsLucide.h>

namespace ex::gui::node::out::display {

    unsigned long int get_node_type_id() {
        return ex::logic::node::out::DisplayOutputNode::get_node_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_PICTURE_IN_PICTURE_2, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(TreeNode *node) {
        std::cout << "update display tree node\n";
        // TODO
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("This window 1: %d | %d | %d", window->docked, window->pinned, gui.view.docking);
        return true;
    }

    const char *get_node_name() {
        return strings::node_output_display_name;
    }

} // namespace ex::gui::node::out::display
