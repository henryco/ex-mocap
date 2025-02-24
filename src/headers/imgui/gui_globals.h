//
// Created by xd on 31/01/25.
//

#ifndef GUI_GLOBALS_H
#define GUI_GLOBALS_H

#include "../../../extra/undo/undo_history.h"
#include "../utils/log_utils.h"
#include "../utils/data_utils.h"
#include "../imgui/gui_state.h"
#include "../state.h"

#include "actions/gui_actions_keymap.h"
#include "nodes/gui_nodes.h"

namespace ex::gui {

    using id_type  = std::uint64_t;

    GuiState &state();

    logs::logger &logger();

    actions::keys::Mappings &key_mappings();

    limbo::undo::history<ex::State> &undo_history();

    const data::map<id_type, gui::GuiNodeMapper> &node_outputs();

    /**
     * Offset by N node_outputs
     */
    const data::map<id_type, gui::GuiNodeMapper> &node_components();
}

#endif //GUI_GLOBALS_H
