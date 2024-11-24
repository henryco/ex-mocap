//
// Created by henryco on 22/11/24.
//

#include "../../headers/imgui/gui.h"

#include <GLFW/glfw3.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <imgui.h>

namespace ex::gui {

    void GUI::update_gui(ex::State &state) {
        static float leftPaneWidth = 200.0f;
        static float rightPaneWidth = 200.0f;
        static float bottomPaneHeight = 150.0f;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        ImGui::Begin("main", nullptr,
                     ImGuiWindowFlags_NoResize |
                             ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoTitleBar |
                             ImGuiWindowFlags_NoScrollWithMouse);

        float mainWidth = ImGui::GetContentRegionAvail().x;

        if (state.gui.show_left) {
            // Left Pane
            ImGui::BeginChild("LeftPane", ImVec2(leftPaneWidth, 0), true);
            ImGui::Text("Left Pane (e.g., File Explorer)");
            ImGui::Button("Sample Button");
            ImGui::EndChild();
            ImGui::SameLine();
        }


        // Main Content with Tabs
        ImGui::BeginChild("MainContainer", ImVec2(mainWidth - (state.gui.show_left ? leftPaneWidth : 0) - (state.gui.show_right ? rightPaneWidth : 0), -bottomPaneHeight), true);
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


        if (state.gui.show_right) {
            // Right Pane
            ImGui::SameLine();
            ImGui::BeginChild("RightPane", ImVec2(rightPaneWidth, 0), true);
            ImGui::Text("Right Pane (e.g., Properties)");
            ImGui::EndChild();
        }

        if (state.gui.show_logs) {
            // 2. Bottom Pane (Logs)
            ImGui::BeginChild("BottomPane", ImVec2(0, bottomPaneHeight), true);
            ImGui::Text("Log Output:");
            ImGui::Separator();
            ImGui::Text("This is a log message.");
            ImGui::Text("Another log message.");
            ImGui::EndChild();
        }

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


