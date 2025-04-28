//
// Created by xd on 4/28/25.
//

#include "../../headers/nodes/node_registry.h"
#include "../../headers/logic/logic_mappers.h"
#include "../../headers/imgui/gui_mappers.h"


namespace ex::logic::mappers {

    inline const data::array_map<id_type, NodeMapper> &cmp_() {
        const auto &components = ex::nodes::mappers::node_components();
        static data::array_map<id_type, NodeMapper> instance;
        for (int i = 0; i < components.size; ++i)
            instance.put(components.key_entries[i], components.val_entries[i].logic);
        return instance;
    }

    inline const data::array_map<id_type, NodeMapper> &out_() {
        const auto &outputs = ex::nodes::mappers::node_outputs();
        static data::array_map<id_type, NodeMapper> instance;
        for (int i = 0; i < outputs.size; ++i)
            instance.put(outputs.key_entries[i], outputs.val_entries[i].logic);
        return instance;
    }

    const data::array_map<id_type, NodeMapper> &node_components() {
        static const data::array_map<id_type, NodeMapper> &instance = cmp_();
        return instance;
    }

    const data::array_map<id_type, NodeMapper> &node_outputs() {
        static const data::array_map<id_type, NodeMapper> &instance = out_();
        return instance;
    }
}

namespace ex::gui::mappers {

    inline const data::array_map<id_type, GuiNodeMapper> &cmp_() {
        const auto &components = ex::nodes::mappers::node_components();
        static data::array_map<id_type, GuiNodeMapper> instance;
        for (int i = 0; i < components.size; ++i)
            instance.put(components.key_entries[i], components.val_entries[i].gui);
        return instance;
    }

    inline const data::array_map<id_type, GuiNodeMapper> &out_() {
        const auto &outputs = ex::nodes::mappers::node_outputs();
        static data::array_map<id_type, GuiNodeMapper> instance;
        for (int i = 0; i < outputs.size; ++i)
            instance.put(outputs.key_entries[i], outputs.val_entries[i].gui);
        return instance;
    }

    const data::array_map<id_type, GuiNodeMapper> &node_components() {
        static const data::array_map<id_type, GuiNodeMapper> &instance = cmp_();
        return instance;
    }

    const data::array_map<id_type, GuiNodeMapper> &node_outputs() {
        static const data::array_map<id_type, GuiNodeMapper> &instance = out_();
        return instance;
    }
}