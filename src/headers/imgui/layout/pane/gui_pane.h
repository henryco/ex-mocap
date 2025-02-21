//
// Created by henryco on 21/02/25.
//

#ifndef GUI_PANE_H
#define GUI_PANE_H

namespace ex::gui::pane {

    using draw_fn = void(*)();

    void set_left_menu_fn(const draw_fn &fn);

    void set_right_menu_fn(const draw_fn &fn);

    const draw_fn &get_left_menu_fn();

    const draw_fn &get_right_menu_fn();

}

#endif //GUI_PANE_H
