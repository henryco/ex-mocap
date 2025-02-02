//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

#include <filesystem>

namespace simpfp {

    namespace internal_ {

        inline std::filesystem::path load_path(const char *file) {
            if (file == nullptr)
                return std::filesystem::current_path();
            const auto path = std::filesystem::path(file);
            if (std::filesystem::exists(path))
                return std::filesystem::is_directory(path) ? path : path.parent_path();
            return std::filesystem::current_path();
        }

        inline char *cpy_str(const char *str) {
            if (str == nullptr)
                return nullptr;
            const auto len  = std::strlen(str);
            char *buff = new char[len + 1];
            std::strcpy(buff, str);
            return buff;
        }

        struct FileInfo {
            const std::filesystem::path *path;

            const char *name;
            const char *type;
            std::size_t size;
            std::size_t date;
            bool        selected;

            ~FileInfo() {
                if (path != nullptr)
                    delete path;
                if (name != nullptr)
                    delete name;
                if (type != nullptr)
                    delete type;
            }
        };

        struct FileContext {
            const std::filesystem::path *path = nullptr;

            const FileInfo *files = nullptr;
            const FileInfo *dirs  = nullptr;

            int files_num = 0;
            int dirs_num  = 0;
            char sort_by  = 0;
            bool sort_asc = false;

            ~FileContext() {
                if (files != nullptr)
                    delete[] files;
                if (dirs != nullptr)
                    delete[] dirs;
                if (path != nullptr)
                    delete[] path;
            }

            static void load(FileContext **context, const char *path) {
                if (*context != nullptr)
                    delete *context;

                const auto file = new std::filesystem::path(load_path(path));

                const auto new_context = new FileContext;
                new_context->path      = file;
                new_context->sort_by   = 0;
                new_context->sort_asc  = false;

                int dirs_num  = 1;
                int files_num = 0;
                for (const auto &entry : std::filesystem::directory_iterator(*file)) {
                    if (entry.is_directory())
                        dirs_num++;
                    else if (entry.is_regular_file())
                        files_num++;
                }

                const auto files = new FileInfo[files_num];
                const auto dirs  = new FileInfo[dirs_num];

                dirs[0] = FileInfo();
                dirs[0].path = new std::filesystem::path(file->parent_path());
                dirs[0].selected = false;
                dirs[0].size     = 0;
                dirs[0].date     = 0;
                dirs[0].name     = cpy_str("..");
                dirs[0].type     = cpy_str("..");

                int d_ = 1, f_ = 0;
                for (const auto &entry : std::filesystem::directory_iterator(*file)) {
                    const auto ep = new std::filesystem::path(entry.path());
                    if (entry.is_directory()) {
                        dirs[d_]          = FileInfo();
                        dirs[d_].path     = ep;
                        dirs[d_].name     = cpy_str(ep->filename().c_str());
                        dirs[d_].type     = cpy_str("");
                        dirs[d_].selected = false;
                        dirs[d_].size     = 0;
                        dirs[d_].date     = 0;
                        d_++;
                    }
                    else if (entry.is_regular_file()) {
                        files[f_]          = FileInfo();
                        files[f_].path     = ep;
                        files[f_].type     = cpy_str(ep->has_extension() ? ep->extension().c_str() : "");
                        files[f_].name     = cpy_str(ep->filename().c_str());
                        files[f_].size     = entry.file_size();
                        files[f_].selected = false;
                        files[f_].date     = 0;
                        f_++;
                    }
                }

                new_context->files_num = files_num;
                new_context->files     = files;
                new_context->dirs_num  = dirs_num;
                new_context->dirs      = dirs;

                *context = new_context;
            }
        };

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

//            ImGui::SameLine();
//            ImGui::BeginDisabled(remove == nullptr);
//            if (ImGui::Button("Delete Folder")) {
//                if (remove != nullptr)
//                    *remove = true;
//            }
//            ImGui::EndDisabled();
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
        static internal_::FileContext *context = nullptr;

        if (internal_::closed) {
            if (internal_::open_ptr != nullptr)
                *internal_::open_ptr = false;
            if (context != nullptr) {
                delete context;
                context = nullptr;
            }
            return;
        }

        if (internal_::open_ptr != nullptr && !*internal_::open_ptr) {
            internal_::closed = true;
            if (context != nullptr) {
                delete context;
                context = nullptr;
            }
            return;
        }

        if (context == nullptr) {
            internal_::FileContext::load(&context, internal_::file_path);
        }

        const auto center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal(internal_::label, internal_::open_ptr,
                                   ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking |
                                   (internal_::resizable ? 0 : ImGuiWindowFlags_NoResize))) {

            static const float reserve_y = (ImGui::GetFrameHeight() * 4) + ImGui::GetStyle().ItemSpacing.y;

            constexpr float ratio     = 0.25f;
            const float     spacing_x = ImGui::GetStyle().ItemSpacing.x;
            const float     free_x    = ImGui::GetContentRegionAvail().x - spacing_x;

            const float free_x_uno = free_x * ratio;
            const float free_x_des = free_x * (1.f - ratio);

            ImGui::Spacing();
            ImGui::Text("%s", context->path->c_str());

            ImGui::BeginChild("##region_dirs", ImVec2(free_x_uno, -reserve_y), ImGuiChildFlags_Borders);

            for (int i = 0; i < context->dirs_num; i++) {
                const auto &dir = context->dirs[i];
                bool selected = false;
                ImGui::PushID(i);
                ImGui::Selectable(dir.name, &selected);
                ImGui::PopID();

                if (selected) {
                    // TODO
                }
            }

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##region_files", ImVec2(free_x_des, -reserve_y), ImGuiChildFlags_Borders);
            if (ImGui::BeginTable("##files_table", 4,
                ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Sortable)) {

                ImGui::TableSetupColumn("File", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed);
                ImGui::TableHeadersRow();
                for (int i = 0; i < context->files_num; i++) {
                    const auto &file = context->files[i];
                    bool selected = false;

                    ImGui::PushID(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::Selectable(file.name, &selected, ImGuiSelectableFlags_SpanAllColumns);

                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", file.size);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", file.type);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", "DATE"); // TODO

                    ImGui::PopID();

                    if (selected) {
                        // TODO
                    }
                }
                ImGui::EndTable();
            }
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


            if (cancel) {
                *internal_::open_ptr = false;
                internal_::closed    = true;
                ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
                if (context != nullptr) {
                    delete context;
                    context = nullptr;
                }
                return;
            }

            // TODO

            ImGui::EndPopup();
        }

        if (internal_::open_ptr != nullptr && !*internal_::open_ptr) {
            internal_::closed = true;
            if (context != nullptr) {
                delete context;
                context = nullptr;
            }
        }
    }

    inline void SelectFile(const char *file) {
        // TODO
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
