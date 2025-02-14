//
// Created by xd on 2/14/25.
//

#ifndef GUI_ACTIONS_H
#define GUI_ACTIONS_H

namespace ex::gui::actions {

    using action_fn_t = void (*)();

    void        set_action(int action, action_fn_t fn);

    void        remove_action(int action);

    action_fn_t get_action(int action);

}

#endif //GUI_ACTIONS_H
