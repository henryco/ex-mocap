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
        TreeNode   *parent     = nullptr;
        TreeNode   *children   = nullptr;

        std::size_t children_n = -1;
        std::size_t id_local   = -1;
        std::size_t id_node    = -1;

        bool        is_group   = false;
        bool        disabled   = false;
        bool        updated    = false;
        bool        deleted    = false;
        bool        created    = false;

        ~TreeNode() {
            for (std::size_t i = 0; i < children_n; i++) {

                TreeNode *head = &children[i];
                if (head == nullptr)
                    continue;

                while (true) {
                    if (head->children == nullptr)
                        break;
                    head = head->children;
                }

                while (true) {
                    if (head->children != nullptr) {
                        delete head->children;
                        head->children = nullptr;
                    }

                    if (head->parent == this || head->parent == nullptr)
                        break;
                    TreeNode *_parent = head->parent;
                    head->parent = nullptr;
                    head = _parent;
                }

            }
        }
    };

    struct State {
        const char  *project_path = nullptr;
        GLFWwindow  *window       = nullptr;
        TreeNode    *work_tree    = nullptr;
        cl_context   ocl_context  = nullptr;
        cl_device_id ocl_device   = nullptr;
        short        clock_freq   = -1;
        bool         gl_sharing   = false;
    };

    namespace state {
        struct camera {
            // TODO
        };
    }

}

#endif // EX_MOTION_STATE_H
