//
// Created by xd on 03/01/25.
//

#ifndef TEST_CMP_H
#define TEST_CMP_H

#include <iostream>

#include "../../../extra/graph/graph.h"
#include "../../../logic/nodes/logic_nodes.h"
#include "../gui_nodes.h"

namespace ex::gui::test {

    struct NodeOne final : limbo::graph::BaseNode,
                           limbo::graph::id::name<"node_one">,
                           limbo::graph::io::out<short, 1, "out 1 s">,
                           limbo::graph::io::out<short, 2, "out 2 s">,
                           limbo::graph::io::out<float, 3, "out 3 f">,
                           limbo::graph::io::out<short, 4, "out 4 s">,

                           limbo::graph::io::in<bool, 1, "in 1 b">,
                           limbo::graph::io::in<char, 2, "in 2 c">,
                           limbo::graph::io::in<float, 3, "in 3 f">
    {

        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';

        NodeOne() {
            std::cout << "CNTR 1\n";
        }

        ~NodeOne() override {
            std::cout << "DSTR 1: " << id_ << std::endl;
        }

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {
            std::cout << "in1: " << id_ << std::endl;
        }

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override {
            std::cout << "out1: " << id_ << std::endl;
        }
    };

    struct NodeTwo final : limbo::graph::BaseNode,
                           limbo::graph::id::name<"node_two">,

                           limbo::graph::io::out<short, 1, "out 1 s">,
                           limbo::graph::io::out<short, 2, "out 2 s">,
                           limbo::graph::io::out<float, 3, "out 3 f">,
                           limbo::graph::io::out<short, 4, "out 4 s">,

                           limbo::graph::io::in<bool, 1, "in 1 b">,
                           limbo::graph::io::in<char, 2, "in 2 c">,
                           limbo::graph::io::in<float, 3, "in 3 f">
    {
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';

        NodeTwo() {
            std::cout << "CNTR 2\n";
        }

        ~NodeTwo() override {
            std::cout << "DSTR 2: " << id_ << std::endl;
        }

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {
            std::cout << "in2: " << id_ << std::endl;
        }

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override {
            std::cout << "in2: " << id_ << std::endl;
        }
    };

    struct NodeTri final : limbo::graph::BaseNode,
                           limbo::graph::id::name<"node_tri">,

                           limbo::graph::io::out<short, 1, "out 1 s">,
                           limbo::graph::io::out<short, 2, "out 2 s">,
                           limbo::graph::io::out<float, 3, "out 3 f">,
                           limbo::graph::io::out<short, 4, "out 4 s">,

                           limbo::graph::io::in<bool, 1, "in 1 b">,
                           limbo::graph::io::in<char, 2, "in 2 c">,
                           limbo::graph::io::in<float, 3, "in 3 f">
    {
        float stored_float = 0.f;
        bool stored_bool = false;
        char stored_char = '\0';

        NodeTri() {
            std::cout << "CNTR 3\n";
        }

        ~NodeTri() override {
            std::cout << "DSTR 3: " << id_ << std::endl;
        }

        void on_input(int in_idx, int out_idx, int clock_i, const void *data) override {
            std::cout << "in3: " << id_ << std::endl;
        }

        void on_output(int in_idx, int out_idx, int clock_i, void *data) override {
            std::cout << "in3: " << id_ << std::endl;
        }

    };

    namespace one {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_preview();
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { get_node_name, get_node_preview, draw_tree_node, draw_tree_menu,
                                                       draw_node_window };
        limbo::graph::BaseNode *instance(TreeNode *from);
        limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

    namespace two {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_preview();
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { get_node_name, get_node_preview, draw_tree_node, draw_tree_menu,
                                                       draw_node_window };
        limbo::graph::BaseNode *instance(TreeNode *from);
        limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

    namespace tri {
        bool draw_tree_node(TreeNode *node, bool nested, bool selected, bool collapse, bool expand);
        void draw_tree_menu(TreeNode *node, bool nested, bool selected);
        bool draw_node_window(GuiNodeWindow *window);
        const char *get_node_preview();
        const char *get_node_name();
        static constexpr GuiNodeMapper gui_mappers = { get_node_name, get_node_preview, draw_tree_node, draw_tree_menu,
                                                       draw_node_window };
        limbo::graph::BaseNode *instance(TreeNode *from);
        limbo::graph::BaseNode *relocate(TreeNode *tree, limbo::graph::BaseNode *node);
        static constexpr logic::NodeMapper data_mappers = { instance, relocate };
    }

}

#endif //TEST_CMP_H
