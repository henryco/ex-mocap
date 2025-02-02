//
// Created by xd on 02/02/25.
//

#ifndef SIMPFP_HPP
#define SIMPFP_HPP

namespace simpfp {

    inline bool ShowFileDialog(const char *label, char *buffer, std::size_t buffer_size, bool *open = nullptr) {
        if (open != nullptr && !*open)
            return false;

        ImGui::OpenPopup(label, ImGuiPopupFlags_NoReopen);
        const auto center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        if (ImGui::BeginPopupModal(label, open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking)) {

            ImGui::Text("File");
            ImGui::EndPopup();
            return true;
        }

        if (open != nullptr)
            *open = false;
        return false;
    }

}

#endif //SIMPFP_HPP
