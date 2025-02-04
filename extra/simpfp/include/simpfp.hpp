//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

#include <algorithm>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <string.h>

namespace simpfp {

    namespace internal_ {
        namespace fs = std::filesystem;

        #define SORT_NONE 0

        #define SORT_NAME 1
        #define SORT_SIZE 2
        #define SORT_TYPE 3
        #define SORT_TIME 4

        #define SORT_ASC 1
        #define SORT_DSC 2

        inline char *cpy_str(const char *str) {
            if (str == nullptr)
                return nullptr;
            const auto len  = std::strlen(str);
            auto *buff = new char[len + 1];
            std::strcpy(buff, str);
            return buff;
        }

        inline void cat_str(char *dst, const char *src, const std::size_t dst_size) {
            const std::size_t free_len = dst_size - std::strlen(dst);
            const std::size_t src_len  = std::strlen(src);
            const std::size_t lim_len  = std::min(src_len, free_len);
            std::strncat(dst, src, lim_len);
        }

        inline fs::path load_path(const char *file) {
            if (file == nullptr)
                return fs::current_path();
            const auto path = fs::path(file);
            if (fs::exists(path))
                return fs::is_directory(path) ? path : path.parent_path();
            return fs::current_path();
        }

        inline char* format_time(const fs::file_time_type& ftime) {
            const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
                    ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            const auto tt = std::chrono::system_clock::to_time_t(sctp);
            const auto gmt = std::localtime(&tt);
            char buffer[20];
            std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", gmt);
            return cpy_str(buffer);
        }

