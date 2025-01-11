//
// Created by henryco on 23/11/24.
//

#include "../headers/boot.h"
#include "../headers/imgui/gui.h"
#include "../headers/cl/cl_gl_interop.h"
#include "../headers/utils/gl_utils.h"

#include <GLFW/glfw3.h>
#include <iostream>

#include "../headers/logic.h"

namespace ex::boot {

    const auto log = logs::get_logger("boot_gui");

    GLFWwindow *init_gl_context(State &state) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW";
            return nullptr;
        }

        log->info("GLFW initialized");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        const auto window = gl::create_window("ExMoCap");
        log->info("Window created");
        state.window = window;

        if (!window) {
            log->error("Window pointer null reference");
            glfwTerminate();
            return nullptr;
        }

        glfwMakeContextCurrent(window);
        log->info("GL context created");

        return window;
    }

    void init_cl_context(State &state, GLFWwindow *window) {
        std::vector<cl::utils::device_platform> cl_device_platforms;
        get_device_platforms(cl_device_platforms);
        log->info("Queried cl_device_platforms: {}", cl_device_platforms.size());

        if (cl_device_platforms.empty()) {
            log->error("Not found any cl_device_platforms");
            return;
        }

        cl_context ocl_context = nullptr;
        for (auto &[platform, device, gl_sharing] : cl_device_platforms) {
            if (!gl_sharing)
                continue;
            ocl_context = cl::utils::create_cl_gl_context(window, platform, device);
            if (ocl_context != nullptr) {
                state.gl_sharing = true;
                state.ocl_device = device;
                log->info("CL context created");
                break;
            }
        }

        if (!ocl_context) {
            log->error("CL context pointer null reference, fallback to default context");
            const auto device = cl_device_platforms[0].device;
            ocl_context       = ex::cl::utils::create_cl_context(device);
            state.gl_sharing  = false;
            state.ocl_device = device;
            log->info("Default CL context created");
        }

        state.ocl_context = ocl_context;
    }

    int gui(int argc, char **argv, const char *project_path) {

        State state{ .project_path = project_path };
        log->info("Application context created");

        const auto window = init_gl_context(state);
        init_cl_context(state, window);

        const auto gui = ex::gui::GUI(window);
        log->info("ImGUI context created");

        auto logic = main::create_logic(state);

        log->info("Initialization complete");

        while (true) {
            if (glfwWindowShouldClose(window))
                break;

            glfwPollEvents();

            logic.run(state);
            gui.build(state);

            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            glClearColor(0, 0, 0, 0);
            glClear(GL_COLOR_BUFFER_BIT);

            gui.draw(0.f);

            glfwSwapBuffers(window);
        }

        if (state.ocl_context != nullptr)
            clReleaseContext(state.ocl_context);
        if (state.ocl_device != nullptr)
            clReleaseDevice(state.ocl_device);

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }

}
