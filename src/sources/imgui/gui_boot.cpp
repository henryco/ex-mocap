//
// Created by henryco on 22/11/24.
//

#include "../../headers/imgui/gui_boot.h"
#include "../../headers/gl/render.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>
#include <iostream>

namespace ex::gui {

    void GUI::update_gui(ex::State &state) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Hello, ImGui!");
        ImGui::Text("This is a simple example");
        ImGui::End();

        ImGui::Render();
    }

    void GUI::draw_gui() {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GUI::GUI(GLFWwindow *window) {
        this->window = window;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");
    }

    GUI::~GUI() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    int boot(const int argc, char **argv, const char *project_path) {

        if (!glfwInit()) {
            std::cerr << "Failed to initialize GLFW";
            return 1;
        }

        auto window = ex::gl::create_window("ExMoCap");
        if (!window) {
            glfwTerminate();
            return 1;
        }

        struct ex::State state;
        auto render = ex::gl::Render(window);
        {
            auto gui = ex::gui::GUI(window);


            while (!glfwWindowShouldClose(window)) {

                gui.update_gui(state);


                gui.draw_gui();

            }
        };

        return 0;
    }
}


