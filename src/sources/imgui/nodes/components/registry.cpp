//
// Created by xd on 2/19/25.
//

#include "../../../../headers/imgui/gui_globals.h"
#include "../../../../headers/imgui/gui_mappers.h"

// TODO REPLACE WITH REAL COMPONENTS
#include "../../../../headers/imgui/nodes/components/test_cmp.h"
// TODO REPLACE WITH REAL COMPONENTS

namespace ex::gui::mappers {

    namespace internal_ {
        inline const data::map<id_type, GuiNodeMapper> &instance_cmp_map_() {
            static data::map<id_type, GuiNodeMapper> instance;
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

    const data::map<id_type, GuiNodeMapper> &node_components() {
        static const data::map<id_type, GuiNodeMapper> &instance = internal_::instance_cmp_map_();
        return instance;
    }
}