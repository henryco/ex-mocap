//
// Created by xd on 2/23/25.
//

#include <GL/glew.h>

#include "../../../extra/data/struct/array_map.h"
#include "../../../headers/imgui/strings/gui_strings.h"
#include "../../../headers/imgui/display/gui_display.h"
#include "../../../headers/imgui/display/imp.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/utils/gui_utils.h"

#include "../../../../extra/data/struct/spinlock.h"

#include <imgui_internal.h>
#include <IconsLucide.h>

namespace {
    ex::data::spinlock lock_;

    constexpr ImVec4 BG_COLORS_[6] = {
        ImVec4(0, 0, 0, 1), // BLACK
        ImVec4(1, 1, 1, 1), // WHITE
        ImVec4(1, 0, 1, 1), // MAGENTA
        ImVec4(0, 1, 0, 1), // GREEN
        ImVec4(0, 0, 1, 1), // BLUE
        ImVec4(1, 0, 0, 1), // RED
    };

    void switch_bg_color_(char *idx) {
        if (idx == nullptr) return;
        *idx = ((*idx) + 1) % IM_ARRAYSIZE(BG_COLORS_);
    }

    const ImVec4 &bg_color_(const int idx) {
        return BG_COLORS_[idx % IM_ARRAYSIZE(BG_COLORS_)];
    }

    int format_c(const int c) {
        if (c == 3)
            return GL_BGR;
        if (c == 4)
            return GL_BGRA;
        if (c == 1)
            return GL_SINGLE_COLOR;
        abort_("channels number invalid: " << c);
    }

    #define PBO_COUNT (3)
    #define PBO_TIMEOUT (1'000'000)
    struct pbo_t {
        void  *pbo_pt[PBO_COUNT];
        GLsync fences[PBO_COUNT];
        GLuint pbo_id[PBO_COUNT];
        GLuint texture;
        ushort channels;
        ushort width;
        ushort height;

        char   index;
    };

    struct display_window_t {
        pbo_t          pbo;
        unsigned char *data;
        const char    *name;
        const char    *uuid;
        char           bg_color;
        bool           updated;
        bool           to_clear;
    };

    display_window_t rv_display_window(const char *uuid, const char *name) {
        display_window_t window;
        ::memset(&window, 0, sizeof(display_window_t));
        window.updated = true;
        window.name    = name;
        window.uuid    = uuid;
        return window;
    }

    void init_buffers(pbo_t *buffers) {
        static constexpr int flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;

        const unsigned width = buffers->width;
        const unsigned height = buffers->height;
        const unsigned channels = buffers->channels;
        const unsigned size = width * height * channels;

        glGenTextures(1, &buffers->texture);
        glBindTexture(GL_TEXTURE_2D, buffers->texture);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); // or GL_NEAREST
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // or GL_NEAREST

        GLint format_i;
        if (channels == 1)      format_i = GL_SINGLE_COLOR;
        else if (channels == 3) format_i = GL_RGB8;
        else if (channels == 4) format_i = GL_RGBA8;
        else abort_("channels number invalid: " << channels);

        glTexStorage2D(GL_TEXTURE_2D, 1, format_i, width, height);
        glBindTexture(GL_TEXTURE_2D, 0);

        glGenBuffers(PBO_COUNT, buffers->pbo_id);
        for (int i = 0; i < PBO_COUNT; ++i) {
            glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffers->pbo_id[i]);
            glBufferStorage(GL_PIXEL_UNPACK_BUFFER, size, nullptr, flags);
            buffers->pbo_pt[i] = glMapBufferRange(GL_PIXEL_UNPACK_BUFFER, 0, size, flags);
        }
    }

    void release_buffers(pbo_t *buffers) {
        if (buffers->texture == 0)
            return;

        for (int i = 0; i < PBO_COUNT; ++i) {
            if (buffers->fences[i]) {
                glClientWaitSync(buffers->fences[i], GL_SYNC_FLUSH_COMMANDS_BIT, PBO_TIMEOUT);
                glDeleteSync(buffers->fences[i]);
                buffers->fences[i] = 0;
            }

            if (buffers->pbo_pt[i]) {
                glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buffers->pbo_id[i]);
                glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);
                buffers->pbo_pt[i] = nullptr;
            }
        }

        glDeleteBuffers(PBO_COUNT, buffers->pbo_id);
        glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

        ::glDeleteTextures(1, &buffers->texture);
        ::memset(buffers->pbo_id, 0, sizeof(buffers->pbo_id));
        buffers->texture = 0;
        buffers->index   = 0;
    }
}

