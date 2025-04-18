//
// Created by xd on 2/24/25.
//

#ifndef EX_GUI_DISPLAY_IMP_H
#define EX_GUI_DISPLAY_IMP_H

namespace ex::display::imp::imgui {

    unsigned int add_output(const char *name);

    void rename_output(unsigned int id, const char *name);

    void remove_output(unsigned int id);

    void clear(unsigned int id);

    void draw_bytes_bgra_8(unsigned int id, unsigned char *data, int w, int h, int cn);

    // TODO
}

#endif //EX_GUI_DISPLAY_IMP_H
