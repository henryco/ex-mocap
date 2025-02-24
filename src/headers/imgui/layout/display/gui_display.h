//
// Created by xd on 2/23/25.
//

#ifndef GUI_DISPLAY_H
#define GUI_DISPLAY_H

namespace ex::gui::display {

    void show_display();

    namespace exporting {
        void add_output(const char *, int w, int h, int channels, int depth);

        void remove_output(const char *);

        // TODO
    }

}

#endif //GUI_DISPLAY_H
