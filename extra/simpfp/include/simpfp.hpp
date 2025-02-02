//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

#include <filesystem>

namespace simpfp {

    namespace internal_ {
        inline const char *label     = nullptr;
        inline const char *file_path = nullptr;
        inline bool       *open_ptr  = nullptr;
        inline bool        closed    = false;
        inline bool        reset     = false;
        inline bool        resizable = true;

        inline bool show_dialog(const char *label, bool *open = nullptr, const ImVec2 *size = nullptr, const bool resizable = true) {
            if (open != nullptr && !*open) {
                internal_::open_ptr  = nullptr;
                internal_::label     = nullptr;
                internal_::reset     = false;
                internal_::closed    = true;
                internal_::resizable = true;
                return false;
            }

            if (label == nullptr) {
                internal_::open_ptr  = nullptr;
                internal_::label     = nullptr;
                internal_::reset     = false;
                internal_::closed    = true;
                internal_::resizable = true;
                return false;
            }

            if (internal_::label == label) {
                if (internal_::closed) {
                    if (open != nullptr)
                        *open = false;
                    return false;
                }

                if (internal_::reset) {
                    if (size != nullptr) {
                        ImGui::SetNextWindowSizeConstraints(*size, ImVec2(-1, -1));
                        ImGui::SetNextWindowSize(*size, ImGuiCond_Appearing);
                    }
                    internal_::resizable = resizable;
                    internal_::reset = false;
                    ImGui::OpenPopup(internal_::label, ImGuiPopupFlags_NoReopen);
                }

                internal_::open_ptr = open;
                return true;
            }

            internal_::open_ptr  = open;
            internal_::label     = label;
            internal_::reset     = false;
            internal_::closed    = false;
            internal_::resizable = resizable;

            if (size != nullptr) {
                ImGui::SetNextWindowSizeConstraints(*size, ImVec2(-1, -1));
                ImGui::SetNextWindowSize(*size, ImGuiCond_Appearing);
            }
            ImGui::OpenPopup(internal_::label, ImGuiPopupFlags_NoReopen);
            return true;
        }

        inline void full_width_input(char *buffer, const std::size_t buffer_size) {
            const auto input_pos = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());

            ImGui::SetNextItemWidth(input_size.x);
            ImGui::InputText("##bordered_input", buffer, buffer_size);

            ImGui::GetWindowDrawList()->AddRect(
                input_pos,
                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                IM_COL32(0, 0, 0, 90));
        }

        inline void buttons_dir(bool *create, bool *remove) {
            ImGui::BeginDisabled(create == nullptr);
            if (ImGui::Button("New Folder")) {
                if (create != nullptr)
                    *create = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled(remove == nullptr);
            if (ImGui::Button("Delete Folder")) {
                if (remove != nullptr)
                    *remove = true;
            }
            ImGui::EndDisabled();
        }

        inline void buttons_action(bool *cancel, bool *accept) {
            constexpr float button_width = 60.0f;
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float available_width = ImGui::GetContentRegionMax().x;

            ImGui::SetCursorPosX(available_width - (button_width * 2 + spacing));

            ImGui::BeginDisabled(cancel == nullptr);
            if (ImGui::Button("Cancel", ImVec2(button_width, 0))) {
                if (cancel != nullptr)
                    *cancel = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled(accept == nullptr);
            if (ImGui::Button("Accept", ImVec2(button_width, 0))) {
                if (accept != nullptr)
                    *accept = true;
            }
            ImGui::EndDisabled();
        }
    }

    inline void OpenFileDialog(const char *label, const char *default_path = nullptr) {
        internal_::open_ptr  = nullptr;
        internal_::closed    = false;
        internal_::reset     = true;
        internal_::label     = label;
        internal_::file_path = default_path;
    }

    inline bool ShowFileDialog(const char *label, bool *open = nullptr) {
        return internal_::show_dialog(label, open);
    }

    inline bool ShowFileDialog(const char *label, bool *open, const ImVec2 &size, const bool resize = true) {
        return internal_::show_dialog(label, open, &size, resize);
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
                                   ImGuiWindowFlags_NoCollapse |
                                   ImGuiWindowFlags_NoDocking  |
                                   (internal_::resizable ? 0 : ImGuiWindowFlags_NoResize))) {

            static const float reserve_y = (ImGui::GetFrameHeight() * 4) + ImGui::GetStyle().ItemSpacing.y;

            constexpr float ratio     = 0.25f;
            const float     spacing_x = ImGui::GetStyle().ItemSpacing.x;
            const float     free_x    = ImGui::GetContentRegionAvail().x - spacing_x;

            const float free_x_uno = free_x * ratio;
            const float free_x_des = free_x * (1.f - ratio);

            ImGui::Spacing();
            ImGui::Text("File");

            ImGui::BeginChild("##region_list", ImVec2(free_x_uno, -reserve_y), ImGuiChildFlags_Borders);
            // TODO
            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##region_table", ImVec2(free_x_des, -reserve_y), ImGuiChildFlags_Borders);
            // TODO
            ImGui::EndChild();

            static char buffer[128] = "some path";
            internal_::full_width_input(buffer, IM_ARRAYSIZE(buffer));
            ImGui::Spacing();

            bool create = false;
            bool remove = false;
            bool cancel = false;
            bool accept = false;

            internal_::buttons_dir(&create, &remove);
            ImGui::Spacing();
            internal_::buttons_action(&cancel, &accept);

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
