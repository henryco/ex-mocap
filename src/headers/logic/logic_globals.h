//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_GLOBALS_H
#define LOGIC_GLOBALS_H

#include <map>
#include "logic_component.h"

namespace ex::logic::globals {

    using id_type = std::uint64_t;
    using id_local = std::uint32_t;

    State &get_global_state();

    const std::map<id_type, ex::LogicNodeComponent> &node_components();

    const std::map<id_local, limbo::nodes::BaseNode *> &node_store();

    limbo::nodes::BaseNode *get_node(const id_local &);

    void store_node(const id_local &, limbo::nodes::BaseNode *);

    void replace_node(const id_local &, limbo::nodes::BaseNode *);

    void remove_node(const id_local &);

    void update_counter(const id_local &);

    void cleanup_nodes();

}

#endif //LOGIC_GLOBALS_H
