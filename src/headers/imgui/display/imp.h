//
// Created by xd on 2/24/25.
//

#ifndef EX_GUI_DISPLAY_IMP_H
#define EX_GUI_DISPLAY_IMP_H

namespace ex::display::imp::imgui {

    unsigned int add_output(const char *name);

    void remove_output(unsigned int id);

    void draw_output(unsigned int id, unsigned long long tex_id, int w, int h);

    // TODO
}

#endif //EX_GUI_DISPLAY_IMP_H
