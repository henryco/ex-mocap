//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

namespace ex {

    struct TreeNode {

        /**
         * Data should be allocated using MALLOC,
         * and should be trivially copyable !!!
         */
        void       *data       = nullptr;

        const char *name       = nullptr;
        TreeNode   *parent     = nullptr;
        TreeNode  **children   = nullptr;

        std::size_t children_n = 0;
        std::size_t id_local   = 0;
        std::size_t id_node    = 0;
        std::size_t index      = 0;
        std::size_t data_size  = 0;

        bool        is_group   = false;
        bool        disabled   = false;
        bool        updated    = false;
        bool        deleted    = false;
        bool        shared     = false;
        bool        created    = false;
        bool        selected   = false;
        bool        connected  = false;

        ~TreeNode();

        void add_node(TreeNode *node);
        void insert_node(int idx, TreeNode *node);
        void remove_node(TreeNode *node);

        TreeNode *find_node(std::size_t id);
        TreeNode *find_node(TreeNode *node);
        TreeNode *shallow_copy();
        TreeNode *deep_copy();

        static TreeNode *find_node(TreeNode *tree, std::size_t id);
        static TreeNode *shallow_copy(TreeNode *node);
    };

    struct selection_node {
        selection_node *root     = nullptr;
        selection_node *next     = nullptr;
        selection_node *prev     = nullptr;
        TreeNode       *selected = nullptr;

        int index = -1;

        selection_node        *copy();
        static selection_node *copy(selection_node *src);
        static bool            single(selection_node *node);
        static void            remove(selection_node *node, selection_node **next);
        static void            free(selection_node *node, bool unselect = false);
    };

    struct node_buffer {
        selection_node *selection = nullptr;
        bool            move      = false;
        bool            ref       = false;

        ~node_buffer();

        static void create(node_buffer **buffer, selection_node *selection, bool move = false, bool ref = false);
        static void create(node_buffer **buffer, TreeNode *node, bool move = false, bool ref = false);
        static void remove(node_buffer **buffer);
    };

    struct State {
        const char     *project_path   = nullptr;
        GLFWwindow     *window         = nullptr;
        TreeNode       *work_tree      = nullptr;
        TreeNode       *active_node    = nullptr;
        selection_node *selected_nodes = nullptr;
        node_buffer    *clipboard_node = nullptr;
        cl_context      ocl_context    = nullptr;
        cl_device_id    ocl_device     = nullptr;
        short           clock_freq     = -1;
        bool            gl_sharing     = false;
    };

    namespace state {
        struct camera {
            // TODO
        };
    }

}

#endif // EX_MOTION_STATE_H
