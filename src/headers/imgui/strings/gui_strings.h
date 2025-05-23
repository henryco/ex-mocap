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

    constexpr auto menu_top_edit        = "Edit";
    constexpr auto menu_top_edit_undo   = ICON_LC_UNDO " Undo";
    constexpr auto menu_top_edit_redo   = ICON_LC_REDO " Redo";
    constexpr auto menu_top_edit_cut    = ICON_LC_SCISSORS " Cut";
    constexpr auto menu_top_edit_copy   = ICON_LC_CLIPBOARD_COPY " Copy";
    constexpr auto menu_top_edit_paste  = ICON_LC_CLIPBOARD_PASTE " Paste";
    constexpr auto menu_top_edit_insert = ICON_LC_CLIPBOARD_PASTE " Insert";

    constexpr auto menu_top_view               = "View";
    constexpr auto menu_top_view_unpin         = "Unpin windows";
    constexpr auto menu_top_view_sidebar_left  = "Sidebar Left";
    constexpr auto menu_top_view_sidebar_right = "Sidebar Right";
    constexpr auto menu_top_view_logs          = "Logs";
    constexpr auto menu_top_view_metrics       = "Metrics";
    constexpr auto menu_top_view_fps           = "Frame rate";
    constexpr auto menu_top_view_reset_layout  = "Reset layout";

    constexpr auto menu_top_help       = "Help";
    constexpr auto menu_top_help_about = "About";

    constexpr auto logs_bot           = ICON_LC_LOGS " Log Output ";
    constexpr auto logs_bot_highlight = "Disable highlighting for warnings and errors";
    constexpr auto logs_bot_follow    = "Follow logs";
    constexpr auto logs_bot_copy      = "Copy to clipboard";
    constexpr auto logs_bot_clear_all = "Clear all";

    constexpr auto gui_display_button_bg_change = "Change background color";

    constexpr auto gui_tree_menu_general_add     = "Add new node";
    constexpr auto gui_tree_menu_general_new     = "New node";
    constexpr auto gui_tree_menu_general_out     = "New output";
    constexpr auto gui_tree_menu_general_paste   = "Paste node";
    constexpr auto gui_tree_menu_general_section = "Create section";

    constexpr auto gui_tree_menu_group         = "Group";
    constexpr auto gui_tree_menu_group_ungroup = "Ungroup";
    constexpr auto gui_tree_menu_group_disable = "Disable";
    constexpr auto gui_tree_menu_group_enable  = "Enable";
    constexpr auto gui_tree_menu_group_cut     = "Cut";
    constexpr auto gui_tree_menu_group_copy    = "Copy";
    constexpr auto gui_tree_menu_group_delete  = "Delete";

    constexpr auto gui_tree_top_menu_expand_all   = " Expand All ";
    constexpr auto gui_tree_top_menu_collapse_all = " Collapse All ";

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
    constexpr auto gui_tree_ops_history_paste           = "Paste Node";
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

    constexpr auto gui_dialogs_project_save           = "Save a file";
    constexpr auto gui_dialogs_project_save_accept    = "Save";
    constexpr auto gui_dialogs_project_save_alert     = "Are you sure?";
    constexpr auto gui_dialogs_project_save_alert_msg = "This action will overwrite selected files.";
    constexpr auto gui_dialogs_project_save_alert_yes = "Accept";
    constexpr auto gui_dialogs_project_save_alert_no  = "Cancel";

    constexpr auto gui_dialogs_project_open           = "Open a file";
    constexpr auto gui_dialogs_project_open_accept    = "Open";
    constexpr auto gui_dialogs_project_open_alert     = "Open another project?";
    constexpr auto gui_dialogs_project_open_alert_msg = "Are you sure? Unsaved progress will be lost.";
    constexpr auto gui_dialogs_project_open_alert_yes = "Proceed";
    constexpr auto gui_dialogs_project_open_alert_no  = "Cancel";

    constexpr auto gui_dialogs_project_open_error     = "Error";
    constexpr auto gui_dialogs_project_open_error_msg = "Invalid file type";
    constexpr auto gui_dialogs_project_open_error_ok  = "OK";

    constexpr auto gui_dialogs_project_save_error     = "Error";
    constexpr auto gui_dialogs_project_save_error_msg = "Invalid file name";
    constexpr auto gui_dialogs_project_save_error_ok  = "OK";

    constexpr auto gui_dialogs_project_new           = "Create new project";
    constexpr auto gui_dialogs_project_new_accept    = "Create";
    constexpr auto gui_dialogs_project_new_alert     = "Create new project?";
    constexpr auto gui_dialogs_project_new_alert_msg = "Are you sure? Unsaved progress will be lost.";
    constexpr auto gui_dialogs_project_new_alert_yes = "Proceed";
    constexpr auto gui_dialogs_project_new_alert_no  = "Cancel";

    constexpr auto gui_dialogs_project_exit_alert     = "Confirm exit";
    constexpr auto gui_dialogs_project_exit_alert_msg = "Changes which are not saved will be lost.";
    constexpr auto gui_dialogs_project_exit_alert_yes = "Exit";
    constexpr auto gui_dialogs_project_exit_alert_no  = "Cancel";

    constexpr auto gui_dialogs_alert_error     = "Error";
    constexpr auto gui_dialogs_alert_error_yes = "OK";

    constexpr auto node_section_new_name = "New section";

    constexpr auto node_output_display_name = "Display";

    constexpr auto node_component_image_id         = "Image ID";
    constexpr auto node_component_image_name       = "Image";
    constexpr auto node_component_image_select     = " Select ";
    constexpr auto node_component_image_source     = "Source";
    constexpr auto node_component_image_sourceless = "Source not selected";

    constexpr auto node_component_camera_id        = "Image ID";
    constexpr auto node_component_camera_name      = "Camera";
    constexpr auto node_component_camera_mode      = "Device Mode";
    constexpr auto node_component_camera_device    = "Camera Device";
    constexpr auto node_component_camera_refresh   = "Refresh";
    constexpr auto node_component_camera_reset     = "Reset";
    constexpr auto node_component_camera_reset_all = " Reset Settings ";
    constexpr auto node_component_camera_buffering = "Buffering";

    constexpr auto node_component_camera_buffering_0 = "Default";
    constexpr auto node_component_camera_buffering_1 = "Single (1)";
    constexpr auto node_component_camera_buffering_2 = "Double (2)";
    constexpr auto node_component_camera_buffering_3 = "Triple (3)";

    constexpr auto node_component_sync_name = "Sync";

    constexpr auto node_component_transform_id        = "Image ID";
    constexpr auto node_component_transform_name      = "Transform";
    constexpr auto node_component_transform_region    = "Region";
    constexpr auto node_component_transform_region_x0 = "x0";
    constexpr auto node_component_transform_region_y0 = "y0";
    constexpr auto node_component_transform_region_x1 = "x1";
    constexpr auto node_component_transform_region_y1 = "y1";
    constexpr auto node_component_transform_resize    = "Resize";
    constexpr auto node_component_transform_resize_w  = "width##resize";
    constexpr auto node_component_transform_resize_h  = "height##resize";
    constexpr auto node_component_transform_resize_l  = "Linear interpolation";
    constexpr auto node_component_transform_mirror    = "Mirror";
    constexpr auto node_component_transform_mirror_h  = "Horizontally";
    constexpr auto node_component_transform_mirror_v  = "Vertically";
    constexpr auto node_component_transform_rotate    = "Rotate";
    constexpr auto node_component_transform_rotate_0  = "None";
    constexpr auto node_component_transform_rotate_90 = "Clockwise 90º";

}

#endif //GUI_STRINGS_H
