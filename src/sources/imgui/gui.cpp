//
// Created by henryco on 22/11/24.
//

#include "../../headers/imgui/layout/gui_layout.h"
#include "../../headers/imgui/gui.h"

#include <IconsLucide.h>
#include <IconsLucide.h_lucide.ttf.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <imgui.h>

namespace ex::gui {

    namespace internal_ {
        static bool should_exit = false;
    }

    inline void init_font_icons(ImGuiIO &io) {
        GUI::log->info("initializing font icons");

        constexpr float base_font_size   = 13.f;
        constexpr float icon_font_size   = 16.f;
        constexpr int   over_sample_size = 5;

        ImFontConfig default_config;
        default_config.FontDataOwnedByAtlas = true;
        default_config.PixelSnapH           = true;
        default_config.SizePixels           = base_font_size;
        default_config.OversampleH          = over_sample_size;
        default_config.OversampleV          = over_sample_size;

        io.Fonts->AddFontDefault(&default_config);

        static constexpr ImWchar icons_ranges[] = {
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

        GUI::log->info("font icons initialized");
    }

    void GUI::build(ex::State &state) const {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();

        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

        internal_::should_exit = false;
        if (ex::gui::layout::build(state)) {
            internal_::should_exit = true;
        }

        ImGui::Render();
    }

    void GUI::draw(const float) const {
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    bool GUI::should_close() const {
        return internal_::should_exit;
    }

    GUI::GUI(GLFWwindow *window) {
        this->window = window;

        GUI::log->info("creating imgui context");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        init_font_icons(io);

        ImGui::StyleColorsLight();

        GUI::log->info("initialize imgui gl backend");

        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");

        GUI::log->info("imgui gl backend initialized");
        GUI::log->info("imgui context created");
    }

    GUI::~GUI() {
        GUI::log->info("destroy imgui context");

        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        GUI::log->info("imgui context destroyed");
    }
}


