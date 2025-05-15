//
// Created by xd on 31/01/25.
//

#include "../../headers/imgui/gui_globals.h"
#include "../../headers/imgui/strings/gui_strings.h"

namespace ex::gui {

    namespace {
        bool dirty_state = false;
    }

    bool is_dirty() {
        return dirty_state;
    }

    void set_dirty(const bool is) {
        dirty_state = is;
    }

    GuiState &state() {
        static GuiState state;
        return state;
    }

    logs::logger &logger() {
        static ex::logs::logger log = ex::logs::get_logger("gui_layout");
        return log;
    }

    limbo::undo::history &undo_history() {
        static limbo::undo::history undo_history(strings::gui_tree_ops_history_blank);
        return undo_history;
    }

    actions::keys::Mappings &key_mappings() {
        static actions::keys::Mappings instance;
        return instance;
    }

} // namespace ex::gui
