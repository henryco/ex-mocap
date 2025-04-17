//
// Created by xd on 2/23/25.
//

#include "../../../extra/data/struct/array_map.h"
#include "../../../headers/imgui/display/gui_display.h"
#include "../../../headers/imgui/display/imp.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/utils/gui_utils.h"

#include <imgui_internal.h>

namespace ex::display::imp::imgui {

    int format_c(const int c) {
        if (c == 3)
            return GL_BGR;
        if (c == 4)
            return GL_BGRA;
        if (c == 1)
            return GL_SINGLE_COLOR;
        abort_("channels number invalid: " << c);
    }

    GLuint create_texture_bgra(const int width, const int height, const int channels) {
        GLuint texture_id;
        glGenTextures(1, &texture_id);
        glBindTexture(GL_TEXTURE_2D, texture_id);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // or GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // or GL_NEAREST

        GLint format_e, format_i;
        if (channels == 1) {
            format_e = GL_SINGLE_COLOR;
            format_i = GL_SINGLE_COLOR;
        } else if (channels == 3) {
            format_e = GL_BGR;
            format_i = GL_RGB;
        } else if (channels == 4) {
            format_e = GL_BGRA;
            format_i = GL_RGBA;
        } else {
            abort_("channels number invalid: " << channels);
        }

        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     format_i,
                     width,
                     height,
                     0,
                     format_e,
                     GL_UNSIGNED_BYTE,
                     nullptr);

        glBindTexture(GL_TEXTURE_2D, 0);

        return texture_id;
    }

    struct display_window_t {
        const char    *name;
        const char    *uuid;
        unsigned char *data;
        // todo cl_mem handle
        GLuint         txid;
        int            w;
        int            h;
        int            c;
        bool           u;
    };

    static ex::data::array_map<unsigned int, display_window_t> display_map_(10);

    unsigned int add_output(const char *name) {
        static unsigned int id_counter = 0;

        const auto new_id = ++id_counter;
        const auto uuid   = new char[strlen(name) + 24];
        const auto label  = new char[strlen(name) + 1];

        sprintf(uuid, "%s##display/%d", name, new_id);
        strcpy(label, name);

        display_map_.put(new_id, { label, uuid, nullptr, 0, 0, 0, 0, true });
        return new_id;
    }

    void remove_output(const unsigned int id) {
        if (!display_map_.contains(id))
            return;
        const auto &d_ref = display_map_[id];
        if (d_ref.uuid != nullptr)
            delete[] d_ref.uuid;
        if (d_ref.name != nullptr)
            delete[] d_ref.name;
        if (d_ref.txid != 0)
            glDeleteTextures(1, &d_ref.txid);
        display_map_.remove(id);
    }

    void clear(const unsigned int id) {
        display_window_t &ref = display_map_.at(id);
        if (ref.txid != 0)
            glDeleteTextures(1, &ref.txid);
        ref.txid = 0;
        ref.data = nullptr;
        ref.u    = false;
    }

    void draw_bytes_bgra_8(const unsigned int id, unsigned char *data, const int w, const int h, const int cn) {
        display_window_t &ref = display_map_.at(id);
        ref.data              = data;
        ref.c                 = cn;
        ref.w                 = w;
        ref.h                 = h;
        ref.u                 = true;

        if (w <= 0 || h <= 0)
            abort_("w <= 0 || h <= 0");
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

    bool draw_display(const ImGuiWindow *host_window, ex::display::imp::imgui::display_window_t &disp) {
        static const GuiState &gui = ex::gui::state();
        ImGui::SetNextWindowClass(&display_dock_host_class_);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        // CUSTOM BG COLOR, TODO: MOVE TO SETTINGS
        ImVec4 custom_bg = ImVec4(0.f, 0.f, 0.f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_bg);

        if (ImGui::Begin(disp.uuid, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {

            if (disp.u == true) {

                if (disp.txid == 0 && disp.data != nullptr) {
                    disp.txid = ex::display::imp::imgui::create_texture_bgra(disp.w, disp.h, disp.c);
                }

                if (disp.txid != 0 && disp.data != nullptr) {
                    glBindTexture(GL_TEXTURE_2D, disp.txid);
                    glTexSubImage2D(GL_TEXTURE_2D,
                                    0,
                                    0,
                                    0,
                                    disp.w,
                                    disp.h,
                                    ex::display::imp::imgui::format_c(disp.c),
                                    GL_UNSIGNED_BYTE,
                                    disp.data);
                    glBindTexture(GL_TEXTURE_2D, 0);
                }
            }

            if (disp.txid != 0) {

                {
                    // CENTERING, TODO: MOVE TO SETTINGS
                    const float window_width  = ImGui::GetContentRegionAvail().x;
                    const float window_height = ImGui::GetContentRegionAvail().y;
                    const float x_offset      = (window_width - disp.w)  * 0.5f;
                    const float y_offset      = (window_height - disp.h) * 0.5f;
                    if (x_offset > 0.0f)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);
                    if (y_offset > 0.0f)
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_offset);
                }

                ImGui::Image(disp.txid, { static_cast<float>(disp.w), static_cast<float>(disp.h) });
                disp.u = false;
            }
        }

        if (const ImGuiWindow *curr = ImGui::GetCurrentWindow()) {
            if (curr->ParentWindow == host_window) {
                ImGui::End();
                ImGui::PopStyleColor();
                return true;
            }
        }

        ImGui::End();
        ImGui::PopStyleColor();

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