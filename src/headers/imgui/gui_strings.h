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

    constexpr auto gui_tree_menu_general_add = "Add new node";
    constexpr auto gui_tree_menu_general_new = "New node";

    constexpr auto gui_tree_menu_group         = "Group";
    constexpr auto gui_tree_menu_group_ungroup = "Ungroup";
    constexpr auto gui_tree_menu_group_disable = "Disable";
    constexpr auto gui_tree_menu_group_enable  = "Enable";
    constexpr auto gui_tree_menu_group_cut     = "Cut";
    constexpr auto gui_tree_menu_group_copy    = "Copy";
    constexpr auto gui_tree_menu_group_delete  = "Delete";

    constexpr auto gui_tree_menu_node_new         = "New";
    constexpr auto gui_tree_menu_node_add_into    = "Add into";
    constexpr auto gui_tree_menu_node_add_after   = "Add After";
    constexpr auto gui_tree_menu_node_rename      = "Rename";
    constexpr auto gui_tree_menu_node_disable     = "Disable";
    constexpr auto gui_tree_menu_node_enable      = "Enable";
    constexpr auto gui_tree_menu_node_group       = "Group";
    constexpr auto gui_tree_menu_node_ungroup     = "Ungroup";
    constexpr auto gui_tree_menu_node_inspect     = "Inspect";
    constexpr auto gui_tree_menu_node_duplicate   = "Duplicate";
    constexpr auto gui_tree_menu_node_cut         = "Cut";
    constexpr auto gui_tree_menu_node_copy        = "Copy";
    constexpr auto gui_tree_menu_node_copy_ref    = "Copy Ref";
    constexpr auto gui_tree_menu_node_paste       = "Paste";
    constexpr auto gui_tree_menu_node_paste_after = "Paste After";
    constexpr auto gui_tree_menu_node_paste_into  = "Paste Into";
    constexpr auto gui_tree_menu_node_delete      = "Delete";
    constexpr auto gui_tree_menu_node_edit_name   = "Edit name: ";
    constexpr auto gui_tree_menu_node_edit_apply  = "Apply";

    constexpr auto gui_tree_ops_history_blank           = "Blank";
    constexpr auto gui_tree_ops_history_add             = "Add node";
    constexpr auto gui_tree_ops_history_add_into        = "Add into";
    constexpr auto gui_tree_ops_history_add_after       = "Add after";
    constexpr auto gui_tree_ops_history_node_renamed    = "Rename node";
    constexpr auto gui_tree_ops_history_node_duplicated = "Duplicate node";
    constexpr auto gui_tree_ops_history_paste_after     = "Paste after";
    constexpr auto gui_tree_ops_history_paste_into      = "Paste into";
    constexpr auto gui_tree_ops_history_move_after      = "Move after";
    constexpr auto gui_tree_ops_history_move_into       = "Move into";
    constexpr auto gui_tree_ops_history_group           = "Group selection";
    constexpr auto gui_tree_ops_history_ungroup         = "Ungroup selection";
    constexpr auto gui_tree_ops_history_node_enabled    = "Enable node";
    constexpr auto gui_tree_ops_history_node_disabled   = "Disable node";
    constexpr auto gui_tree_ops_history_node_deleted    = "Delete node";
    constexpr auto gui_tree_ops_history_select_enabled  = "Enable selection";
    constexpr auto gui_tree_ops_history_select_disabled = "Disable selection";
    constexpr auto gui_tree_ops_history_select_deleted  = "Delete selection";

    constexpr auto gui_dialogs_file_save        = "Save a file";
    constexpr auto gui_dialogs_file_save_accept = "Save";

    constexpr auto gui_dialogs_file_open        = "Open a file";
    constexpr auto gui_dialogs_file_open_accept = "Open";

}

#endif //GUI_STRINGS_H
