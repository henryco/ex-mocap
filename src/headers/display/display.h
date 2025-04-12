//
// Created by xd on 2/24/25.
//

#ifndef EX_MOTION_DISPLAY_H
#define EX_MOTION_DISPLAY_H

namespace ex::display {

    using disp_id_t = unsigned int;

    struct IO {
        disp_id_t (*add_output)(const char *)                                              = nullptr;
        void (*remove_output)(disp_id_t)                                                   = nullptr;
        void (*clear)(disp_id_t)                                                           = nullptr;
        void (*draw_bytes_bgra_8)(disp_id_t id, unsigned char *data, int w, int h, int cn) = nullptr;
        // TODO
    };

    extern IO &io;

}

#endif //EX_MOTION_DISPLAY_H
