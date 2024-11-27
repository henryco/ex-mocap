//
// Created by xd on 27/11/24.
//

#include "../../headers/cl/cl_gl_interop.h"
#include "../../headers/utils/log_utils.h"

#if defined(_WIN32)
#define GLFW_EXPOSE_NATIVE_WIN32
#define GLFW_EXPOSE_NATIVE_WGL
#elif defined(__linux__)
#define GLFW_EXPOSE_NATIVE_X11
#define GLFW_EXPOSE_NATIVE_GLX
#endif

#include <GLFW/glfw3native.h>
#include <cstdio>

namespace ex::cl::utils {

    ex::logs::logger provide_logger() {
        static const auto log = ex::logs::get_logger("cl_gl_interop");
        return log;
    }

    cl_context_properties* get_cl_gl_context_properties(GLFWwindow* window, cl_platform_id platform) {
        const auto log = provide_logger();
        log->info("Getting cl_gl context properties");

        static cl_context_properties properties[] = {
            CL_CONTEXT_PLATFORM, (cl_context_properties) platform,

#if defined(_WIN32)
            CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetWGLContext(window),
            CL_WGL_HDC_KHR, (cl_context_properties) wglGetCurrentDC(),
#elif defined(__linux__)
            CL_GL_CONTEXT_KHR, (cl_context_properties) glfwGetGLXContext(window),
            CL_GLX_DISPLAY_KHR, (cl_context_properties) glfwGetX11Display(),
#endif
            0
        };

        return properties;
    }

    cl_context create_cl_gl_context(GLFWwindow* window) {
        const auto log = provide_logger();

        log->info("Creating cl_gl context");

        cl_int err;

        cl_platform_id platform;
        err = clGetPlatformIDs(1, &platform, nullptr);
        if (err != CL_SUCCESS) {
            log->error("Failed to get cl platform ids: {}", err);
            fprintf(stderr, "Failed to get cl platform ids: %d\n", err);
            return nullptr;
        }

        cl_device_id device;
        err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, nullptr);
        if (err != CL_SUCCESS) {
            log->error("Failed to get opencl device id: {}", err);
            fprintf(stderr, "Failed to get opencl device id: %d\n", err);
            return nullptr;
        }

        auto props = ex::cl::utils::get_cl_gl_context_properties(window, platform);
        log->info("Acquired cl_gl interop context properties");

        cl_context context = clCreateContext(props, 1, &device,
                                             nullptr, nullptr, &err);
        if (err != CL_SUCCESS) {
            log->error("Failed to create opencl context: {}", err);
            fprintf(stderr, "Failed to create opencl context: %d\n", err);
            return nullptr;
        }

        log->info("Created cl_gl interop context");
        return context;
    }
}

