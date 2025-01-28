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
            Op undo = { "C-z", ImGuiMod_Ctrl | ImGuiKey_Z };
            Op redo = { "C-y", ImGuiMod_Ctrl | ImGuiKey_Y };
        } history;

        struct {
            Op cut       = { "C-x", ImGuiMod_Ctrl | ImGuiKey_X };
            Op copy      = { "C-c", ImGuiMod_Ctrl | ImGuiKey_C };
            Op paste     = { "C-v", ImGuiMod_Ctrl | ImGuiKey_V };
            Op insert    = { "C-S-v", ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_V };
            Op reference = { "C-r", ImGuiMod_Ctrl | ImGuiKey_R };
            Op duplicate = { "C-d", ImGuiMod_Ctrl | ImGuiKey_D };
            Op deletion  = { "Delete", ImGuiKey_Delete };
        } work_tree;
    };

}

#endif //GUI_KEYMAP_H
