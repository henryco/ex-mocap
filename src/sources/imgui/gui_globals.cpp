//
// Created by xd on 31/01/25.
//

#include "../../headers/imgui/gui_globals.h"
#include "../../headers/imgui/gui_strings.h"
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
        static limbo::undo::history<ex::State> undo_history(strings::gui_tree_ops_history_blank);
        return undo_history;
    }

    keys::KeyMapping &key_mappings() {
        static keys::KeyMapping instance;
        return instance;
    }

    const data::map<std::size_t, gui::GuiNodeComponent> &node_components() {
        // TODO REPLACE WITH REAL COMPONENTS
        static bool fist_time = true;
        static data::map<std::size_t, gui::GuiNodeComponent> instance(10);

        if (fist_time) {
            instance.put(0, ex::gui::test::one::gui_mappers);
            instance.put(1, ex::gui::test::tri::gui_mappers);
            instance.put(2, ex::gui::test::two::gui_mappers);
            instance.put(3, ex::gui::test::one::gui_mappers);
            instance.put(4, ex::gui::test::tri::gui_mappers);
            instance.put(5, ex::gui::test::two::gui_mappers);
            instance.put(6, ex::gui::test::one::gui_mappers);
            fist_time = false;
        }

        return instance;
    }

} // namespace ex::gui
