//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

#include <filesystem>

namespace simpfp {

    namespace internal_ {
        inline const char *label    = nullptr;
        inline bool       *open_ptr = nullptr;
        inline bool        closed   = false;
        inline bool        reset    = false;
    }

    inline void OpenFileDialog(const char *label) {
        internal_::open_ptr = nullptr;
        internal_::closed   = false;
        internal_::reset    = true;
        internal_::label    = label;
    }

    inline bool ShowFileDialog(const char *label, bool *open = nullptr) {
        if (open != nullptr && !*open) {
            internal_::open_ptr = nullptr;
            internal_::label    = nullptr;
            internal_::reset    = false;
            internal_::closed   = true;
            return false;
        }

        if (label == nullptr) {
            internal_::open_ptr = nullptr;
            internal_::label    = nullptr;
            internal_::reset    = false;
            internal_::closed   = true;
            return false;
        }

        if (internal_::label == label) {
            if (internal_::closed) {
                if (open != nullptr)
                    *open = false;
                return false;
            }

            if (internal_::reset) {
                ImGui::OpenPopup(internal_::label, ImGuiPopupFlags_NoReopen);
                internal_::reset = false;
            }

            internal_::open_ptr = open;
            return true;
        }

        internal_::open_ptr = open;
        internal_::label    = label;
        internal_::reset    = false;
        internal_::closed   = false;
        ImGui::OpenPopup(internal_::label, ImGuiPopupFlags_NoReopen);
        return true;
    }

    inline void EndFileDialog() {
        if (internal_::closed) {
            if (internal_::open_ptr != nullptr)
                *internal_::open_ptr = false;
            return;
        }

        if (internal_::open_ptr != nullptr && !*internal_::open_ptr) {
            internal_::closed = true;
        }

        const auto center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(internal_::label, internal_::open_ptr,
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {

            ImGui::Text("File");
            // TODO
            ImGui::EndPopup();
        }

        if (internal_::open_ptr != nullptr && !*internal_::open_ptr) {
            internal_::closed = true;
        }
    }

    inline bool FileSelected(char *buffer, std::size_t buffer_size) {
        // TODO
        return true;
    }

    inline bool FileAccepted(char *buffer, std::size_t buffer_size) {
        // TODO
        return true;
    }

}

#endif //SIMPFP_HPP
