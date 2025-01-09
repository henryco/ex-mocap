//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include "../gui_mappers.h"
#include "../../nodes/nodes.h"

namespace ex::gui::test {

    namespace one {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

    namespace two {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

    namespace tri {
        bool draw_tree_node(GuiState &gui, const char *name, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_node_window };
    }

    class NodeOne: public nodes::Node<NodeOne>,
                   public nodes::id::name<"node_one">,

                   public nodes::io::out<short, 1, "out 1 s">,
                   public nodes::io::out<short, 2, "out 2 s">,
                   public nodes::io::out<float, 3, "out 3 f">,
                   public nodes::io::out<short, 4, "out 4 s">,

                   public nodes::io::in<bool, 1, "in 1 b">,
                   public nodes::io::in<char, 2, "in 2 c">,
                   public nodes::io::in<float, 3, "in 3 f">,

                   public nodes::ReactiveOut<short, float>,
                   public nodes::ReactiveIn<bool, char, float>
    {
    private:
        bool stored_float = 0.f;
        bool stored_bool = false;
        bool stored_char = true;
    public:
        bool on_input(const bool data, int input_index, int output_index, int clock_i) override {
            stored_bool = data;
            return true;
        }

        bool on_input(const char data, int input_index, int output_index, int clock_i) override {
            stored_char = data;
            return true;
        }

        bool on_input(const float data, int input_index, int output_index, int clock_i) override {
            stored_float = data;
            return true;
        }

        short on_output(short *null_ptr, const int output_index, int clock_i) override {
            return stored_float + output_index;
        }

        float on_output(float *null_ptr, const int output_index, int clock_i) override {
            return (8 * stored_float) + output_index;
        }
    };

}

#endif //TEST_CMP_H
