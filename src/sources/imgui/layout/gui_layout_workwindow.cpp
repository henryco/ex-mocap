//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_layout.h"

namespace ex::gui {

    void show_work_window(ex::State &state, GuiState &gui) {
        static bool docked = false;
        static bool show = true;

        if (!show)
            return;

        static bool *watch = nullptr;
        if (watch != nullptr && !(*watch))
            return;

        bool *ptr = (!docked || gui.view.docking) ? &show : nullptr;

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(gui.docking.dock_right_id, cond_node());

        // TODO
        ImGui::Begin("My Window3", ptr, reset_flag(gui, true) | ImGuiWindowFlags_NoCollapse);
        docked = ex::gui::window_is_docked();
        watch = watch_parent_dock(gui);
        ImGui::Text("This window 3: %d | %d", docked, gui.view.docking);
        ImGui::End();
        // TODO
    }

}