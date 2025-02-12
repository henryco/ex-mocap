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

    enum class node_state : std::uint16_t {
        blank     = 0,
        is_group  = 1 << 1,
        is_link   = 1 << 2,
        disabled  = 1 << 3,
        deleted   = 1 << 4,
        moved     = 1 << 5,
        dimmed    = 1 << 6,
        warning   = 1 << 7,
        created   = 1 << 8,
        selected  = 1 << 9,
        connected = 1 << 10
    };

    struct TreeNode {

        const char *name       = nullptr;
        TreeNode   *parent     = nullptr;
        TreeNode  **children   = nullptr;

        std::uint64_t id_node    = 0;
        std::uint32_t children_n = 0;
        std::uint32_t data_size  = 0;
        std::uint32_t id_local   = 0;
        std::uint32_t id_link    = 0;
        std::uint32_t index      = 0;
        node_state    state      = node_state::blank;

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
        static TreeNode dummy();

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
        bool            should_save    = false;
        bool            should_open    = false;
    };

    /** OR */
    constexpr node_state operator | (const node_state &lhs, const node_state &rhs) {
        return static_cast<node_state>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
    }

    /** AND */
    constexpr node_state operator & (const node_state &lhs, const node_state &rhs) {
        return static_cast<node_state>(static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs));
    }

    /** Assign XOR */
    constexpr node_state operator ^= (node_state &lhs, const node_state &rhs) {
        return lhs = static_cast<node_state>(static_cast<uint16_t>(lhs) ^ static_cast<uint16_t>(rhs));
    }

    /** Assign OR */
    constexpr node_state operator |= (node_state &lhs, const node_state &rhs) {
        return lhs = static_cast<node_state>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
    }

    /** Assign OR */
    constexpr node_state operator += (node_state &lhs, const node_state &rhs) {
        return lhs = static_cast<node_state>(static_cast<uint16_t>(lhs) | static_cast<uint16_t>(rhs));
    }

    constexpr node_state operator -= (node_state &lhs, const node_state &rhs) {
        return lhs = static_cast<node_state>((~(static_cast<uint16_t>(rhs))) & static_cast<uint16_t>(lhs));
    }

    constexpr node_state operator && (const node_state &lhs, const bool &rhs) {
        return static_cast<node_state>(rhs ? static_cast<uint16_t>(lhs) : 0);
    }

    constexpr bool operator >>= (const node_state &lhs, const node_state &rhs) {
        return (static_cast<uint16_t>(lhs) & static_cast<uint16_t>(rhs)) != 0;
    }

}

#endif // EX_MOTION_STATE_H
