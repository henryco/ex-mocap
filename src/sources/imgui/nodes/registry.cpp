//
// Created by xd on 2/19/25.
//

#include "../../../headers/imgui/gui_globals.h"


// TODO REPLACE WITH REAL COMPONENTS
#include "../../../headers/imgui/nodes/test_cmp.h"
// TODO REPLACE WITH REAL COMPONENTS


namespace ex::gui::internal_ {
    inline const data::map<id_type, gui::GuiNodeComponent> &instance_cmp_map_() {
        static data::map<id_type, gui::GuiNodeComponent> instance;

        static const auto &out_map = ex::gui::node_outputs();
        for (int i = 0; i < out_map.size; i++)
            instance.put(out_map.key_entries[i], out_map.val_entries[i]);

        // TODO REPLACE WITH REAL COMPONENTS
        instance.put(0, ex::gui::test::one::gui_mappers);
        instance.put(1, ex::gui::test::tri::gui_mappers);
        instance.put(2, ex::gui::test::two::gui_mappers);
        instance.put(3, ex::gui::test::one::gui_mappers);
        instance.put(4, ex::gui::test::tri::gui_mappers);
        instance.put(5, ex::gui::test::two::gui_mappers);
        instance.put(6, ex::gui::test::one::gui_mappers);
        // TODO REPLACE WITH REAL COMPONENTS
        return instance;
    }
}

namespace ex::gui {
    const data::map<id_type, gui::GuiNodeComponent> &node_components() {
        static const data::map<id_type, gui::GuiNodeComponent> &instance = internal_::instance_cmp_map_();
        return instance;
    }
}