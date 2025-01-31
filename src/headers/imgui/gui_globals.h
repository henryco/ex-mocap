//
// Created by xd on 31/01/25.
//

#ifndef GUI_GLOBALS_H
#define GUI_GLOBALS_H

#include "gui_keymap.h"
#include "../state.h"
#include "../undo/undo_history.h"
#include "../utils/log_utils.h"

namespace ex::gui {
    undo::history<ex::State> &undo_history();

    logs::logger &logger();

    keys::KeyMapping &key_mappings();
}

#endif //GUI_GLOBALS_H
