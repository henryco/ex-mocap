//
// Created by xd on 02/12/24.
//

#include "../../../headers/imgui/layout/display/gui_display.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/imgui/gui_globals.h"

namespace ex::gui {

    void show_content() {
        GuiState &gui = ex::gui::state();
        static float old_height = 0;

        ImGui::BeginChild("ContentBox",
                          content_size(gui),
                          0
                                  | ImGuiChildFlags_Borders
                                  | content_resize_flag(gui)
                                  | ((gui.re_size.inc_y || gui.window.resized) ? ImGuiChildFlags_AutoResizeY : 0),
                          reset_flag(gui)
        );

        display::show_display();

        const float curr_height = ImGui::GetWindowHeight();
        ImGui::EndChild();

        gui.re_size.inc_y = (!gui.re_size.inc_y) && (old_height != curr_height);
        old_height = curr_height;
    }

    void show_main_container() {
        GuiState &gui = ex::gui::state();

        static float old_width, old_height = 0;

        ImGui::BeginChild("MainContainer",
                          main_size(gui),
                          0
                                  | (gui.view.show_right ? ImGuiChildFlags_ResizeX : 0)
                                  | ((gui.re_size.inc_x || gui.window.resized) ? ImGuiChildFlags_AutoResizeX : 0)
                                  ,
                          0
                                  | ImGuiWindowFlags_NoScrollWithMouse
                                  | ImGuiWindowFlags_NoScrollbar
                                  | reset_flag(gui)
        );
        {
            show_content();
            show_logs_pane();
        }

        const float main_width  = ImGui::GetWindowWidth();
        const float main_height = ImGui::GetWindowHeight();

        gui.re_size.inc_x = (!gui.re_size.inc_x) && (old_width != main_width);
        gui.re_size.inc_y = (!gui.re_size.inc_y) && (old_height != main_height);
        old_width         = main_width;
        old_height        = main_height;

        ImGui::EndChild();
    }

}
