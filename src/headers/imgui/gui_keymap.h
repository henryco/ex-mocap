//
// Created by xd on 28/01/25.
//

#ifndef GUI_KEYMAP_H
#define GUI_KEYMAP_H

#include "imgui.h"

namespace ex::gui::keys {

    struct Op {
        const char *label = nullptr;
        int         key   = 0;
    };

    struct KeyMapping {
        struct {
            Op cut       = { "Ctrl+X", ImGuiMod_Ctrl | ImGuiKey_X };
            Op copy      = { "Ctrl+C", ImGuiMod_Ctrl | ImGuiKey_C };
            Op paste     = { "Ctrl+V", ImGuiMod_Ctrl | ImGuiKey_V };
            Op reference = { "Ctrl+R", ImGuiMod_Ctrl | ImGuiKey_R };
            Op duplicate = { "Ctrl+D", ImGuiMod_Ctrl | ImGuiKey_D };
            Op deletion  = { "Delete", ImGuiKey_Delete };
        } work_tree;
    };

    inline KeyMapping mapping() {
        // TODO: move to source file
        static KeyMapping instance;
        return instance;
    }
}

#endif //GUI_KEYMAP_H
