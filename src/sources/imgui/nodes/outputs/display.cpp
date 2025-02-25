//
// Created by xd on 2/25/25.
//

#include "../../../../headers/logic/nodes/outputs/display.h"
#include "../../../../headers/imgui/nodes/outputs/display.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/utils/gui_utils.h"
#include "imgui.h"

#include <IconsLucide.h>

namespace ex::gui::node::out::display {

    unsigned long int get_node_type_id() {
        return ex::logic::node::out::DisplayOutputNode::get_node_id();
    }

    bool draw_tree_node(TreeNode *node, const bool nested, const bool selected, const bool collapse,
                        const bool expand) {
        if (expand)   ImGui::SetNextItemOpen(true, ImGuiCond_Always);
        if (collapse) ImGui::SetNextItemOpen(false, ImGuiCond_Always);
        return plain_node(node->name, true, selected, nested);
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const GuiState &gui = ex::gui::state();
        ImGui::Text("This window 1: %d | %d | %d", window->docked, window->pinned, gui.view.docking);
        return true;
    }

    const char *get_node_name() {
        return ICON_LC_COMPONENT " Display";
    }

} // namespace ex::gui::node::out::display
