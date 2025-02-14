//
// Created by xd on 31/01/25.
//

#include "../../headers/imgui/gui_globals.h"
#include "../../headers/imgui/components/test_cmp.h"

namespace ex::gui {

    GuiState &state() {
        static GuiState state;
        return state;
    }

    logs::logger &logger() {
        static ex::logs::logger log = ex::logs::get_logger("gui_layout");
        return log;
    }

    limbo::undo::history<ex::State> &undo_history() {
        static limbo::undo::history<ex::State> undo_history("Blank");
        return undo_history;
    }

    keys::KeyMapping &key_mappings() {
        static keys::KeyMapping instance;
        return instance;
    }

    const std::map<std::size_t, ex::gui::GuiNodeComponent> &node_components() {
        // TODO REPLACE WITH REAL COMPONENTS
        static std::map<std::size_t, ex::gui::GuiNodeComponent> instance = {
            { 0, ex::gui::test::one::gui_mappers },
            { 1, ex::gui::test::tri::gui_mappers },
            { 2, ex::gui::test::two::gui_mappers },
            { 3, ex::gui::test::one::gui_mappers },
            { 4, ex::gui::test::tri::gui_mappers },
            { 5, ex::gui::test::two::gui_mappers },
            { 6, ex::gui::test::one::gui_mappers },
        };
        return instance;
    }

} // namespace ex::gui
