//
// Created by xd on 2/24/25.
//

#ifndef EX_MOTION_DISPLAY_H
#define EX_MOTION_DISPLAY_H

namespace ex::display {

    struct IO {
        void (*add_output)(const char *, int w, int h, int channels, int depth) = nullptr;
        void (*remove_output)(const char *)                                     = nullptr;
        // TODO
    };

    static IO io;

}

#endif //EX_MOTION_DISPLAY_H
