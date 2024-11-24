//
// Created by henryco on 22/11/24.
//

#include "../../headers/imgui/gui.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

namespace ex::gui {

    static float left_pane_width  = 200.0f;
    static float right_pane_width = 200.0f;
    static float bottom_pane_height = 150.0f;
    static float main_width = 0.f;

    void show_left_pane(ex::State &state) {
        if (!state.gui.show_left)
            return;
        ImGui::BeginChild("LeftPane", ImVec2(left_pane_width, 0), true);
        ImGui::Text("Left Pane (e.g., File Explorer)");
        ImGui::Button("Sample Button");
        ImGui::EndChild();
        ImGui::SameLine();
    }

    void show_right_pane(ex::State &state) {
        if (!state.gui.show_right)
            return;
        ImGui::SameLine();
        ImGui::BeginChild("RightPane", ImVec2(right_pane_width, 0), true);
        ImGui::Text("Right Pane (e.g., Properties)");
        ImGui::EndChild();
    }

    void show_logs(ex::State &state) {
        if (!state.gui.show_logs)
            return;
        ImGui::BeginChild("BottomPane", ImVec2(0, bottom_pane_height), true);
        ImGui::Text("Log Output:");
        ImGui::Separator();
        ImGui::Text("This is a log message.");
        ImGui::Text("Another log message.");
        ImGui::EndChild();
    }

    void show_main_content(ex::State &state) {
        ImGui::BeginChild("MainContainer", ImVec2(main_width - (state.gui.show_left ? left_pane_width : 0) - (state.gui.show_right ? right_pane_width : 0), -bottom_pane_height), true);
        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Tab 1")) {
                ImGui::Text("Content for Tab 1");
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Tab 2")) {
                ImGui::Text("Content for Tab 2");
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
        ImGui::EndChild();
    }

    void GUI::update_gui(ex::State &state) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        {
            ImGui::Begin("main", nullptr,
                         ImGuiWindowFlags_NoResize |
                                 ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoTitleBar |
                                 ImGuiWindowFlags_NoScrollWithMouse);

            main_width = ImGui::GetContentRegionAvail().x;

            show_left_pane(state);
            show_main_content(state);
            show_right_pane(state);
            show_logs(state);

            ImGui::End();
        }

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


