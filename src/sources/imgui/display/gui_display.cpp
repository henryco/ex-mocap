//
// Created by xd on 2/23/25.
//

#include "../../../headers/imgui/display/gui_display.h"
#include "../../../headers/imgui/display/imp.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/utils/data_utils.h"
#include "../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>
#include <iostream>

namespace ex::display::imp::imgui {

    static unsigned int id_counter = 0;

    struct display_window_t {
        const char *name;
        const char *uuid;
        int         width;
        int         height;
        int         channels;
        int         depth;
    };

    static ex::data::map<unsigned int, display_window_t> display_map_(10);

    unsigned int add_output(const char *name, const int width, const int height, const int channels, const int depth) {
        const auto new_id = ++id_counter;
        const auto uuid = new char[strlen(name) + 24];
        sprintf(uuid, "%s##display/%03d", name, new_id);

        std::cout << "display: " << uuid << '\n';

        display_map_.put(new_id, { name, uuid, width, height, channels, depth });
        return new_id;
    }

    void remove_output(const unsigned int id) {
        if (!display_map_.contains(id))
            return;
        const auto &d_ref = display_map_[id];
        if (d_ref.uuid != nullptr)
            delete[] d_ref.uuid;
        display_map_.remove(id);
    }

}

namespace ex::gui::display {

    void debug_node_host_(ImGuiDockNode *dock_host_node) {
        ImGui::Begin("Dock Debug Window");
        ImGui::DebugNodeDockNode(dock_host_node, "Debug");
        ImGui::End();
    }

    ImGuiWindowClass create_display_dock_host_class_() {
        ImGuiWindowClass class_;
        class_.ClassId = 111777111;
        class_.DockingAllowUnclassed = false;
        class_.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        return class_;
    }

    inline int dock_host_flag_(GuiState &gui) {
        constexpr int docked = ImGuiDockNodeFlags_NoUndocking;
        return 0 | (gui.view.docking ? 0 : (docked))
#ifndef EXM_DISABLE_EXPERIMENTAL
               | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton
#endif
               | ImGuiDockNodeFlags_PassthruCentralNode;
    }

    static const ImGuiWindowClass display_dock_host_class_ = create_display_dock_host_class_();

    bool draw_display(const ImGuiWindow *host_window, const ex::display::imp::imgui::display_window_t &disp) {
        static const GuiState &gui = ex::gui::state();
        ImGui::SetNextWindowClass(&display_dock_host_class_);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        if (ImGui::Begin(disp.uuid)) {
            ImGui::Text("%s", disp.name);
            // TODO
        }

        if (const ImGuiWindow *curr = ImGui::GetCurrentWindow()) {
            if (curr->ParentWindow == host_window) {
                ImGui::End();
                return true;
            }
        }

        ImGui::End();
        return false;
    }

    void draw_displays_(const ImGuiWindow *host_window, bool *docking_empty) {
        namespace edii = ex::display::imp::imgui;
        bool docked = false;

        for (int i = 0; i < edii::display_map_.size; i++) {
            if (draw_display(host_window, edii::display_map_.val_entries[i]))
                docked = true;
        }

        *docking_empty = !docked;
    }

    void show_display() {
        static GuiState &gui = ex::gui::state();
        static bool docking_empty = false;

        const int flags = docking_empty ? (ImGuiDockNodeFlags_NoDockingSplit) : 0;
        const int aux_flags = dock_host_flag_(gui);

        ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.f, 0.f, 0.f, .0f));

        gui.docking.dock_main_id = ImGui::GetID("DockMainDisplay");
        ImGui::DockSpace(
            gui.docking.dock_main_id,
            ImVec2(0.0f, 0.0f),
            ImGuiDockNodeFlags_None | aux_flags | flags,
            &display_dock_host_class_);

        ImGui::PopStyleColor();

        const auto dock_host_node = ImGui::DockBuilderGetNode(gui.docking.dock_main_id);
        dock_host_node->LocalFlags |= ImGuiDockNodeFlags_NoDockingSplit;

        draw_displays_(dock_host_node->HostWindow, &docking_empty);
//        debug_node_host_(dock_host_node);
    }

}