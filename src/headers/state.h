//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

#include <CL/cl.h>
#include <GLFW/glfw3.h>

namespace ex {

    struct State {
        const char  *project_path = nullptr;
        GLFWwindow  *window       = nullptr;
        cl_context   ocl_context  = nullptr;
        cl_device_id ocl_device   = nullptr;
        bool         gl_sharing   = false;
    };

}

#endif // EX_MOTION_STATE_H
