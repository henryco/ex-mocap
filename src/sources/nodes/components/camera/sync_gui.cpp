//
// Created by xd on 5/2/25.
//

#include "../../../../headers/nodes/components/camera/sync_gui.h"
#include "../../../../headers/nodes/components/camera/sync_logic.h"
#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include <IconsLucide.h>
#include <simpfp.h>

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

}
