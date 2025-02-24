//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include "../../../extra/nodes/nodes.h"
#include "../../logic/nodes/logic_nodes.h"
#include "../gui_nodes.h"

namespace ex::gui::test {

    class NodeOne: public limbo::nodes::Node<NodeOne>,
                   public limbo::nodes::id::name<"node_one">,

                   public limbo::nodes::io::out<short, 1, "out 1 s">,
                   public limbo::nodes::io::out<short, 2, "out 2 s">,
                   public limbo::nodes::io::out<float, 3, "out 3 f">,
                   public limbo::nodes::io::out<short, 4, "out 4 s">,

                   public limbo::nodes::io::in<bool, 1, "in 1 b">,
                   public limbo::nodes::io::in<char, 2, "in 2 c">,
                   public limbo::nodes::io::in<float, 3, "in 3 f">,

                   public limbo::nodes::ReactiveOut<short, float>,
                   public limbo::nodes::ReactiveIn<bool, char, float>
    {
    private:
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:
        NodeOne() {
            std::cout << "CNTR 1\n";
        }

        ~NodeOne() override {
            std::cout << "DSTR 1: " << local_id << '\n';
        }

        bool on_input(const bool data, int input_index, int output_index, int clock_i, Output<bool> *io) override {
            stored_bool = data;
            return true;
        }

        bool on_input(const char data, int input_index, int output_index, int clock_i, Output<char> *io) override {
            stored_char = data;
            return true;
        }

        bool on_input(const float data, int input_index, int output_index, int clock_i, Output<float> *io) override {
            stored_float = data;
            return true;
        }

        short on_output(short *null_ptr, const int output_index, int clock_i, Input<short> *io) override {
            return stored_float + output_index + 2;
        }

        float on_output(float *null_ptr, const int output_index, int clock_i, Input<float> *io) override {
            const float o = (1 * stored_float) + output_index + 2;
//            std::cout << "out_1[" << get_id() << "]: " << o << '\n';
            return o;
        }
    };

    class NodeTwo: public limbo::nodes::Node<NodeTwo>,
                   public limbo::nodes::id::name<"node_two">,

                   public limbo::nodes::io::out<short, 1, "out 1 s">,
                   public limbo::nodes::io::out<short, 2, "out 2 s">,
                   public limbo::nodes::io::out<float, 3, "out 3 f">,
                   public limbo::nodes::io::out<short, 4, "out 4 s">,

                   public limbo::nodes::io::in<bool, 1, "in 1 b">,
                   public limbo::nodes::io::in<char, 2, "in 2 c">,
                   public limbo::nodes::io::in<float, 3, "in 3 f">,

                   public limbo::nodes::ReactiveOut<short, float>,
                   public limbo::nodes::ReactiveIn<bool, char, float>
    {
    private:
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:
        NodeTwo() {
            std::cout << "CNTR 2\n";
        }

        ~NodeTwo() override {
            std::cout << "DSTR 2: " << local_id << '\n';
        }

        bool on_input(const bool data, int input_index, int output_index, int clock_i, Output<bool> *io) override {
            stored_bool = data;
            return true;
        }

        bool on_input(const char data, int input_index, int output_index, int clock_i, Output<char> *io) override {
            stored_char = data;
            return true;
        }

        bool on_input(const float data, int input_index, int output_index, int clock_i, Output<float> *io) override {
            stored_float = data;
            return true;
        }

        short on_output(short *null_ptr, const int output_index, int clock_i, Input<short> *io) override {
            return stored_float + output_index + 3;
        }

        float on_output(float *null_ptr, const int output_index, int clock_i, Input<float> *io) override {
            const float o = (3 * stored_float) + output_index + 3;
//            std::cout << "out_2[" << get_id() << "]: " << o << '\n';
            return o;
        }
    };

    class NodeTri: public limbo::nodes::Node<NodeTri>,
                   public limbo::nodes::id::name<"node_tri">,

                   public limbo::nodes::io::out<short, 1, "out 1 s">,
                   public limbo::nodes::io::out<short, 2, "out 2 s">,
                   public limbo::nodes::io::out<float, 3, "out 3 f">,
                   public limbo::nodes::io::out<short, 4, "out 4 s">,

                   public limbo::nodes::io::in<bool, 1, "in 1 b">,
                   public limbo::nodes::io::in<char, 2, "in 2 c">,
                   public limbo::nodes::io::in<float, 3, "in 3 f">,

                   public limbo::nodes::ReactiveOut<short, float>,
                   public limbo::nodes::ReactiveIn<bool, char, float>
    {
    private:
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:
        NodeTri() {
            std::cout << "CNTR 3\n";
        }

        ~NodeTri() override {
            std::cout << "DSTR 3: " << local_id << '\n';
        }

        bool on_input(const bool data, int input_index, int output_index, int clock_i, Output<bool> *io) override {
            stored_bool = data;
            return true;
        }

        bool on_input(const char data, int input_index, int output_index, int clock_i, Output<char> *io) override {
            stored_char = data;
            return true;
        }

        bool on_input(const float data, int input_index, int output_index, int clock_i, Output<float> *io) override {
            stored_float = data;
            return true;
        }

        short on_output(short *null_ptr, const int output_index, int clock_i, Input<short> *io) override {
            return stored_float + output_index + 4;
        }

        float on_output(float *null_ptr, const int output_index, int clock_i, Input<float> *io) override {
            const float o = (2 * stored_float) + output_index + 4;
//            std::cout << "out_3[" << get_id() << "]: " << o << '\n';
            return o;
        }
    };

    namespace one {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { draw_tree_node, draw_tree_menu, draw_node_window, get_node_name };

        limbo::nodes::BaseNode *instance(TreeNode *from);
        limbo::nodes::BaseNode *relocate(TreeNode *tree, limbo::nodes::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

    namespace two {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { draw_tree_node, draw_tree_menu, draw_node_window, get_node_name };

        limbo::nodes::BaseNode *instance(TreeNode *from);
        limbo::nodes::BaseNode *relocate(TreeNode *tree, limbo::nodes::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

    namespace tri {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { draw_tree_node, draw_tree_menu, draw_node_window, get_node_name };

        limbo::nodes::BaseNode *instance(TreeNode *from);
        limbo::nodes::BaseNode *relocate(TreeNode *tree, limbo::nodes::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

}

#endif //TEST_CMP_H
