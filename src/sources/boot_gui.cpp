//
// Created by henryco on 23/11/24.
//

#include "../headers/boot.h"
#include "../headers/gl/gl_utils.h"
#include "../headers/imgui/gui.h"
#include "../headers/cl/cl_gl_interop.h"

#include <GLFW/glfw3.h>
#include <iostream>

int ex::boot::gui(int argc, char **argv, const char *project_path) {

    static const auto log = ex::logs::get_logger("boot_gui");

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW";
        return 1;
    }

    log->info("GLFW initialized");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const auto window = ex::gl::create_window("ExMoCap");
    log->info("Window created");

    if (!window) {
        log->error("Window pointer null reference");
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    log->info("GL context created");

    ex::cl::utils::check_gl_sharing_support();

    auto ocl_context = ex::cl::utils::create_cl_gl_context(window);
    log->info("CL context created");

    if (!ocl_context) {
        log->error("CL context pointer null reference");
        // TODO SHUTDOWN?
    }

    auto gui = ex::gui::GUI(window);
    log->info("ImGUI context created");

    ex::State state {};
    log->info("Application context created");

    // TODO INITIALIZATION

    log->info("Initialization complete");

    while (true) {
        if (glfwWindowShouldClose(window))
            break;

        glfwPollEvents();

        // TODO LOGIC

        gui.build(state);

        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        gui.draw(0.f);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
