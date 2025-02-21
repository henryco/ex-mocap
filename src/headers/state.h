//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

#include <cstdint>

namespace ex {

//    constexpr

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

    struct project_state {
        const char *project_path  = nullptr;
        const char *title         = nullptr;
        bool        has_changes   = false;
        bool        brand_new     = false;
        bool        should_save   = false;
        bool        should_open   = false;
        bool        should_create = false;
    };

    struct error_state {
        const char *msg  = nullptr;
        uint64_t    code = 0;
    };

    struct cl_state {
        cl_context   context = nullptr;
        cl_device_id device  = nullptr;
    };

    struct gl_state {
        GLFWwindow *window     = nullptr;
        int32_t     target_fps = -1;
        bool        sharing    = false;
    };

    struct logic_state {
        TreeNode *work_tree   = nullptr;
        TreeNode *active_node = nullptr;
        int16_t   clock_freq  = -1;
        bool      rewire      = false;
        bool      dirty       = false;
    };

    struct metrics_state {
        float frame_duration_ns = 0.0f;
        float logic_duration_ns = 0.0f;
        float total_duration_ns = 0.0f;
    };

    struct State {
        project_state project = {};
        logic_state   logic   = {};
        error_state   error   = {};
        cl_state      cl      = {};
        gl_state      gl      = {};
        metrics_state metrics = {};
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
