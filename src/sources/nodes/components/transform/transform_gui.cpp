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
        auto *trs_node = dynamic_cast<logic::node::transform::TransformNode *>(
                ex::logic::state::get_node(window->node->id_node));
        if (trs_node == nullptr || trs_node->get_node_id() != window->node->id_node)
            return true;

        // TODO

        return true;
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        // TODO
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
