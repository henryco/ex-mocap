//
// Created by xd on 2/24/25.
//

#ifndef EX_GUI_DISPLAY_IMP_H
#define EX_GUI_DISPLAY_IMP_H

namespace ex::display::imp::imgui {

    // create display
    unsigned int add_output(const char *name);

    // rename display
    void rename_output(unsigned id, const char *name);

    // remove display
    void remove_output(unsigned id);

    // clear gl texture
    void clear(unsigned id);

    // prepare pbo and textures
    void prepare_bgra_8(unsigned id, int w, int h, int cn);

    // direct access to pbo mapped gpu memory
    void *direct_bgra_8(unsigned id);

    // plain old draw
    bool draw_bytes_bgra_8(unsigned id, const void *data);

    // flush changes to gl texture
    bool flush_bgra_8(unsigned id);
}

#endif //EX_GUI_DISPLAY_IMP_H
