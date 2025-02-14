//
// Created by xd on 2/14/25.
//

#include "../../headers/imgui/gui_actions.h"
#include "../../headers/utils/data_utils.h"


namespace ex::gui::actions {

    data::array<int> &registry_() {
        static data::array<int> array(20);
        return array;
    }

    data::array<action_fn_t> &functions_() {
        static data::array<action_fn_t> array(20);
        return array;
    }

    data::array<action_fn_t> &fns_ = functions_();
    data::array<int>         &reg_ = registry_();

    void set_action(const int action, const action_fn_t fn) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i] == action) {
                fns_[i] = fn;
                return;
            }
        }
        reg_.push(action);
        fns_.push(fn);
    }

    void remove_action(const int action) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i] == action) {
                reg_.swap_remove(i);
                fns_.swap_remove(i);
                return;
            }
        }
    }

    action_fn_t get_action(const int action) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i] == action)
                return fns_[i];
        }
        return nullptr;
    }

    action_fn_t *list_actions(int *size_n) {
        *size_n = fns_.size;
        return fns_.data;
    }

}
