//
// Created by xd on 2/18/25.
//

#include "../../../headers/imgui/layout/gui_layout.h"
#include "../../../headers/logic/logic_globals.h"
#include "../../../headers/utils/gui_utils.h"
#include "../../../headers/imgui/gui_strings.h"
#include "../../../headers/utils/data_utils.h"

namespace ex::gui {

    const char *prepare_message(const char *msg, const std::uint64_t &err_code) {
        static const int overhead = 24 + strlen(ICON_LC_TRIANGLE_ALERT);
        const char      *pre      = msg != nullptr ? msg : "Unknown Error";

        static data::buffer<char> buffer;
        buffer.allocate(strlen(msg) + overhead);
        sprintf(buffer.data, "%s (%lu) %s", ICON_LC_TRIANGLE_ALERT, err_code, pre);

        return buffer.data;
    }

    void show_alert_dialogs() {
        static bool   show  = false;
        static State &state = ex::logic::globals::state();

        if (!show && state.error.code != 0) {
            ImGui::OpenPopup(strings::gui_dialogs_alert_error);
            show = true;
        }

        if (!show)
            return;

        const char *err_msg = prepare_message(state.error.msg, state.error.code);
        if (show_alert_dialog(strings::gui_dialogs_alert_error, err_msg, strings::gui_dialogs_alert_error_yes)) {
            state.error.code = 0;
            show = false;
        }

    }

}