//
// Created by xd on 31/01/25.
//

#ifndef LOGIC_STATE_H
#define LOGIC_STATE_H

#include "../../extra/data/struct/rb_map.h"
#include "../../extra/graph/graph.h"

namespace ex::logic::state {

    using id_type  = unsigned long int;
    using id_local = unsigned int;

    const ex::data::rb_map<id_local, limbo::graph::BaseNode *> &node_storage();

    limbo::graph::BaseNode *get_node(const id_local &);

    limbo::graph::Clock &get_clock();

    void store_node(const id_local &, limbo::graph::BaseNode *);

    void replace_node(const id_local &, limbo::graph::BaseNode *);

    void remove_node(const id_local &);

    void reset_nodes();

}

#endif //LOGIC_STATE_H
