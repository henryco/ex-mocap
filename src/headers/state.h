//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

#include <cstdint>

namespace ex {

    namespace state::uuid {
        std::uint32_t generate_id_inc();
        std::uint32_t get_last_id();
        void          set_last_id(std::uint32_t id);
        void          reset_last_id(std::uint32_t id = 0);
    }

    struct TreeNode {

        /**
         * Data should be allocated using MALLOC,
         * and should be trivially copyable !!!
         */
        void       *data       = nullptr;

        /**
         * Underlying node instance (ie: limbo::nodes::BaseNode *)
         */
        void       *node       = nullptr;

        const char *name       = nullptr;
        TreeNode   *parent     = nullptr;
        TreeNode  **children   = nullptr;

        std::uint64_t id_node    = 0;
        std::uint32_t children_n = 0;
        std::uint32_t data_size  = 0;
        std::uint32_t id_local   = 0;
        std::uint32_t id_link    = 0;
        std::uint32_t index      = 0;

        bool        is_group   = false;
        bool        is_link    = false;
        bool        disabled   = false;
        bool        updated    = false;
        bool        deleted    = false;
        bool        moved      = false;
        bool        dimmed     = false;
        bool        warning    = false;
        bool        created    = false;
        bool        selected   = false;
        bool        connected  = false;

        ~TreeNode();

        void add_node(TreeNode *node);
        void insert_node(int idx, TreeNode *node);
        void remove_node(TreeNode *node);
        bool is_parent(TreeNode *node) const;

        TreeNode *find_node(std::uint32_t id);
        TreeNode *find_node(TreeNode *node);
        TreeNode *shallow_copy() const;
        TreeNode *deep_copy(bool generated_id = false) const;

        static TreeNode *find_node(TreeNode *tree, std::uint32_t id);
        static TreeNode *shallow_copy(TreeNode *node);
        static TreeNode *deep_copy(TreeNode *node, bool generated_id = false);

        static TreeNode *create(std::uint32_t id = 0);
        static TreeNode *link(TreeNode *node);

        static bool is_parent(TreeNode *test, TreeNode *node);

    private:
        TreeNode() = default;
    };

    struct selection_node {
        selection_node *root     = nullptr;
        selection_node *next     = nullptr;
        selection_node *prev     = nullptr;
        TreeNode       *selected = nullptr;

        int index = -1;

        selection_node        *copy() const;
        static selection_node *copy(selection_node *src);
        static selection_node *from_node(TreeNode *node);
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
        int             gui_target_ns  = -1;
        short           clock_freq     = -1;
        bool            gl_sharing     = false;
        bool            force_sync     = false;
        bool            force_rewire   = false;
    };

    namespace state {
        struct camera {
            // TODO
        };
    }

}

#endif // EX_MOTION_STATE_H
