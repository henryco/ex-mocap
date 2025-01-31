//
// Created by xd on 31/01/25.
//

#include "../../headers/imgui/gui_globals.h"
#include "../../headers/imgui/cmp/test_cmp.h"

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

    const std::map<std::size_t, ex::gui::NodeComponent> &node_components() {
        // TODO REPLACE WITH REAL COMPONENTS
        static std::map<std::size_t, ex::gui::NodeComponent> instance = {
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
