//
// Created by henryco on 23/11/24.
//

#include "../headers/boot.h"
#include "../headers/imgui/gui.h"
#include "../headers/logic/logic.h"
#include "../headers/cl/cl_gl_interop.h"
#include "../headers/utils/gl_utils.h"
#include "../headers/logic/logic_state.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace ex::boot {

    const auto log = logs::get_logger("boot_gui");

    void framebuffer_size_callback(GLFWwindow *, const int width, const int height) {
        glViewport(0, 0, width, height);
    }

    GLFWwindow *init_gl_context(State &state) {
        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW";
            return nullptr;
        }

        log->info("GLFW initialized");

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_DOUBLEBUFFER, GLFW_TRUE);

        const auto window = gl::create_window("ExMoCap");
        log->info("Window created");
        state.window = window;

        if (!window) {
            log->error("Window pointer null reference");
            glfwTerminate();
            return nullptr;
        }

        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
        glfwSwapInterval(0);
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

    void gui_set_target_fps(State &state, const double fps) {
        state.gui_target_ns = static_cast<int>(1000000000.0 / fps);
    }

    inline void update_gui(const ex::gui::GUI &gui, State &state, GLFWwindow *window) {
        if (state.gui_target_ns > 0) {
            static auto t0 = std::chrono::high_resolution_clock::now();
            const auto t1 = std::chrono::high_resolution_clock::now();
            if (t1 - t0 < std::chrono::nanoseconds(state.gui_target_ns))
                return;
            t0 = std::chrono::high_resolution_clock::now();
        }

        gui.build(state);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        gui.draw(0.f);
        glfwSwapBuffers(window);
    }

    int gui(int, char **, const char *project_path) {

        State &state = ex::logic::state::get_state();
        state.project_path = project_path;

        gui_set_target_fps(state, 60);

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
            update_gui(gui, state, window);
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
