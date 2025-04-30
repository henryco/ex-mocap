//
// Created by xd on 31/01/25.
//

#ifndef GUI_GLOBALS_H
#define GUI_GLOBALS_H

#include "../../../extra/undo/undo_history.h"
#include "../utils/log_utils.h"
#include "../imgui/gui_state.h"
#include "../state.h"

#include "actions/gui_actions_keymap.h"

namespace ex::gui {

    using id_type  = std::uint64_t;

    GuiState &state();

    logs::logger &logger();

    actions::keys::Mappings &key_mappings();

    limbo::undo::history &undo_history();
}

#endif //GUI_GLOBALS_H
