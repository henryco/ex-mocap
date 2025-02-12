//
// Created by xd on 31/01/25.
//

#ifndef GUI_GLOBALS_H
#define GUI_GLOBALS_H

#include <map>

#include "../state.h"
#include "../../../extra/undo/undo_history.h"
#include "../utils/log_utils.h"

#include "gui_keymap.h"
#include "gui_component.h"

namespace ex::gui {

    logs::logger &logger();

    keys::KeyMapping &key_mappings();

    limbo::undo::history<ex::State> &undo_history();

    const std::map<std::size_t, ex::gui::GuiNodeComponent> &node_components();

}

#endif //GUI_GLOBALS_H