namespace ex::display::imp::imgui {

    static ex::data::array_map<unsigned int, display_window_t> display_map_(10);

    unsigned int add_output(const char *name) {
        static unsigned int id_counter = 0;

        const auto new_id = ++id_counter;
        const auto uuid   = new char[strlen(name) + 24];
        const auto label  = new char[strlen(name) + 1];

        sprintf(uuid, "%s###display%d", name, new_id);
        strcpy(label, name);

        display_map_.put(new_id, rv_display_window(uuid, label));
        return new_id;
    }

    void rename_output(const unsigned int id, const char *name) {
        if (!display_map_.contains(id))
            return;

        auto &d_ref = display_map_[id];

        const auto uuid  = new char[strlen(name) + 24];
        const auto label = new char[strlen(name) +  1];

        sprintf(uuid, "%s###display%d", name, id);
        strcpy(label, name);

        if (d_ref.uuid != nullptr)
            delete[] d_ref.uuid;
        if (d_ref.name != nullptr)
            delete[] d_ref.name;

        d_ref.name = label;
        d_ref.uuid = uuid;
    }

    void remove_output(const unsigned int id) {
        if (!display_map_.contains(id))
            return;
        lock_.lock();

        auto &d_ref = display_map_[id];
        release_buffers(&d_ref.pbo);

        if (d_ref.uuid != nullptr)
            delete[] d_ref.uuid;
        if (d_ref.name != nullptr)
            delete[] d_ref.name;

        display_map_.remove(id);
        lock_.unlock();
    }

    void clear(const unsigned int id) {
        lock_.lock();
        display_window_t &ref = display_map_.at(id);
        ref.data     = nullptr;
        ref.updated  = false;
        ref.to_clear = true;
        lock_.unlock();
    }

    void draw_bytes_bgra_8(const unsigned int id, unsigned char *data, const int w, const int h, const int cn) {
        lock_.lock();

        display_window_t &ref = display_map_.at(id);

        ref.pbo.width    = w;
        ref.pbo.height   = h;
        ref.pbo.channels = cn;
        ref.updated      = true;
        ref.data         = data;

        lock_.unlock();

        if (w <= 0 || h <= 0)
            abort_("w <= 0 || h <= 0");
    }
}

namespace ex::gui::display {

    static ImGuiWindowClass create_display_dock_host_class_() {
        ImGuiWindowClass class_;
        class_.ClassId = 111777111;
        class_.DockingAllowUnclassed = false;
        class_.DockNodeFlagsOverrideSet = ImGuiDockNodeFlags_NoCloseButton;
        return class_;
    }

    static int dock_host_flag_(GuiState &gui) {
        constexpr int docked = ImGuiDockNodeFlags_NoUndocking;
        return 0 | (gui.view.docking ? 0 : (docked))
#ifndef EXM_DISABLE_EXPERIMENTAL
               | ImGuiDockNodeFlags_NoWindowMenuButton | ImGuiDockNodeFlags_NoCloseButton
#endif
               | ImGuiDockNodeFlags_PassthruCentralNode;
    }

    static const ImGuiWindowClass display_dock_host_class_ = create_display_dock_host_class_();

    static float min_(const float a, const float b) {
        return a > b ? b : a;
    }

    static bool switch_button(const char *label, const bool def_state = false) {
        bool new_state = def_state;

        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(0, 0, 0, 255));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 11.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

        if (ImGui::Button(label)) {
            new_state = !new_state;
        }

        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(3);

