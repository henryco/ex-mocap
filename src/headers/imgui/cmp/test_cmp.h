//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include "../gui_mappers.h"
#include "../../nodes/nodes.h"
#include "../../state/mappers.h"

namespace ex::gui::test {

    class NodeOne: public nodes::Node<NodeOne>,
                   public nodes::id::name<"node_one">,
                   public nodes::Serializable<std::string>,

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
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:

        ~NodeOne() override {
            std::cout << "DSTR 1: " << local_id << '\n';
        }

        std::string serialize() const override {
            return "";
        }

        void deserialize(const std::string &serialized) override {

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

    class NodeTwo: public nodes::Node<NodeTwo>,
                   public nodes::id::name<"node_two">,
                   public nodes::Serializable<std::string>,

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
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:
        ~NodeTwo() override {
            std::cout << "DSTR 2: " << local_id << '\n';
        }

        std::string serialize() const override {
            return "";
        }

        void deserialize(const std::string &serialized) override {

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

    class NodeTri: public nodes::Node<NodeTri>,
                       public nodes::id::name<"node_tri">,
                       public nodes::Serializable<std::string>,

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
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';
    public:
        ~NodeTri() override {
            std::cout << "DSTR 3: " << local_id << '\n';
        }

        std::string serialize() const override {
            return "";
        }

        void deserialize(const std::string &serialized) override {

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
        bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_tree_menu, draw_node_window };

        void from_base(State &state, nodes::BaseNode *from, TreeNode *to);
        void from_tree(State &state, TreeNode *from, nodes::BaseNode *to);
        nodes::BaseNode *instance(const State &state, TreeNode *from);
        nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node);
        static constexpr DataMappers data_mappers = { from_base, from_tree, instance, relocate };
    }

    namespace two {
        bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_tree_menu, draw_node_window };

        void from_base(State &state, nodes::BaseNode *from, TreeNode *to);
        void from_tree(State &state, TreeNode *from, nodes::BaseNode *to);
        nodes::BaseNode *instance(const State &state, TreeNode *from);
        nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node);
        static constexpr DataMappers data_mappers = { from_base, from_tree, instance, relocate };
    }

    namespace tri {
        bool draw_tree_node(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        void draw_tree_menu(GuiState &gui, ex::State &state, TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiState &gui, ex::State &state, NodeWindow<TreeNode> *window);
        static constexpr GuiMapper mapper = { draw_tree_node, draw_tree_menu, draw_node_window };

        void from_base(State &state, nodes::BaseNode *from, TreeNode *to);
        void from_tree(State &state, TreeNode *from, nodes::BaseNode *to);
        nodes::BaseNode *instance(const State &state, TreeNode *from);
        nodes::BaseNode *relocate(const State &state, TreeNode *tree, nodes::BaseNode *node);
        static constexpr DataMappers data_mappers = { from_base, from_tree, instance, relocate };
    }

}

#endif //TEST_CMP_H
