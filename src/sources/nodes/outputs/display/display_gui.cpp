//
// Created by xd on 2/25/25.
//

#include "../../../../headers/nodes/outputs/display/display_logic.h"
#include "../../../../headers/nodes/outputs/display/display_gui.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"
#include "imgui.h"

#include <IconsLucide.h>

namespace ex::gui::node::out::display {

    namespace elno = ex::logic::node::out;

    unsigned long int get_node_type_id() {
        namespace elno = ex::logic::node::out;
        return elno::DisplayOutputNode::get_static_type_id();
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_PICTURE_IN_PICTURE_2, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        if (before == nullptr || after == nullptr)
            return;

        if (!(before->state >>= node_state::disabled) &&
            (after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }

        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node)))
                base->enable();
            return;
        }

        if (const auto base = dynamic_cast<elno::DisplayOutputNode *>(ex::logic::state::get_node(after->id_node))) {
            base->rename_image(after->name);
        }
    }

    bool draw_node_window(GuiNodeWindow *window) {

        const id_type_32 id = window->node->id_node;
        auto *trs_node = dynamic_cast<elno::DisplayOutputNode *>(logic::state::get_node(id));
        if (trs_node == nullptr || trs_node->get_node_id() != id)
            return true;
        {
            ImGui::TextDisabled("Input ID: %u", trs_node->get_input_id());
            ImGui::TextDisabled("Input  W: %u", trs_node->get_input_w());
            ImGui::TextDisabled("Input  H: %u", trs_node->get_input_h());
            ImGui::NewLine();
        }

        return true;
    }

    const char *get_node_preview() {
        static char  out_name[128];
        static char *name = nullptr;
        if (name == nullptr) {
            name = out_name;
            sprintf(name, "%s %s", ICON_LC_PICTURE_IN_PICTURE_2, strings::node_output_display_name);
        }
        return name;
    }

    const char *get_node_name() {
        return strings::node_output_display_name;
    }

} // namespace ex::gui::node::out::display
