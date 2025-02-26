//
// Created by xd on 2/14/25.
//

#include "../../../headers/imgui/actions/gui_actions_registry.h"
#include "../../../extra/data/data_utils.h"

namespace ex::gui::actions::registry {

    data::array<action> &registry_() {
        static data::array<action> array(20);
        return array;
    }

    static data::array<action> &reg_ = registry_();

    void set_action(const action_key_t key, const action_fn_t fn) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i].key == key) {
                reg_[i].fn = fn;
                return;
            }
        }
        reg_.push({key, fn});
    }

    void remove_action(const action_key_t key) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i].key == key) {
                reg_.swap_remove(i);
                return;
            }
        }
    }

    action_fn_t get_action(const action_key_t key) {
        for (auto i = 0; i < reg_.size; i++) {
            if (reg_[i].key == key)
                return reg_[i].fn;
        }
        return nullptr;
    }

    action *list_actions(int *size_n) {
        *size_n = reg_.size;
        return reg_.data;
    }

}
