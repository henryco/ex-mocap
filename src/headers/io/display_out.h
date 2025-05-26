//
// Created by xd on 2/24/25.
//

#ifndef EX_MOTION_DISPLAY_H
#define EX_MOTION_DISPLAY_H

namespace ex::display {

    using disp_id_t = unsigned int;

    struct IO {
        // create display
        disp_id_t (*add_output)(const char *)                      = nullptr;

        // rename display
        void (*rename_output)(disp_id_t, const char *)             = nullptr;

        // remove display
        void (*remove_output)(disp_id_t)                           = nullptr;

        // clear gl texture
        void (*clear)(disp_id_t)                                   = nullptr;

        // prepare display canvas
        void (*prepare_bgra_8)(disp_id_t id, int w, int h, int cn) = nullptr;

        // direct access canvas memory
        void *(*direct_bgra_8)(disp_id_t id)                       = nullptr;

        // plain old draw
        bool (*draw_bytes_bgra_8)(disp_id_t id, const void *data)  = nullptr;

        // flush changes canvas
        bool (*flush_bgra_8)(disp_id_t id)                         = nullptr;
    };

    inline IO io;

}

#endif //EX_MOTION_DISPLAY_H
