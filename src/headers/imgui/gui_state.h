//
// Created by xd on 02/01/25.
//

#ifndef GUI_STATE_H
#define GUI_STATE_H

namespace ex::gui {

    struct Docking {
        unsigned int dock_left_id     = 0;
        unsigned int dock_left_top_id = 0;
        unsigned int dock_left_bot_id = 0;

        unsigned int dock_right_id = 0;
    };

    struct View {
        float pane_width_left     = 350.0f;
        float pane_width_right    = 350.0f;
        float pane_height_bottom  = 200.0f;
        float pane_height_content = 0.f;

        float main_width  = 0.f;
        float main_height = 0.f;
        float main_free_x = 0.f;
        float main_free_y = 0.f;

        bool show_debug = false;
        bool show_right = true;
        bool show_left  = true;
        bool show_logs  = true;
        bool show_menu  = true;

        bool highlight_logs = true;
        bool expanded_logs  = false;
        bool follow_logs    = false;

        bool reset   = false;
        bool docking = false;

        bool first_run = true;
    };

    struct ReSize {
        bool inc_x = false;
        bool inc_y = false;
    };

    struct Window {
        bool resized = false;
        bool reset   = false;
        bool exit    = false;
    };

    struct FileDialogs {
        bool project_save = false;
        bool project_open = false;
        bool project_new  = false;
    };

    struct GuiState {
        FileDialogs file_dialogs;
        Docking     docking;
        Window      window;
        ReSize      re_size;
        View        view;
    };

}

#endif //GUI_STATE_H
