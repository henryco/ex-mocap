//
// Created by xd on 27/11/24.
//

#ifndef EX_MOTION_CL_GL_INTEROP_H
#define EX_MOTION_CL_GL_INTEROP_H

#include <GLFW/glfw3.h>
#include <CL/cl.h>

namespace ex::cl::utils {

    cl_context_properties* get_cl_gl_context_properties(GLFWwindow* window, cl_platform_id platform);

    cl_context create_cl_gl_context(GLFWwindow* window);

    void check_gl_sharing_support();

}

#endif // EX_MOTION_CL_GL_INTEROP_H
