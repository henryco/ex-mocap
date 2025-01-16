//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

namespace ex {

    struct TreeNode {
        void       *data       = nullptr;
        const char *name       = nullptr;
        TreeNode   *parent     = nullptr;
        TreeNode  **children   = nullptr;

        std::size_t children_n = -1;
        std::size_t id_local   = -1;
        std::size_t id_node    = -1;

        bool        is_group   = false;
        bool        disabled   = false;
        bool        updated    = false;
        bool        deleted    = false;
        bool        created    = false;
        bool        selected   = false;
        bool        connected  = false;

        ~TreeNode();

        void      add_node(TreeNode *node);
        TreeNode *find_node(std::size_t id);
    };

    struct selection_node {
        selection_node *root     = nullptr;
        selection_node *next     = nullptr;
        selection_node *prev     = nullptr;
        TreeNode       *selected = nullptr;

        selection_node        *copy();
        static selection_node *copy(selection_node *src);
    };

    struct node_buffer {
        selection_node *selection = nullptr;
        TreeNode       *target    = nullptr;
        bool            child     = false;
        bool            move      = false;

        ~node_buffer();

        static void create(node_buffer **buffer, selection_node *selection, bool move);
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