        inline std::size_t get_unix_timestamp_ms(const fs::file_time_type& ftime) {
            const auto sctp = std::chrono::time_point_cast<std::chrono::milliseconds>(
                ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
            return static_cast<std::size_t>(sctp.time_since_epoch().count());
        }

        struct FileInfo {
            const fs::path *path     = nullptr;
            const char     *name     = nullptr;
            const char     *type     = nullptr;
            const char     *time     = nullptr;
            std::size_t     size     = 0;
            std::size_t     date     = 0;
            int             index    = 0;
            bool            selected = false;

            FileInfo() = default;

            FileInfo(const FileInfo &other) {
                path     = other.path != nullptr ? (new fs::path(*other.path)) : nullptr;
                name     = cpy_str(other.name);
                type     = cpy_str(other.type);
                time     = cpy_str(other.time);
                selected = other.selected;
                size     = other.size;
                date     = other.date;
            }

            FileInfo(FileInfo &&other) noexcept {
                path     = other.path;
                name     = other.name;
                type     = other.type;
                time     = other.time;
                size     = other.size;
                date     = other.date;
                selected = other.selected;

                other.path = nullptr;
                other.name = nullptr;
                other.type = nullptr;
                other.time = nullptr;
            }

            FileInfo &operator = (FileInfo &&other) noexcept {
                if (&other == this)
                    return *this;

                if (name != nullptr && other.name != name)
                    delete[] name;
                if (type != nullptr && other.type != type)
                    delete[] type;
                if (time != nullptr && other.time != time)
                    delete[] time;
                if (path != nullptr && other.path != path)
                    delete path;

                path     = other.path;
                name     = other.name;
                type     = other.type;
                time     = other.time;
                size     = other.size;
                date     = other.date;
                selected = other.selected;

                other.path = nullptr;
                other.name = nullptr;
                other.type = nullptr;
                other.time = nullptr;

                return *this;
            }

            FileInfo &operator = (const FileInfo &other) {
                if (&other == this)
                    return *this;

                if (name != nullptr && other.name != name)
                    delete[] name;
                if (type != nullptr && other.type != type)
                    delete[] type;
                if (time != nullptr && other.time != time)
                    delete[] time;
                if (path != nullptr && other.path != path)
                    delete path;

                path     = other.path != nullptr ? (new fs::path(*other.path)) : nullptr;
                name     = cpy_str(other.name);
                type     = cpy_str(other.type);
                time     = cpy_str(other.time);
                selected = other.selected;
                size     = other.size;
                date     = other.date;

                return *this;
            }

            ~FileInfo() {
                if (name != nullptr)
                    delete[] name;
                if (type != nullptr)
                    delete[] type;
                if (time != nullptr)
                    delete[] time;
                if (path != nullptr)
                    delete path;
            }
        };

        struct FileContext {
            static constexpr std::size_t buffer_size = 256;
            char                        *buffer      = new char[buffer_size]{};

            const fs::path *path     = nullptr;
            const FileInfo *dirs     = nullptr;
            FileInfo       *files    = nullptr;
            FileInfo      **selected = nullptr;

            int selected_num = 0;
            int files_num    = 0;
            int dirs_num     = 0;

            char sort_by   = SORT_NONE;
            char sort_type = SORT_NONE;

            bool accepted = false;

            ~FileContext() {
                if (selected != nullptr)
                    delete[] selected;
                if (files != nullptr)
                    delete[] files;
                if (dirs != nullptr)
                    delete[] dirs;
                if (path != nullptr)
                    delete path;
                if (buffer != nullptr)
                    delete[] buffer;
            }

            static void load(FileContext **context, const char *path) {
                char sort_by   = SORT_NONE;
                char sort_type = SORT_NONE;

                if (*context != nullptr) {
                    sort_by   = (*context)->sort_by;
                    sort_type = (*context)->sort_type;
                    delete *context;
                }

                const auto file = new fs::path(load_path(path));

                const auto new_context = new FileContext;
                new_context->path      = file;
                new_context->sort_by   = sort_by;
                new_context->sort_type = sort_type;

                const int     init_dirs  = file->has_parent_path() ? 1 : 0;
                constexpr int init_files = 0;

                int dirs_num  = init_dirs;
                int files_num = init_files;
                for (const auto &entry : fs::directory_iterator(*file)) {
                    if (entry.is_directory())
                        dirs_num++;
                    else if (entry.is_regular_file())
                        files_num++;
                }

                const auto files = new FileInfo[files_num];
                const auto dirs  = new FileInfo[dirs_num];

                if (init_dirs > 0) {
                    dirs[0].path     = new fs::path(file->parent_path());
                    dirs[0].selected = false;
                    dirs[0].index    = 0;
                    dirs[0].size     = 0;
                    dirs[0].date     = 0;
                    dirs[0].name     = new char[]{".."};
                    dirs[0].type     = new char[]{".."};
                    dirs[0].time     = new char[]{".."};
                }

                int d_ = init_dirs, f_ = init_files;
                for (const auto &entry : fs::directory_iterator(*file)) {
                    const auto ep = new fs::path(entry.path());
                    if (entry.is_directory()) {
                        dirs[d_].path     = ep;
                        dirs[d_].name     = cpy_str(ep->filename().c_str());
                        dirs[d_].type     = new char[]{""};
                        dirs[d_].time     = new char[]{""};
                        dirs[d_].selected = false;
                        dirs[d_].size     = 0;
                        dirs[d_].date     = 0;
                        dirs[d_].index    = d_;
                        d_++;
                    }
                    else if (entry.is_regular_file()) {
                        const auto time_entry = fs::last_write_time(entry);

                        files[f_].path     = ep;
                        files[f_].type     = cpy_str(ep->has_extension() ? ep->extension().c_str() : new char[]{""});
                        files[f_].name     = cpy_str(ep->filename().c_str());
                        files[f_].date     = get_unix_timestamp_ms(time_entry);
                        files[f_].time     = format_time(time_entry);
                        files[f_].size     = entry.file_size();
                        files[f_].selected = false;
                        files[f_].index    = f_;
                        f_++;
                    }
                }

                new_context->files_num = files_num;
                new_context->files     = files;
                new_context->dirs_num  = dirs_num;
                new_context->dirs      = dirs;

                *context = new_context;

                if (sort_by != SORT_NONE && sort_type != SORT_NONE)
                    FileContext::sort(*context, sort_by, sort_type);
            }

            static void select_single(FileContext *context, const int index) {
                if (context->selected_num == 1 && context->files[index].selected)
                    return;

                for (int c = 0; c < context->files_num; c++)
                    context->files[c].selected = false;
                context->files[index].selected = true;

                if (context->selected != nullptr && context->selected_num == 1) {
                    context->selected[0] = &context->files[index];
                    return;
                }

                if (context->selected != nullptr)
                    delete[] context->selected;
                context->selected     = new FileInfo *[1]{ &context->files[index] };
                context->selected_num = 1;
            }

            static void select_add(FileContext *context, const int index) {
                if (context->selected == nullptr || context->selected_num <= 0) {
                    if (context->selected != nullptr)
                        delete[] context->selected;
                    context->selected              = new FileInfo *[1]{ &context->files[index] };
                    context->selected_num          = 1;
                    context->files[index].selected = true;
                    return;
                }

                if (context->files[index].selected)
                    return;

                const auto new_arr = new FileInfo *[context->selected_num + 1];
                std::memcpy(new_arr, context->selected, sizeof(FileInfo *) * context->selected_num);
                new_arr[context->selected_num] = &context->files[index];
                context->files[index].selected = true;

                delete[] context->selected;

                context->selected_num = context->selected_num + 1;
                context->selected     = new_arr;
            }

            static void select_range(FileContext *context, const int index) {
                if (context->selected == nullptr || context->selected_num <= 0) {
                    if (context->selected != nullptr)
                        delete[] context->selected;
                    context->selected              = new FileInfo *[1]{ &context->files[index] };
                    context->selected_num          = 1;
                    context->files[index].selected = true;
                    return;
                }

                const int idx_min = context->selected[0]->index;
                const int idx_max = context->selected[context->selected_num - 1]->index;

                const int idx_st = std::min(idx_min, index);
                const int idx_ed = std::max(idx_max, index);
                const int size   = idx_ed - idx_st + 1;

                if (size == 1) {
                    select_single(context, index);
                    return;
                }

                if (context->selected_num != size)
                    delete[] context->selected;

                context->selected     = new FileInfo *[size];
                context->selected_num = size;

                for (int c = 0, k = 0; c < context->files_num; c++) {
                    if (c >= idx_st && c <= idx_ed) {
                        context->files[c].selected = true;
                        context->selected[k++]     = &context->files[c];
                        continue;
                    }
                    context->files[c].selected = false;
                }
            }

            static void sort(FileContext *context, const char by, const char order) {
                context->sort_by   = by;
                context->sort_type = order;

                if (by <= 0 || order <= 0)
                    return;

                if (by == SORT_NAME) {
                    if (order == SORT_ASC) {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return std::strcmp(a.name, b.name) < 0;
                        });
                    } else {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return std::strcmp(a.name, b.name) > 0;
                        });
                    }
                }

