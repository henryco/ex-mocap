//
// Created by xd on 04/01/25.
//

#include "../../../extra/data/data_utils.h"
#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/gui_mappers.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gui_utils.h"

#include <cstring>

namespace ex::gui {

    void err_abort(const char *msg = nullptr) {
        if (msg != nullptr)
            fprintf(stderr, "%s\n", msg);
        abort();
    }

    inline bool *watch_parent_dock_(GuiState &gui) {
        if (ex::gui::window_withing_dock(gui.docking.dock_left_id))
            return &gui.view.show_left;
        if (ex::gui::window_withing_dock(gui.docking.dock_right_id))
            return &gui.view.show_right;
        return nullptr;
    }

    struct Box {

        GuiNodeWindow window;
        std::uint32_t id;

        Box() = default;

        Box(const GuiNodeWindow &window, const std::uint32_t &id) {
            this->window = window;
            this->id     = id;
        }

        Box(const Box &other) {
            window = other.window;
            id     = other.id;
            if (other.window.uuid != nullptr) {
                const auto buffer = static_cast<char *>(::malloc(strlen(other.window.uuid) + 1));
                strcpy(buffer, other.window.uuid);
                window.uuid = buffer;
            }
        }

        Box(Box &&other) noexcept {
            window = other.window;
            id     = other.id;

            other.window.visible = nullptr;
            other.window.uuid    = nullptr;
            other.window.name    = nullptr;
            other.window.node    = nullptr;
        }

        Box &operator = (const Box &other) {
            if (this == &other)
                return *this;

            window = other.window;
            id     = other.id;

            if (other.window.uuid != nullptr) {
                const auto buffer = static_cast<char *>(::malloc(strlen(other.window.uuid) + 1));
                strcpy(buffer, other.window.uuid);
                window.uuid = buffer;
            }

            return *this;
        }

        Box &operator = (Box &&other) noexcept {
            if (this == &other)
                return *this;

            if (window.uuid != nullptr)
                ::free((void *) window.uuid);

            window = other.window;
            id     = other.id;

            other.window.visible = nullptr;
            other.window.uuid    = nullptr;
            other.window.name    = nullptr;
            other.window.node    = nullptr;

            return *this;
        }

        ~Box() {
            if (window.uuid != nullptr)
                ::free((void *) window.uuid);
        }
    };

    void gen_uuid(GuiNodeWindow &window) {
        const auto buffer = static_cast<char *>(::malloc(strlen(window.node->name) + 64));
        sprintf(buffer, "%s##window/%lu/%u", window.node->name, window.node->id_type, window.node->id_node);
        if (window.uuid != nullptr)
            ::free((void *) window.uuid);
        window.uuid = buffer;
    }

    GuiNodeWindow win_from_node(TreeNode *node) {
        if (node == nullptr) {
            err_abort("Tree node cannot be null");
            return {};
        }

        auto win = GuiNodeWindow{ .name = node->name, .node = node };
        gen_uuid(win);
        return win;
    }

    Box create_new_box(TreeNode *node) {
        if (node == nullptr) {
            err_abort("Tree node cannot be null");
            return {};
        }

        if (node->state >>= node_state::link) {
            const auto source = ex::entity::find(node->id_link);
            const auto tr_src = (source == nullptr) ? node : source;
            return { win_from_node(tr_src), tr_src->id_node };
        }
        return { win_from_node(node), node->id_node };
    }

    bool draw_window(Box &box) {
        static auto     &node_mappers_ = node_mappers();
        static GuiState &gui           = ex::gui::state();

        if (box.window.node->state >>= node_state::group)
            return false; // TODO

        const auto &mapper    = node_mappers_[box.window.node->id_type];
        const auto  render_fn  = mapper.draw_node_window;
        const auto  dock_id_fn = mapper.get_dock_id;

        if (render_fn == nullptr)
            return false;

        GuiNodeWindow &node = box.window;
        if (node.visible != nullptr && !(*node.visible))
            return false;

        if (node.name != node.node->name) {
            node.name = node.node->name;
            gen_uuid(node);
        }

        bool show = true;
        bool *ptr = (node.pinned || !node.docked || gui.view.docking) ? &show : nullptr;

        const uint32_t dock_id = (dock_id_fn == nullptr) ? gui.docking.dock_right_id : dock_id_fn();

        ex::gui::next_no_close();
        ImGui::SetNextWindowDockID(dock_id, gui.view.reset ? ImGuiCond_Always : ImGuiCond_Once);
        ex::gui::begin_docked_window(node.uuid, ptr, gui.view.reset, true);
        node.docked = ex::gui::window_is_docked();
        node.visible = watch_parent_dock_(gui);

        const bool rendered = render_fn(&box.window);

        ImGui::End();

        box.window.pinned |= !box.window.docked;

        if (!rendered)
            return false;
        return show;
    }

    void show_work_window() {
        static auto     &node_mappers_ = node_mappers();
        static State    &state         = ex::logic::globals::state();

        static ex::data::array<Box> windows_(5);

        const bool dirty_state = ex::gui::is_dirty();
        TreeNode  *active_node = ex::entity::find(state.logic.active_node);

        const Box *active_one = nullptr;
        for (int i = 0; i < windows_.size; ++i) {
            Box &box = windows_[i];

            if (active_node != nullptr && (box.window.node == active_node || box.id == active_node->id_node)) {
                active_one = &box;
            }

            if (box.id == ID32_UNSET) {
                active_node = nullptr;
                active_one  = nullptr;
                windows_.swap_remove(i--);
                continue;
            }

            if (dirty_state) {
                box.window.node = ex::entity::find(box.id);
                if (&box == active_one) {
                    state.logic.active_node = box.window.node->id_node;
                    active_node             = box.window.node;
                }
                if (box.window.node != nullptr)
                    gen_uuid(box.window);
            }

            if (box.window.node == nullptr) {
                if (&box == active_one) {
                    state.logic.active_node = -1;
                    active_node             = nullptr;
                    active_one              = nullptr;
                }
                windows_.swap_remove(i--);
                continue;
            }

            if (box.window.node->state >>= node_state::deleted) {
                if (&box == active_one) {
                    state.logic.active_node = -1;
                    active_node             = nullptr;
                    active_one              = nullptr;
                }
                windows_.swap_remove(i--);
                continue;
            }

            if (!box.window.pinned && (&box) != active_one) {
                windows_.swap_remove(i--);
                continue;
            }

            if (!node_mappers_.contains(box.window.node->id_type)) {
                if (&box == active_one) {
                    state.logic.active_node = -1;
                    active_node             = nullptr;
                    active_one              = nullptr;
                }
                windows_.swap_remove(i--);
            }
        }

        if (active_node != nullptr && active_one == nullptr) {
            bool contains = false;
            for (int i = 0; i < windows_.size; ++i) {
                const Box &box = windows_[i];
                if (box.window.node == active_node || box.id == active_node->id_node) {
                    contains = true;
                    break;
                }
            }

            if (!contains) {
                if (!(active_node->state >>= node_state::deleted) &&
                    !(active_node->state >>= node_state::group)) {
                    // TODO
                    windows_.push(create_new_box(active_node));
                }
                active_one = &(windows_[windows_.size - 1]);
            }
        }

        for (int i = 0; i < windows_.size; ++i) {
            Box &box = windows_[i];

            if (draw_window(box))
                continue;

            if (&box == active_one) {
                state.logic.active_node = -1;
                active_one              = nullptr;
            }

            windows_.swap_remove(i--);
        }
    }

}