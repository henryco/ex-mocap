//
// Created by xd on 28/01/25.
//

#include "../../../headers/imgui/gui_layout.h"
#include "../../../headers/imgui/gui_tree_ops.h"

namespace ex::gui::tree {

    static TreeNode *inspected_node  = nullptr;
    static bool      inspection_open = false;

    void show_inspected_node(State &, GuiState &gui) {
        if (inspected_node == nullptr || inspected_node->node == nullptr || inspected_node->deleted || !inspection_open) {
            inspection_open = false;
            return;
        }

        const auto rf = ex::gui::window_reset_flags(gui.view.reset, true);
        ImGui::Begin("Node inspector", &inspection_open, rf);
        ImGui::Text("%s", inspected_node->name);

        const auto base_node = static_cast<ex::nodes::BaseNode *>(inspected_node->node);

        int inputs_size;
        const auto inputs = base_node->describe_inputs(&inputs_size);

        for (int i = 0; i < inputs_size; i++) {
            ImGui::Text("input: %s", inputs[i].name);
        }

        // TODO
        ImGui::End();
    }

    void pick_inspected_node(State &, GuiState &, TreeNode *node) {
        inspection_open = true;
        inspected_node  = node;
    }

}