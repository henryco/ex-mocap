//
// Created by xd on 23/11/24.
//

#ifndef EX_MOTION_STATE_H
#define EX_MOTION_STATE_H

namespace ex {
    struct GUI {
        bool show_logs = true;
        bool show_left = true;
        bool show_right = true;
    };

    struct State {
        GUI gui{};
    };
}

#endif // EX_MOTION_STATE_H
