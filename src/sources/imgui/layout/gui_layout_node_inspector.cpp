//
// Created by xd on 28/01/25.
//

#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/tree/gui_tree_ops.h"
#include "../../../headers/logic/logic_state.h"

namespace ex::gui::tree {

    static limbo::graph::BaseNode *inspected_base  = nullptr;
    static TreeNode               *inspected_node  = nullptr;
    static bool                    inspection_open = false;
    static long                    inspection_id   = -1;

    void show_inspected_node() {
        static const GuiState &gui = ex::gui::state();

        if (inspected_node == nullptr ||
            (inspected_node->state >>= node_state::deleted) ||
            !inspection_open)
        {
            inspection_open = false;
            inspected_base  = nullptr;
            inspected_base  = nullptr;
            inspection_id   = -1;
            return;
        }

        if (ex::gui::is_dirty() && inspection_id >= 0) {
            TreeNode *new_node = ex::entity::find(inspection_id);
            if (new_node == nullptr) {
                inspection_open = false;
                inspected_node  = nullptr;
                inspected_base  = nullptr;
                inspection_id   = -1;
                return;
            }
            inspected_node = new_node;
            inspected_base = nullptr;
        }

        if (inspected_base == nullptr) {
            inspected_base = ex::logic::state::get_node(inspected_node->id_node);
            if (inspected_base == nullptr) {
                inspection_open = false;
                inspected_node  = nullptr;
                inspection_id   = -1;
                return;
            }
        }

        const auto rf = ex::gui::window_reset_flags(gui.view.reset, true);
        ImGui::Begin("Node inspector", &inspection_open, rf);
        ImGui::Text("%s: [%u] [%s]", inspected_node->name, inspected_node->id_node, inspected_base->get_type_name());

        ImGui::TextDisabled("");

        int inputs_size;
        const auto inputs = inspected_base->describe_inputs(&inputs_size);

        int outputs_size;
        const auto outputs = inspected_base->describe_outputs(&outputs_size);

        for (int i = 0; i < inputs_size; i++) {
            ImGui::Text("input  (# %d): %s (%s)", inputs[i].index, inputs[i].name, inputs[i].aux ? "aux" : "reg");
        }

        ImGui::TextDisabled("");

        for (int i = 0; i < outputs_size; i++) {
            ImGui::Text("output (# %d): %s (%s)", outputs[i].index, outputs[i].name, outputs[i].aux ? "aux" : "reg");
        }

        ImGui::TextDisabled("");
        ImGui::TextDisabled("---");
        ImGui::TextDisabled("");

        if (inspected_base == nullptr) {
            ImGui::End();
            return;
        }

        const auto outputs_ = inspected_base->outputs_;
        for (int i = 0; i < outputs_.size; ++i) {
            auto &head = outputs_[i];
            const auto io = dynamic_cast<limbo::graph::NodeType *>(head.node_ptr);
            if (io != nullptr) {
                ImGui::Text("* i(# %d) <- o(# %d) [%lu] [%s]", head.index_in, head.index_out, head.node_ptr->get_node_id(), io->get_type_name());
            } else {
                ImGui::Text("* i(# %d) <- o(# %d) [%lu] [-]", head.index_in, head.index_out, head.node_ptr->get_node_id());
            }
        }

        const auto inputs_ = inspected_base->inputs_;
        for (int i = 0; i < inputs_.size; ++i) {
            auto &head = inputs_[i];
            const auto io = dynamic_cast<limbo::graph::NodeType *>(head.node_ptr);
            if (io != nullptr) {
                ImGui::Text("* o(# %d) -> i(# %d) [%lu] [%s]", head.index_out, head.index_in, head.node_ptr->get_node_id(), io->get_type_name());
            } else {
                ImGui::Text("* o(# %d) -> i(# %d) [%lu] [-]", head.index_out, head.index_in, head.node_ptr->get_node_id());
            }
        }

        ImGui::End();
    }

    void pick_inspected_node(TreeNode *node) {
        inspection_id   = node->id_node;
        inspected_node  = node;
        inspected_base  = nullptr;
        inspection_open = true;
    }

}