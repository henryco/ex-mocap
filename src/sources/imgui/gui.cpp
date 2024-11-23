//
// Created by henryco on 22/11/24.
//

#include "../../headers/gl/render.h"
#include "../../headers/imgui/gui.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

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

    void GUI::draw_gui(const float delta) {
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
}


