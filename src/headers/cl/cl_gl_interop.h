//
// Created by xd on 27/11/24.
//

#ifndef EX_MOTION_CL_GL_INTEROP_H
#define EX_MOTION_CL_GL_INTEROP_H

#include <vector>
#include <CL/cl.h>
#include <GLFW/glfw3.h>

namespace ex::cl::utils {

    typedef struct device_platform {
        cl_platform_id platform;
        cl_device_id device;
        bool gl_sharing;
    } device_platform;

    cl_context_properties* get_cl_gl_context_properties(GLFWwindow* window, cl_platform_id platform);

    cl_context create_cl_gl_context(GLFWwindow* window, cl_platform_id platform, cl_device_id device);

    cl_context create_cl_context(cl_device_id device);

    void get_device_platforms(std::vector<device_platform> &platforms);

}

#endif // EX_MOTION_CL_GL_INTEROP_H
