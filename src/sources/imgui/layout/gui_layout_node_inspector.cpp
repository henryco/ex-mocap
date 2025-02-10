//
// Created by xd on 28/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"
#include "../../../../extra/nodes/nodes.h"

namespace ex::gui::tree {

    static TreeNode *inspected_node  = nullptr;
    static bool      inspection_open = false;

    void show_inspected_node(State &, GuiState &gui) {
        if (inspected_node == nullptr || inspected_node->node == nullptr || inspected_node->deleted || !inspection_open) {
            inspection_open = false;
            return;
        }

        const auto base_node = static_cast<limbo::nodes::BaseNode *>(inspected_node->node);

        const auto rf = ex::gui::window_reset_flags(gui.view.reset, true);
        ImGui::Begin("Node inspector", &inspection_open, rf);
        ImGui::Text("%s: [%u] [%s]", inspected_node->name, inspected_node->id_local, base_node->get_node_name());

        ImGui::TextDisabled("");

        int inputs_size;
        const auto inputs = base_node->describe_inputs(&inputs_size);

        int outputs_size;
        const auto outputs = base_node->describe_outputs(&outputs_size);

        for (int i = 0; i < inputs_size; i++) {
            ImGui::Text("input  [%d]: %s (%s)", inputs[i].index, inputs[i].name, inputs[i].aux ? "aux" : "reg");
        }

        ImGui::TextDisabled("");

        for (int i = 0; i < outputs_size; i++) {
            ImGui::Text("output [%d]: %s (%s)", outputs[i].index, outputs[i].name, outputs[i].aux ? "aux" : "reg");
        }

        ImGui::TextDisabled("");
        ImGui::TextDisabled("---");
        ImGui::TextDisabled("");

        const auto connections = base_node->list_connections();
        auto head = connections;
        while (head != nullptr) {
            const auto io = dynamic_cast<limbo::nodes::NodeDescriptor *>(head->io_ptr);
            if (io != nullptr) {
                ImGui::Text("* %d -> %d [%lu] [%s]", head->index_in, head->index_out, head->io_ptr->get_id(), io->get_node_name());
            } else {
                ImGui::Text("* %d -> %d [%lu] [-]", head->index_in, head->index_out, head->io_ptr->get_id());
            }
            head = head->next;
        }

        ImGui::End();
    }

    void pick_inspected_node(State &, GuiState &, TreeNode *node) {
        inspection_open = true;
        inspected_node  = node;
    }

}