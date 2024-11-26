//
// Created by henryco on 22/11/24.
//

#include "../../headers/imgui/gui.h"

#include <IconsLucide.h_lucide.ttf.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace ex::gui {

    inline void init_font_icons(ImGuiIO &io) {
        constexpr float base_font_size   = 13.f;
        constexpr float icon_font_size   = 16.f;
        constexpr float font_multiply    = .7f;
        constexpr int   over_sample_size = 5;

        ImFontConfig default_config;
        default_config.FontDataOwnedByAtlas = true;
//        default_config.RasterizerDensity    = 1.25f;
        default_config.PixelSnapH           = true;
//        default_config.RasterizerMultiply   = font_multiply;
        default_config.SizePixels           = base_font_size;
        default_config.OversampleH          = over_sample_size;
        default_config.OversampleV          = over_sample_size;

        io.Fonts->AddFontDefault(&default_config);

        static const ImWchar icons_ranges[] = {
            ICON_MIN_LC,
            ICON_MAX_16_LC,
            0
        };

        ImFontConfig icons_config;
        icons_config.GlyphMinAdvanceX     = icon_font_size;
        icons_config.FontDataOwnedByAtlas = false;
        icons_config.MergeMode            = true;
        icons_config.PixelSnapH           = true;
        icons_config.OversampleH          = 5;
        icons_config.OversampleV          = 5;
        icons_config.GlyphOffset          = ImVec2(0.f, 5.f);

        io.Fonts->AddFontFromMemoryTTF(
                const_cast<void*>(static_cast<const void*>(s_lucide_ttf)),
                sizeof(s_lucide_ttf),
                icon_font_size,
                &icons_config,
                icons_ranges);
        io.Fonts->Build();
    }

    void GUI::build(ex::State &state) const {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        GUI::layout(state);

        ImGui::Render();
    }

    void GUI::draw(const float delta) const {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    GUI::GUI(GLFWwindow *window) {
        this->window = window;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        init_font_icons(io);

        ImGui::StyleColorsLight();

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");
    }

    GUI::~GUI() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}


