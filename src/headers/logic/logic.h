//
// Created by xd on 17/12/24.
//

#ifndef LOGIC_H
#define LOGIC_H

#include "../../../extra/graph/graph.h"
#include "../state.h"

namespace ex::main {

    struct simple_list {
        TreeNode    *node = nullptr;
        simple_list *next = nullptr;

        ~simple_list() {
            // hopefully it will not end up with stack overflow :/
            if (next != nullptr) {
                delete next;
                next = nullptr;
            }
        }
    };

    class Logic {
    private:
        limbo::graph::Clock clock_;

    public:

        Logic() = default;

        ~Logic() = default;

        void initialize(State& state);

        void run(State &state);

    protected:
        void traverse_(State &state, TreeNode *node, bool *rewire, bool fresh);

        void reset_internal_state_(State &state);

        void synchronize_(State &state);

        void process_(State &state);

        void rewire_(State &state);

        void build_tree_(State &state, TreeNode *node, TreeNode *previous, simple_list *group_list);

        void deserialize_(State &state);

        void serialize_(State &state);

        void create_new_(State &state);

        void gc_(State &state);

        void mark_node_deleted_(State &state, TreeNode *node, bool moved);

        bool connect_nodes_(TreeNode *from, TreeNode *to);
    };

    Logic create_logic(State& state);

}

#endif //LOGIC_H
