//
// Created by xd on 2/24/25.
//

#include "../../../headers/imgui/gui_globals.h"

namespace ex::gui::internal_ {
    inline const data::map<id_type, gui::GuiNodeMapper> &instance_out_map_() {
        static data::map<id_type, gui::GuiNodeMapper> instance;
        // TODO
        // instance.put(0, ex::gui::test::one::gui_mappers);
        // TODO
        return instance;
    }
}

namespace ex::gui {
    const data::map<id_type, gui::GuiNodeMapper> &node_outputs() {
        static const data::map<id_type, gui::GuiNodeMapper> &instance = internal_::instance_out_map_();
        return instance;
    }
}