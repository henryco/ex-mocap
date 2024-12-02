//
// Created by xd on 02/12/24.
//

#ifndef EX_MOTION_GUI_LAYOUT_H
#define EX_MOTION_GUI_LAYOUT_H

#include "../state.h"

#include <IconsLucide.h>
#include <imgui.h>

namespace ex::gui {

#define cond_node() (view.reset ? ImGuiCond_Always : ImGuiCond_Once)

    typedef struct Docking {
        ImGuiID dock_left_id = 0;
        ImGuiID dock_left_top_id = 0;
        ImGuiID dock_left_bot_id = 0;

        ImGuiID dock_right_id = 0;
    } Docking;

    typedef struct View {
        float pane_width_left     = 350.0f;
        float pane_width_right    = 350.0f;
        float pane_height_bottom  = 200.0f;
        float pane_height_content = 0.f;

        float main_width  = 0.f;
        float main_height = 0.f;
        float main_free_x = 0.f;
        float main_free_y = 0.f;

        bool show_right = true;
        bool show_left  = true;
        bool show_logs  = true;
        bool show_menu  = true;

        bool highlight_logs = true;
        bool expanded_logs = false;
        bool follow_logs = false;
        bool should_exit = false;

        bool reset = false;
        bool docking = false;

        bool first_run = true;
    } View;

    typedef struct ReSize {
        bool inc_x = false;
        bool inc_y = false;
    } ReSize;

    static constexpr float offset_main_h = 8.f;
    static constexpr float offset_main_v = 23.f;
    static constexpr float offset_logs_v = 4.0f;

    static bool window_resized = false;
    static bool reset_layout = false;

    static Docking docking = Docking{};
    static View view = View{};

    namespace layout {
        /**
         * @return should exit
         */
        bool build(ex::State &state);
    }
}

#endif // EX_MOTION_GUI_LAYOUT_H
