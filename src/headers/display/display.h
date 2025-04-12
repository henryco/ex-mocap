//
// Created by xd on 2/24/25.
//

#ifndef EX_MOTION_DISPLAY_H
#define EX_MOTION_DISPLAY_H

namespace ex::display {

    struct IO {
        unsigned int (*add_output)(const char *) = nullptr;
        void (*remove_output)(unsigned int)      = nullptr;
        void (*draw_output)(unsigned int id, unsigned long long tex_id, int w, int h);
        // TODO
    };

    extern IO &io;

}

#endif //EX_MOTION_DISPLAY_H
