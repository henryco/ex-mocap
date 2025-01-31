//
// Created by xd on 31/01/25.
//

#include "../../headers/imgui/gui_globals.h"

namespace ex::gui {

    logs::logger &logger() {
        static ex::logs::logger log = ex::logs::get_logger("gui_layout");
        return log;
    }

    ex::undo::history<ex::State> &undo_history() {
        static ex::undo::history<ex::State> undo_history("Blank");
        return undo_history;
    }

    keys::KeyMapping &key_mappings() {
        static keys::KeyMapping instance;
        return instance;
    }

}
