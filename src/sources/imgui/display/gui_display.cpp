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
        static constexpr int formats_[] = {
            GL_SINGLE_COLOR,
            GL_RG,
            GL_BGR,
            GL_BGRA
        };
        return formats_[c - 1];
    }

    #define PBO_COUNT (3)
    #define PBO_TIMEOUT (1'000'000)
    struct pbo_t {
        const void *data_ptr;

        void  *pbo_pt[PBO_COUNT];
        GLsync fences[PBO_COUNT];
        GLuint pbo_id[PBO_COUNT];
        GLuint texture;
        ushort channels;
        ushort width;
        ushort height;
        char   index;
    };

    struct meta_t {
        const char *uuid;
        const char *name;
        char        bg_c;
    };

    struct state_t {
        bool to_prepare;
        bool to_clear;
        bool to_flush;
        bool to_draw;
    };

    struct display_window_t {
        pbo_t   pbo;
        meta_t  meta;
        state_t state;
    };

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

    display_window_t rv_display_window(const char *uuid, const char *name) {
        display_window_t window;
        ::memset(&window, 0, sizeof(display_window_t));
        window.meta.name = name;
        window.meta.uuid = uuid;
        return window;
    }

    void release_state(state_t *pt) {
        ::memset(pt, 0, sizeof(state_t));
    }

    void release_meta(meta_t *pt) {
        if (pt->uuid != nullptr)
            ::free((void *) pt->uuid);
        if (pt->name != nullptr)
            ::free((void *) pt->name);
        pt->uuid = nullptr;
        pt->name = nullptr;
    }

    void release_display(display_window_t *p) {
        if (p == nullptr)
            return;
        release_buffers(&p->pbo);
        release_meta(&p->meta);
        release_state(&p->state);
    }
}

namespace ex::display::imp::imgui {

    static ex::data::array_map<unsigned int, display_window_t> display_map_(10);

    unsigned int add_output(const char *name) {
        static unsigned int id_counter = 0;

        const unsigned new_id = ++id_counter;
        const unsigned size = strlen(name);

        char *const uuid  = static_cast<char *>(::malloc(size + 24));
        char *const label = static_cast<char *>(::malloc(size + 1));

        ::memset(uuid, 0, size + 24);
        ::sprintf(uuid, "%s###display%d", name, new_id);
        uuid[size + 23] = '\0';

        ::memset(label, 0, size + 1);
        ::strcpy(label, name);
        label[size] = '\0';

        display_map_.put(new_id, rv_display_window(uuid, label));
        return new_id;
    }

    void rename_output(const unsigned int id, const char *name) {
        if (!display_map_.contains(id))
            return;

        auto &d_ref = display_map_[id];
        release_meta(&d_ref.meta);

        const unsigned size = strlen(name);

        char *const uuid  = static_cast<char *>(::malloc(size + 24));
        char *const label = static_cast<char *>(::malloc(size + 1));

        ::memset(uuid, 0, size + 24);
        ::sprintf(uuid, "%s###display%d", name, id);
        uuid[size + 23] = '\0';

        ::memset(label, 0, size + 1);
        ::strcpy(label, name);
        label[size] = '\0';

        d_ref.meta.name = label;
        d_ref.meta.uuid = uuid;
    }

    void remove_output(const unsigned int id) {
        if (!display_map_.contains(id))
            return;
        lock_.lock();
        release_display(&display_map_[id]);
        display_map_.remove(id);
        lock_.unlock();
    }

    void clear(const unsigned int id) {
        lock_.lock();
        display_window_t &ref = display_map_.at(id);
        release_state(&ref.state);
        ref.state.to_clear = true;
        lock_.unlock();
    }

    void prepare_bgra_8(unsigned int id, int w, int h, int cn) {
        if (w <= 0 || h <= 0)
            abort_("w <= 0 || h <= 0");
        lock_.lock();
        display_window_t &ref = display_map_.at(id);
        release_state(&ref.state);
        ref.state.to_prepare = true;
        ref.pbo.channels     = cn;
        ref.pbo.height       = h;
        ref.pbo.width        = w;
        lock_.unlock();
    }