                else if (by == SORT_TYPE) {
                    if (order == SORT_ASC) {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return std::strcmp(a.type, b.type) < 0;
                        });
                    } else {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return std::strcmp(a.type, b.type) > 0;
                        });
                    }
                }

                else if (by == SORT_SIZE) {
                    if (order == SORT_ASC) {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return a.size < b.size;
                        });
                    } else {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return a.size > b.size;
                        });
                    }
                }

                else if (by == SORT_TIME) {
                    if (order == SORT_ASC) {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return a.date < b.date;
                        });
                    } else {
                        std::sort(context->files, context->files + context->files_num,
                        [] (const FileInfo &a, const FileInfo &b) {
                            return a.date > b.date;
                        });
                    }
                }

            }
        };

        inline const char *label      = nullptr;
        inline const char *file_path  = nullptr;
        inline bool       *open_ptr   = nullptr;
        inline bool        reset      = false;
        inline bool        closed     = true;
        inline bool        resizable  = true;
        inline bool        single     = true;
        inline bool        accept_dir = false;

        inline auto label_accept = "Select";
        inline auto label_cancel = "Cancel";
        inline auto label_create = "New Folder";
        inline auto label_remove = "Delete Folder";

        inline void reset_vars() {
            internal_::open_ptr   = nullptr;
            internal_::label      = nullptr;
            internal_::reset      = false;
            internal_::closed     = true;
            internal_::resizable  = true;
            internal_::single     = true;
            internal_::accept_dir = false;
            internal_::label_accept = "Select";
            internal_::label_cancel = "Cancel";
            internal_::label_create = "New Folder";
            internal_::label_remove = "Delete Folder";
        }

        inline bool show_dialog(const char *label, bool *open = nullptr, const ImVec2 *size = nullptr, const bool resizable = true) {
            if (open != nullptr && !*open) {
                internal_::reset_vars();
                return false;
            }

            if (label == nullptr) {
                internal_::reset_vars();
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

        inline void buttons_dir(bool *create, [[maybe_unused]] bool *remove) {
            ImGui::BeginDisabled(create == nullptr);
            if (ImGui::Button(internal_::label_create)) {
                if (create != nullptr)
                    *create = true;
            }
            ImGui::EndDisabled();

//            ImGui::SameLine();
//            ImGui::BeginDisabled(remove == nullptr);
//            if (ImGui::Button(internal_::label_remove)) {
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
            if (ImGui::Button(internal_::label_cancel, ImVec2(button_width, 0))) {
                if (cancel != nullptr)
                    *cancel = true;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();

            ImGui::BeginDisabled(accept == nullptr);
            if (ImGui::Button(internal_::label_accept, ImVec2(button_width, 0))) {
                if (accept != nullptr)
                    *accept = true;
            }
            ImGui::EndDisabled();
        }
    }

    inline internal_::FileContext *context = nullptr;

    inline void OpenFileDialog(
        const char *label,
        const char *default_path = nullptr,
        const bool accept_dir    = false,
        const char *accept_label = "Select",
        const char *cancel_label = "Cancel",
        const char *create_label = "New Folder"
//        const char *remove_label = "Delete Folder"
    ) {
        internal_::open_ptr     = nullptr;
        internal_::closed       = false;
        internal_::single       = true;
        internal_::reset        = true;
        internal_::label        = label;
        internal_::accept_dir   = accept_dir;
        internal_::file_path    = default_path;
        internal_::label_accept = accept_label;
        internal_::label_cancel = cancel_label;
        internal_::label_create = create_label;
//        internal_::label_remove = remove_label;
    }

    inline bool FileDialogOpen() {
        return !internal_::closed;
    }

    inline void CloseFileDialog() {
        *internal_::open_ptr = false;
        internal_::closed    = true;
        if (context != nullptr) {
            delete context;
            context = nullptr;
        }
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

            const bool key_shift = ImGui::IsKeyPressed(ImGuiKey_LeftShift) || ImGui::IsKeyDown(ImGuiKey_LeftShift);
            const bool key_ctrl  = ImGui::IsKeyPressed(ImGuiKey_LeftCtrl) || ImGui::IsKeyDown(ImGuiKey_LeftCtrl);

            if (ImGui::IsKeyPressed(ImGuiKey_Escape) || ImGui::IsKeyDown(ImGuiKey_Escape)) {
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

            ImGui::Spacing();
            ImGui::Text("%s", context->path->c_str());
            ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize("*").x
                - ImGui::GetStyle().FramePadding.x - ImGui::GetStyle().ItemSpacing.x);
            ImGui::TextDisabled("%s", (key_shift || key_ctrl) ? "*" : "");

            ImGui::BeginChild("##region_dirs", ImVec2(free_x_uno, -reserve_y), ImGuiChildFlags_Borders);

            for (int i = 0; i < context->dirs_num; i++) {
                const auto &dir = context->dirs[i];
                bool selected = false;
                ImGui::PushID(i);
                ImGui::Selectable(dir.name, &selected);
                ImGui::PopID();

                if (selected) {
                    internal_::FileContext::load(&context, internal_::fs::absolute(*dir.path).c_str());
                }
            }

            ImGui::EndChild();

            ImGui::SameLine();

            ImGui::BeginChild("##region_files", ImVec2(free_x_des, -reserve_y), ImGuiChildFlags_Borders);

            bool selection_change = false;
            ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(10.0f, 3.0f));
            if (ImGui::BeginTable("##files_table", 4,
                ImGuiTableFlags_NoSavedSettings | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_Sortable |
                ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingFixedFit)) {

                ImGui::TableSetupColumn(" File", ImGuiTableColumnFlags_WidthStretch, 0.35f);
                ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
                ImGui::TableSetupColumn("Date", ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide);
                ImGui::TableHeadersRow();

                if (const auto sp = ImGui::TableGetSortSpecs(); sp != nullptr && sp->SpecsDirty) {
                    if (const auto cs = sp->Specs)
                        internal_::FileContext::sort(context, (cs->ColumnIndex + 1), cs->SortDirection);
                    sp->SpecsDirty = false;
                }

                for (int i = 0; i < context->files_num; i++) {
                    const auto &file = context->files[i];
                    bool selected = file.selected;

                    ImGui::PushID(i);

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();
                    ImGui::SameLine(0, 5);
                    ImGui::Selectable(file.name, &selected, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);

                    ImGui::TableNextColumn();
                    ImGui::Text("%lu", file.size);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", file.type);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", file.time);
                    ImGui::SameLine(0, 5);
                    ImGui::TextDisabled(" ");

                    ImGui::PopID();

                    if (selected == file.selected)
                        continue;

                    selection_change = true;

                    if (!internal_::single && key_ctrl) {
                        internal_::FileContext::select_add(context, i);
                        continue;
                    }

                    if (!internal_::single && key_shift) {
                        internal_::FileContext::select_range(context, i);
                        continue;
                    }

                    internal_::FileContext::select_single(context, i);
                }
                ImGui::EndTable();
            }
            ImGui::PopStyleVar();
            ImGui::EndChild();

            char                 *buffer  = context->buffer;
            constexpr std::size_t max_len = internal_::FileContext::buffer_size - 1;

            if (selection_change) {
                std::memset(buffer, 0, internal_::FileContext::buffer_size);

                if (context->selected != nullptr && context->selected_num == 1) {
                    const auto str_src = context->selected[0]->name;
                    std::strncpy(buffer, str_src, std::min(max_len, std::strlen(str_src)));
                }

                else if (context->selected != nullptr && context->selected_num > 1) {
                    for (int k = 0; k < context->selected_num; k++) {
                        if (k > 0)
                            internal_::cat_str(buffer, "; ", max_len);
                        const auto str_src = context->selected[k]->name;
                        internal_::cat_str(buffer, str_src, max_len);
                    }
                }
            }

            if (context->selected_num > 1)
                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.5f);
            ImGui::BeginDisabled(context->selected_num > 1);
            internal_::full_width_input(buffer, max_len);
            ImGui::EndDisabled();
            if (context->selected_num > 1)
                ImGui::PopStyleVar();

            ImGui::Spacing();

            const bool can_save = internal_::accept_dir || context->selected_num > 0 || std::strlen(buffer) > 0;

            bool create = false;
            bool remove = false;
            bool cancel = false;
            bool accept = false;

            internal_::buttons_dir(&create, &remove);
            ImGui::Spacing();
            internal_::buttons_action(&cancel, can_save ? &accept : nullptr);

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

            if (create) {
                // TODO
                ImGui::EndPopup();
                return;
            }

            if (accept) {
                context->accepted = true;
                ImGui::EndPopup();
                return;
            }

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

    inline int CountSelected() {
        if (context == nullptr || context->selected == nullptr)
            return 0;
        return context->selected_num;
    }

    inline const char *CurrentPath() {
        if (context == nullptr || context->path == nullptr)
            return nullptr;
        return context->path->c_str();
    }

    inline const char **PeekSelected(std::size_t *const out_n) {
        if (context == nullptr || context->selected_num <= 0)
            return nullptr;

        *out_n = context->selected_num;

        if (internal_::accept_dir) {
            // TODO
            return nullptr;
        }

        // TODO
        return nullptr;
    }

    inline const char **FilesAccepted(std::size_t *const out_n) {
        internal_::single = false;
        if (context == nullptr || context->selected_num <= 0 || !context->accepted)
            return nullptr;

        context->accepted = false;
        return nullptr;
    }

    inline bool FileAccepted(char *buffer_out, const std::size_t size) {
        if (context == nullptr || !context->accepted)
            return false;

        context->accepted = false;

        if (context->selected == nullptr || context->selected_num <= 0) {
            if (context->buffer == nullptr || std::strlen(context->buffer) <= 0) {
                if (internal_::accept_dir) {
                    std::memset(buffer_out, 0, size);
                    std::strncpy(buffer_out, context->path->c_str(), size);
                    return true;
                }
                return false;
            }

            std::memset(buffer_out, 0, size);
            std::strncpy(buffer_out, internal_::fs::absolute(*context->path / context->buffer).c_str(), size);
            return true;
        }

        std::memset(buffer_out, 0, size);
        std::strncpy(buffer_out,
                     internal_::fs::absolute(context->selected[0]->path->parent_path() / context->buffer).c_str(),
                     size);
        return true;
    }

}

#endif //SIMPFP_HPP
