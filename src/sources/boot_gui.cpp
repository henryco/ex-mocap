//
// Created by henryco on 23/11/24.
//

#include "../headers/boot.h"
#include "../headers/imgui/gui.h"
#include "../headers/logic/logic.h"
#include "../headers/logic/logic_globals.h"
#include "../headers/imgui/gui_globals.h"
#include "../headers/utils/time_utils.h"
#include "../headers/utils/glfw_utils.h"
#include "../headers/cl/cl_gl_interop.h"
#include "../headers/display/display.h"
#include "../headers/imgui/display/imp.h"

#include <GLFW/glfw3.h>
#include <iostream>

namespace ex::boot {

#define DEFAULT_REFRESH_RATE 60

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

        int display_refresh_rate;
        const auto window = gl::create_window(state.project.title, &display_refresh_rate);

        log->info("Window created");
        state.gl.target_fps = std::max(DEFAULT_REFRESH_RATE, display_refresh_rate);
        state.gl.window = window;

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
                state.gl.sharing = true;
                state.cl.device = device;
                log->info("CL context created");
                break;
            }
        }

        if (!ocl_context) {
            log->error("CL context pointer null reference, fallback to default context");
            const auto device = cl_device_platforms[0].device;
            ocl_context       = ex::cl::utils::create_cl_context(device);
            state.gl.sharing  = false;
            state.cl.device = device;
            log->info("Default CL context created");
        }

        state.cl.context = ocl_context;
    }

    void init_gui_display() {
        ex::display::io.add_output    = &ex::display::imp::imgui::add_output;
        ex::display::io.remove_output = &ex::display::imp::imgui::remove_output;
        ex::display::io.draw_output   = &ex::display::imp::imgui::draw_output;
    }

    inline void update_title(const State &state) {
        static const char *prev_title = state.project.title;

        if (prev_title != state.project.title) {
            prev_title = state.project.title;
            glfwSetWindowTitle(state.gl.window, state.project.title);
        }
    }

    inline void update_history(const State &state) {
        if (state.project.brand_new)
            ex::gui::undo_history().reset();
    }

    inline void update_gui(const ex::gui::GUI &gui, State &state, GLFWwindow *window) {

        static auto prev_target_fps = state.gl.target_fps;
        static auto fps_guard = ex::time::fps_guard::from_fps(state.gl.target_fps);
        if (prev_target_fps != state.gl.target_fps) {
            prev_target_fps = state.gl.target_fps;
            fps_guard.set_fps(state.gl.target_fps);
        }
        if (!fps_guard.step())
            return;

        static ex::time::measure_avg timing;
        if (state.metrics.timing_enabled)
            timing.start();

        gui.build(state);
        state.logic.dirty = false;

        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        gui.draw(0.f);
        glfwSwapBuffers(window);

        if (state.metrics.timing_enabled && timing.stop())
            state.metrics.frame_duration_ns = timing.result();
    }

    int gui(int, char **, const char *project_path) {
//        limbo::graph::memory::allocator = ex::data::mempool::pool;
        State &state = ex::logic::globals::state();
        state.project.project_path = project_path;
        state.project.title        = "ExMoCap";

        log->info("Application context created");

        const auto window = init_gl_context(state);
        init_cl_context(state, window);

        const auto gui = ex::gui::GUI(window);
        log->info("ImGUI context created");

        init_gui_display();
        log->info("Gui display implementation provided");

        auto logic = main::create_logic();

        log->info("Initialization complete");

        while (true) {
            static ex::time::measure_avg timing = 1000;
            if (state.metrics.timing_enabled)
                timing.start();
            // ==========================================================

            if (gui.should_close())
                break;

            glfwPollEvents();

            logic.run();
            update_history(state);
            update_title(state);
            update_gui(gui, state, window);

            // ==========================================================
            if (state.metrics.timing_enabled && timing.stop())
                state.metrics.total_duration_ns = timing.result();
        }

        if (state.cl.context != nullptr)
            clReleaseContext(state.cl.context);
        if (state.cl.device != nullptr)
            clReleaseDevice(state.cl.device);

        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;
    }

}
