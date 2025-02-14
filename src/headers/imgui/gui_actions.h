//
// Created by xd on 2/14/25.
//

#ifndef GUI_ACTIONS_H
#define GUI_ACTIONS_H

namespace ex::gui::actions {

    using action_fn_t  = void (*)();
    using action_key_t = int;

    struct action {
        action_key_t key;
        action_fn_t  fn;
    };

    void set_action(action_key_t key, action_fn_t fn);

    void remove_action(action_key_t ley);

    action_fn_t get_action(action_key_t ley);

    action *list_actions(int *size_n);

}

#endif //GUI_ACTIONS_H