    bool draw_bytes_bgra_8(const unsigned int id, const void *data) {
        lock_.lock();
        display_window_t &ref = display_map_.at(id);

        if (ref.pbo.texture == 0) {
            lock_.unlock();
            return false;
        }

        release_state(&ref.state);
        ref.state.to_draw = true;
        ref.pbo.data_ptr  = data;
        lock_.unlock();
        return true;
    }

    void *direct_bgra_8(unsigned int id) {
        lock_.lock();
        display_window_t &ref = display_map_.at(id);

        if (ref.pbo.texture == 0) {
            lock_.unlock();
            return nullptr;
        }

        lock_.unlock();
        return ref.pbo.pbo_pt[ref.pbo.index];
    }

    bool flush_bgra_8(unsigned int id) {
        lock_.lock();
        display_window_t &ref = display_map_.at(id);

        if (ref.pbo.texture == 0) {
            lock_.unlock();
            return false;
        }

        release_state(&ref.state);
        ref.pbo.index = (ref.pbo.index + 1) % PBO_COUNT;
        ref.state.to_flush = true;

        lock_.unlock();
        return true;
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

        const ImVec4 &custom_bg = bg_color_(disp.meta.bg_c);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, custom_bg);

        if (ImGui::Begin(disp.meta.uuid, nullptr, ImGuiWindowFlags_NoFocusOnAppearing)) {
            const float pos_y_0 = ImGui::GetCursorPosY();

            if (disp.state.to_clear) {
                release_buffers(&disp.pbo);
                release_state(&disp.state);
                goto skip_point;
            }

            if (disp.state.to_prepare) {
                release_buffers(&disp.pbo);
                init_buffers(&disp.pbo);
                release_state(&disp.state);
                goto skip_point;
            }

            if (disp.state.to_draw) {
                release_state(&disp.state);
                if (disp.pbo.data_ptr == nullptr)
                    goto skip_point;

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
                ::memcpy(buff.pbo_pt[index], disp.pbo.data_ptr, size);

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

                // clear old data
                disp.pbo.data_ptr = nullptr;
                lock_.unlock();
                goto skip_point;
            }

            if (disp.state.to_flush) {
                release_state(&disp.state);

                pbo_t &buff  = disp.pbo;
                char   next  = buff.index;
                char   index = (next - 1 + PBO_COUNT) % PBO_COUNT;

                const unsigned width    = buff.width;
                const unsigned height   = buff.height;
                const unsigned channels = buff.channels;

                lock_.lock();

                if (buff.fences[index]) {
                    GLenum result = glClientWaitSync(buff.fences[index], GL_SYNC_FLUSH_COMMANDS_BIT, PBO_TIMEOUT);
                    if (result == GL_TIMEOUT_EXPIRED) {
                        // TODO: GPU SLOW?
                    }
                    glDeleteSync(buff.fences[index]);
                    buff.fences[index] = 0;
                }

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
                if (buff.fences[next])
                    glDeleteSync(buff.fences[next]);
                buff.fences[next] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

                lock_.unlock();
            }

            skip_point:

            const float window_width  = ImGui::GetContentRegionAvail().x;
            const float window_height = ImGui::GetContentRegionAvail().y;

            if (disp.pbo.texture != 0) {
                // TODO: MOVE TO SETTINGS
                constexpr bool should_fit = true;

                const ImVec2 img_size = img_size_(
                        disp.pbo.width,
                        disp.pbo.height,
                        window_width,
                        window_height,
                        should_fit
                );

                {
                    // TODO: MOVE TO SETTINGS
                    const float x_offset = (window_width - img_size.x) * 0.5f;
                    const float y_offset = (window_height - img_size.y) * 0.5f;
                    if (x_offset > 0.0f)
                        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x_offset);
                    if (y_offset > 0.0f)
                        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y_offset);
                }

                ImGui::Image(disp.pbo.texture, img_size);
            }

            if (ImGui::IsWindowHovered(ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {

                ImGui::SetCursorPosX(window_width - ImGui::CalcTextSize(ICON_LC_PALETTE).x);
                ImGui::SetCursorPosY(pos_y_0);
                if (switch_button(ICON_LC_PALETTE)) {
                    switch_bg_color_(&disp.meta.bg_c);
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