//
// Created by xd on 2/19/25.
//

#include "../../../headers/imgui/gui_globals.h"


// TODO REPLACE WITH REAL COMPONENTS
#include "../../../headers/imgui/components/test_cmp.h"
// TODO REPLACE WITH REAL COMPONENTS


namespace ex::gui {

    const data::map<id_type, gui::GuiNodeComponent> &node_components() {
        static bool fist_time = true;
        static data::map<id_type, gui::GuiNodeComponent> instance;

        if (fist_time) {
            // TODO REPLACE WITH REAL COMPONENTS
            instance.put(0, ex::gui::test::one::gui_mappers);
            instance.put(1, ex::gui::test::tri::gui_mappers);
            instance.put(2, ex::gui::test::two::gui_mappers);
            instance.put(3, ex::gui::test::one::gui_mappers);
            instance.put(4, ex::gui::test::tri::gui_mappers);
            instance.put(5, ex::gui::test::two::gui_mappers);
            instance.put(6, ex::gui::test::one::gui_mappers);
            // TODO REPLACE WITH REAL COMPONENTS
            fist_time = false;
        }

        return instance;
    }

}