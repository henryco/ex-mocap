//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

namespace ex {

    struct TreeNode {
        TreeNode   *parent     = nullptr;
        TreeNode   *children   = nullptr;
        void       *data       = nullptr;
        std::size_t children_n = 0;
        std::size_t id_node    = -1;
        std::size_t id_local   = -1;
        bool        updated    = false;
    };

    struct State {
        const char  *project_path = nullptr;
        GLFWwindow  *window       = nullptr;
        cl_context   ocl_context  = nullptr;
        cl_device_id ocl_device   = nullptr;
        bool         gl_sharing   = false;
        TreeNode    *work_tree    = nullptr;
    };

}

#endif // EX_MOTION_STATE_H
