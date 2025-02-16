//
// Created by xd on 04/01/25.
//

#include "../../../headers/imgui/gui_globals.h"
#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/data_utils.h"
#include "../../../headers/utils/gui_utils.h"

#include <cstring>
#include <string>

namespace ex::gui {

    void err_abort(const char *msg = nullptr) {
        if (msg != nullptr)
            fprintf(stderr, "%s\n", msg);
        abort();
    }

    struct Box {
        GuiNodeWindow window;
        std::uint32_t id;

        ~Box() {
            std::cout << "~Box: " << window.uuid << std::endl;

            if (window.uuid != nullptr)
                delete window.uuid;
        }
    };

    void gen_uuid(GuiNodeWindow &window) {
        std::cout << "gen_uuid: " << window.node->name << std::endl;

        const auto s_uuid =
                std::string(std::string(window.node->name) + "##" + std::to_string(window.node->id_local) + "/" +
                            std::to_string(window.node->id_node));
        const auto buffer = new char[s_uuid.size() + 1];
        strcpy(buffer, s_uuid.c_str());
        if (window.uuid != nullptr)
            delete window.uuid;
        window.uuid = buffer;
    }

    GuiNodeWindow win_from_node(TreeNode *node) {
        if (node == nullptr) {
            err_abort("Tree node cannot be null");
            return {};
        }

        std::cout << "win_from_node: " << node->name << std::endl;

        auto win = GuiNodeWindow{ .name = node->name, .node = node };
        gen_uuid(win);
        return win;
    }

    Box create_new_box(const State &state, TreeNode *node) {
        if (node == nullptr) {
            err_abort("Tree node cannot be null");
            return {};
        }

        std::cout << "create_new_box: " << node->name << std::endl;

        if (node->state >>= node_state::is_link) {
            const auto source = TreeNode::find_node(state.logic.work_tree, node->id_link);
            const auto tr_src = (source == nullptr) ? node : source;
            return { .window = win_from_node(tr_src), .id = tr_src->id_local };
        }
        return { .window = win_from_node(node), .id = node->id_local };
    }

    void show_work_window() {
        static auto     &node_mappers_ = node_components();
        static State    &state         = ex::logic::globals::state();
        static GuiState &gui           = ex::gui::state();

        static ex::data::array<Box> windows_(5);

        const bool dirty_state = state.logic.dirty;
        TreeNode  *active_node = state.logic.active_node;

        const Box *active_one = nullptr;
        for (int i = 0; i < windows_.size; ++i) {
            Box &box = windows_[i];

            if (active_node != nullptr && box.window.node == active_node) {
                active_one = &box;
            }

            if (dirty_state) {
                box.window.node = TreeNode::find_node(state.logic.work_tree, box.id);
                if (&box == active_one)
                    state.logic.active_node = box.window.node;
                if (box.window.node != nullptr)
                    gen_uuid(box.window);
            }

            if (box.window.node == nullptr) {
                if (&box == active_one) {
                    state.logic.active_node = nullptr;
                    active_one = nullptr;
                }
                windows_.swap_remove(i--);
                continue;
            }

            if (box.window.node->state >>= node_state::deleted) {
                if (&box == active_one) {
                    state.logic.active_node = nullptr;
                    active_one = nullptr;
                }
                windows_.swap_remove(i--);
                continue;
            }

            if (!box.window.pinned && !box.window.docked && (&box) != active_one) {
                windows_.swap_remove(i--);
                continue;
            }

            if (!node_mappers_.contains(box.window.node->id_node)) {
                if (&box == active_one) {
                    state.logic.active_node = nullptr;
                    active_one = nullptr;
                }
                windows_.swap_remove(i--);
            }
        }

        if (active_node != nullptr && active_one == nullptr) {
            windows_.push(create_new_box(state, active_node));
        }

        for (int i = 0; i < windows_.size; ++i) {
            Box &box = windows_[i];
            const auto render_fn = node_mappers_.at(box.window.node->id_node).draw_node_window;

            if (render_fn == nullptr || !(render_fn(gui, state, &box.window))) {
                if (&box == active_one) {
                    state.logic.active_node = nullptr;
                    active_one = nullptr;
                }
                windows_.swap_remove(i--);
                continue;
            }

            box.window.pinned |= !box.window.docked;
        }
    }

}