        return new_state;
    }

    static ImVec2 img_size_(const float x, const float y, const float win_w, const float win_h, const bool fit) {
        if (!fit || (x == win_w && y == win_h))
            return { x, y };
        const float scale = min_(win_w / x, win_h / y);
        const float n_w = x * scale;
        const float n_h = y * scale;
        return { n_w, n_h };
    }

    static bool draw_display(const ImGuiWindow *host_window, display_window_t &disp) {
        static const GuiState &gui = ex::gui::state();
        ImGui::SetNextWindowClass(&display_dock_host_class_);
        ImGui::SetNextWindowDockID(gui.docking.dock_main_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);

        const ImVec4 &custom_bg = bg_color_(disp.bg_color);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_bg);

        if (ImGui::Begin(disp.uuid, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
            const float pos_y_0 = ImGui::GetCursorPosY();

            if (disp.to_clear == true) {
                release_buffers(&disp.pbo);
                disp.to_clear = false;
            }

            else if (disp.updated == true) {

                if (disp.pbo.texture == 0 && disp.data != nullptr) {
                    init_buffers(&disp.pbo);
                }

                if (disp.pbo.texture != 0 && disp.data != nullptr) {

                    pbo_t &buff  = disp.pbo;
                    char   index = disp.pbo.index;

                    const unsigned width    = buff.width;
                    const unsigned height   = buff.height;
                    const unsigned channels = buff.channels;
                    const unsigned size     = width * height * channels;

                    lock_.lock();

                    // sync
                    if (buff.fences[index]) {
                        GLenum result = glClientWaitSync(buff.fences[index], GL_SYNC_FLUSH_COMMANDS_BIT, PBO_TIMEOUT);
                        if (result == GL_TIMEOUT_EXPIRED) {
                            // TODO: GPU SLOW?
                        }
                        glDeleteSync(buff.fences[index]);
                        buff.fences[index] = 0;
                    }

                    // write
                    // TODO ASYNC WRITE (worker)
                    ::memcpy(buff.pbo_pt[index], disp.data, size);

                    // update
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, buff.pbo_id[index]);
                    glBindTexture(GL_TEXTURE_2D, buff.texture);
                    glTexSubImage2D(GL_TEXTURE_2D,
                                    0,
                                    0,
                                    0,
                                    width,
                                    height,
                                    format_c(channels),
                                    GL_UNSIGNED_BYTE,
                                    0
                    );
                    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);
                    glBindTexture(GL_TEXTURE_2D, 0);

                    // --- Insert fence after upload ---
                    buff.fences[index] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

                    // Advance ring index
                    disp.pbo.index = (index + 1) % PBO_COUNT;

                    lock_.unlock();
                }

            }

            const float window_width  = ImGui::GetContentRegionAvail().x;
            const float window_height = ImGui::GetContentRegionAvail().y;

            if (disp.pbo.texture != 0) {
                constexpr bool should_fit = true; // TODO: MOVE TO SETTINGS

                const ImVec2 img_size = img_size_(
                        disp.pbo.width,
                        disp.pbo.height,
                        window_width,
                        window_height,
                        should_fit
                );

                {
                    // CENTERING, TODO: MOVE TO SETTINGS
                    const float x_offset = (window_width - img_size.x) * 0.5f;
                    const float y_offset = (window_height - img_size.y) * 0.5f;
                    if (x_offset > 0.0f)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);
                    if (y_offset > 0.0f)
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_offset);
                }

                ImGui::Image(disp.pbo.texture, img_size);
                disp.updated = false;
            }

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {

                ImGui::SetCursorPosX(window_width - ImGui::CalcTextSize(ICON_LC_PALETTE).x);
                ImGui::SetCursorPosY(pos_y_0);
                if (switch_button(ICON_LC_PALETTE)) {
                    switch_bg_color_(&disp.bg_color);
                }
                ex::gui::plain_tooltip(strings::gui_display_button_bg_change);

                // TODO: BUTTON CENTER

                // TODO: BUTTON FIT
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

    static void draw_displays_(const ImGuiWindow *host_window, bool *docking_empty) {
        namespace edii = ex::display::imp::imgui;
        bool docked = false;

        for (int i = 0; i < edii::display_map_.size; i++) {
            if (draw_display(host_window, edii::display_map_.val_entries[i]))
                docked = true;
        }

        *docking_empty = !docked;
    }

//    static void debug_node_host_(ImGuiDockNode *dock_host_node) {
//        ImGui::Begin("Dock Debug Window");
//        ImGui::DebugNodeDockNode(dock_host_node, "Debug");
//        ImGui::End();
//    }

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