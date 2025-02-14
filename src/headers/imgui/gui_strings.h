//
// Created by xd on 2/14/25.
//

#ifndef GUI_STRINGS_H
#define GUI_STRINGS_H

#include <IconsLucide.h>

namespace ex::gui::strings {

    constexpr auto menu_top_file         = "File";
    constexpr auto menu_top_file_new     = "New";
    constexpr auto menu_top_file_open    = "Open";
    constexpr auto menu_top_file_save    = "Save";
    constexpr auto menu_top_file_save_as = "Save As";
    constexpr auto menu_top_file_exit    = "Exit";

    constexpr auto menu_top_edit       = "Edit";
    constexpr auto menu_top_edit_undo  = ICON_LC_UNDO " Undo";
    constexpr auto menu_top_edit_redo  = ICON_LC_REDO " Redo";
    constexpr auto menu_top_edit_cut   = ICON_LC_SCISSORS " Cut";
    constexpr auto menu_top_edit_copy  = ICON_LC_CLIPBOARD_COPY " Copy";
    constexpr auto menu_top_edit_paste = ICON_LC_CLIPBOARD_PASTE " Paste";

    constexpr auto menu_top_view               = "View";
    constexpr auto menu_top_view_unpin         = "Unpin windows";
    constexpr auto menu_top_view_sidebar_left  = "Sidebar Left";
    constexpr auto menu_top_view_sidebar_right = "Sidebar Right";
    constexpr auto menu_top_view_logs          = "Logs";
    constexpr auto menu_top_view_metrics       = "Metrics";
    constexpr auto menu_top_view_reset_layout  = "Reset layout";

    constexpr auto menu_top_help       = "Help";
    constexpr auto menu_top_help_about = "About";

    constexpr auto logs_bot           = ICON_LC_LOGS " Log Output ";
    constexpr auto logs_bot_highlight = "Disable highlighting for warnings and errors";
    constexpr auto logs_bot_follow    = "Follow logs";
    constexpr auto logs_bot_copy      = "Copy to clipboard";
    constexpr auto logs_bot_clear_all = "Clear all";



}

#endif //GUI_STRINGS_H
