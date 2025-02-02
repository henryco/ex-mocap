//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

namespace simpfp {

    inline bool ShowFileDialog(const char *label, char *buffer, std::size_t buffer_size, bool *open = nullptr) {
        if (open != nullptr && !*open) {
            return false;
        }

        return true;
    }

}

#endif //SIMPFP_HPP
