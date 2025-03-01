//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_STATE_H
#define LOGIC_STATE_H

#include "nodes/logic_nodes.h"
#include <map>

namespace ex::logic::state {

    using id_type  = std::uint64_t;
    using id_local = std::uint32_t;

    const std::map<id_local, limbo::graph::BaseNode *> &node_storage();

    limbo::graph::BaseNode *get_node(const id_local &);

    void store_node(const id_local &, limbo::graph::BaseNode *);

    void replace_node(const id_local &, limbo::graph::BaseNode *);

    void remove_node(const id_local &);

}

#endif //LOGIC_STATE_H
