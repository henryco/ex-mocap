//
// Created by xd on 01/06/25.
//

#include "../../../../headers/nodes/components/chromakey/chromakey_gui.h"
#include "../../../../headers/nodes/components/chromakey/chromakey_logic.h"

#include "../../../../headers/imgui/strings/gui_strings.h"
#include "../../../../headers/logic/logic_globals.h"
#include "../../../../headers/logic/logic_mappers.h"
#include "../../../../headers/logic/logic_state.h"
#include "../../../../headers/utils/gui_utils.h"

#include "../../../../../extra/data/struct/time_stack.h"

#include <IconsLucide.h>
#include "imgui.h"

namespace ex::gui::node::chromakey {

    namespace {
        namespace lnm = logic::node::chromakey;
        using chk_node_t = lnm::ChromakeyNode;

        void fn_apply(const unsigned node, const lnm::config_t &data) {
            if (chk_node_t *instance = dynamic_cast<chk_node_t *>(logic::state::get_node(node)))
                instance->set_config(data);
        }

        // up to 1KB debounce queue
        data::time_stack<23, id_type_32, lnm::config_t, &fn_apply> time_stack;

        const char *prev_name_() {
            static char out_name[128];
            sprintf(out_name, "%s %s", ICON_LC_SWATCH_BOOK, strings::node_component_chromakey_name);
            return out_name;
        }

        void parcel_free_(GuiNodeParcel *parcel) {
            if (parcel == nullptr)
                return;
            if (parcel->data != nullptr)
                ::free(parcel->data);
            ::free(parcel);
        }

        void hsv_to_hsl(float h, float s_v, float v, float *h_out, float *s_l_out, float *l_out) {
            const float l = v * (1.0f - s_v * 0.5f);
            float s_l = 0.0f;
            if (l > 0.0f && l < 1.0f) {
                float min_l = (l < 1.0f - l) ? l : (1.0f - l);
                s_l = (v - l) / min_l;
            }
            *h_out   = h;
            *s_l_out = s_l;
            *l_out   = l;
        }

        void hsl_to_hsv(float h, float s_l, float l, float *h_out, float *s_v_out, float *v_out) {
            const float v = l + s_l * ((l < 0.5f) ? l : (1.0f - l));
            float s_v = 0.0f;
            if (v > 0.0f)
                s_v = 2.0f * (1.0f - l / v);
            *h_out   = h;
            *s_v_out = s_v;
            *v_out   = v;
        }
    }

    unsigned long int get_node_type_id() {
        return chk_node_t::get_static_type_id();
    }

    const char *get_node_name() {
        return strings::node_component_chromakey_name;
    }

    const char *get_node_preview() {
        static const char *name = prev_name_();
        return name;
    }

    bool draw_tree_node(TreeNode *n, const bool nested, const bool selected, const bool collapse, const bool expand) {
        static char out_name[128]{};
        sprintf(out_name, "%s %s", ICON_LC_SWATCH_BOOK, n->name);
        return generic_node(out_name, false, selected, nested, expand, collapse);
    }

    bool draw_node_window(GuiNodeWindow *window) {
        const id_type_32 node_id = window->node->id_node;

        auto *base = dynamic_cast<chk_node_t *>(logic::state::get_node(node_id));
        if (base == nullptr || base->get_node_id() != node_id)
            return true;

        const double time_now = ImGui::GetTime();

        lnm::config_t config;
        if (const lnm::config_t *state = time_stack.retrieve_consume(node_id, time_now, 1.f))
            ::memcpy(&config, state, sizeof(lnm::config_t));
        else
            ::memcpy(&config, &base->config, sizeof(lnm::config_t));


        static const ex::State &state = ex::logic::globals::state();

        {
            ImGui::SeparatorText(strings::node_component_chromakey_id);

            static char buff[8] = "";
            ::snprintf(buff, 7, "%u", base->get_image_id());

            const auto input_pos  = ImGui::GetCursorScreenPos();
            const auto input_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight());
            ImGui::SetNextItemWidth(input_size.x);
            ImGui::BeginDisabled(true);
            if (ImGui::InputText("###MorphId", buff, 8, ImGuiInputTextFlags_CharsDecimal)) {
                base->set_image_id(::atoi(buff));
            }
            ImGui::EndDisabled();

            constexpr float alpha = 1.f;
            ImGui::GetWindowDrawList()->AddRect(input_pos,
                                                ImVec2(input_pos.x + input_size.x, input_pos.y + input_size.y),
                                                IM_COL32(0, 0, 0, 90 * alpha));
            ImGui::NewLine();
        }

        {
            ImGui::TextDisabled("Input ID: %u", base->get_input_id());
            ImGui::TextDisabled("Image  W: %u", base->get_image_w());
            ImGui::TextDisabled("Image  H: %u", base->get_image_h());
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_chromakey_color, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            static ImVec4 prev_color;

            {// Background color
                ImVec4 col_bg = { config.replace.r, config.replace.g, config.replace.b, config.replace.a };
                if (color_selector(strings::node_component_chromakey_color_s, strings::node_component_chromakey_color_b,
                                   &col_bg, &prev_color, true)) {
                    config.replace.r = col_bg.x;
                    config.replace.g = col_bg.y;
                    config.replace.b = col_bg.z;
                    config.replace.a = col_bg.w;
                    time_stack.force(node_id, config);
                }
            }

            {// Range color low
                float h, s, v;
                ImVec4 col_hi;
                hsl_to_hsv(config.key.hi.h, config.key.hi.s, config.key.hi.l, &h, &s, &v);
                ImGui::ColorConvertHSVtoRGB(h, s, v, col_hi.x, col_hi.y, col_hi.z);
                if (color_selector(strings::node_component_chromakey_color_s, strings::node_component_chromakey_color_h,
                                   &col_hi, &prev_color)) {
                    ImGui::ColorConvertRGBtoHSV(col_hi.x, col_hi.y, col_hi.z, h, s, v);
                    hsv_to_hsl(h, s, v, &config.key.hi.h, &config.key.hi.s, &config.key.hi.l);
                    time_stack.force(node_id, config);
                }
            }

            {// Range color high
                float h, s, v;
                ImVec4 col_lo;
                hsl_to_hsv(config.key.lo.h, config.key.lo.s, config.key.lo.l, &h, &s, &v);
                ImGui::ColorConvertHSVtoRGB(h, s, v, col_lo.x, col_lo.y, col_lo.z);
                if (color_selector(strings::node_component_chromakey_color_s, strings::node_component_chromakey_color_l,
                                   &col_lo, &prev_color)) {
                    ImGui::ColorConvertRGBtoHSV(col_lo.x, col_lo.y, col_lo.z, h, s, v);
                    hsv_to_hsl(h, s, v, &config.key.lo.h, &config.key.lo.s, &config.key.lo.l);
                    time_stack.force(node_id, config);
                }
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_chromakey_downscale, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            {// Scale factor
                static char buff[9] = "";
                ::sprintf(buff, "%g", config.upscale.scale);
                if (ImGui::InputText(strings::node_component_chromakey_downscale_f, buff, 9, ImGuiInputTextFlags_CharsScientific)) {
                    const double value = ::atof(buff);
                    config.upscale.scale = value;
                    if (value <= 0)
                        config.upscale.scale = 1.f;
                    if (value > 10) // sanity check, downscale 100, wtf???
                        config.upscale.scale = 1.f;
                    time_stack.push(time_now, node_id, config);
                }
            }

            // Linear interpolation
            if (ImGui::Checkbox(strings::node_component_chromakey_downscale_l, &config.upscale.linear))
                time_stack.push(time_now, node_id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_chromakey_preprocess, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            ImGui::BeginDisabled(!config.before.blur);
            {// Blur kernel half-size
                static char buff[9] = "";
                ::sprintf(buff, "%hhu", config.before.power);
                if (ImGui::InputText(strings::node_component_chromakey_preprocess_p, buff, 9, ImGuiInputTextFlags_CharsDecimal)) {
                    const int value    = ::atoi(buff);
                    config.before.power = value;
                    if (value < 0)
                        config.before.power = 0;
                    if (value > 20)
                        config.before.power = 20;
                    time_stack.push(time_now, node_id, config);
                }
            }
            ImGui::EndDisabled();

            // Enable blur
            if (ImGui::Checkbox(strings::node_component_chromakey_preprocess_e, &config.before.blur))
                time_stack.push(time_now, node_id, config);

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::CollapsingHeader(strings::node_component_chromakey_refinement, ImGuiTreeNodeFlags_Framed)) {
            ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
            ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0.35));

            ImGui::BeginDisabled(config.refine.morph == lnm::mode_e::NONE);
            {// Morph kernel half-size
                static char buff[9] = "";
                ::sprintf(buff, "%hhu", config.refine.power);
                if (ImGui::InputText(strings::node_component_chromakey_refinement_p, buff, 9, ImGuiInputTextFlags_CharsDecimal)) {
                    const int value    = ::atoi(buff);
                    config.refine.power = value;
                    if (value < 0)
                        config.refine.power = 0;
                    if (value > 20)
                        config.refine.power = 20;
                    time_stack.push(time_now, node_id, config);
                }
            }
            ImGui::EndDisabled();

            int op = static_cast<int>(config.refine.morph);

            if (ImGui::RadioButton(strings::node_component_morph_mode_n, &op, static_cast<int>(lnm::mode_e::NONE))) {
                config.refine.morph = static_cast<lnm::mode_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            if (ImGui::RadioButton(strings::node_component_morph_mode_e, &op, static_cast<int>(lnm::mode_e::ERODE))) {
                config.refine.morph = static_cast<lnm::mode_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            if (ImGui::RadioButton(strings::node_component_morph_mode_d, &op, static_cast<int>(lnm::mode_e::DILATE))) {
                config.refine.morph = static_cast<lnm::mode_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            if (ImGui::RadioButton(strings::node_component_morph_mode_o, &op, static_cast<int>(lnm::mode_e::OPEN))) {
                config.refine.morph = static_cast<lnm::mode_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            if (ImGui::RadioButton(strings::node_component_morph_mode_c, &op, static_cast<int>(lnm::mode_e::CLOSE))) {
                config.refine.morph = static_cast<lnm::mode_e>(op);
                time_stack.push(time_now, node_id, config);
            }

            ImGui::PopStyleColor();
            ImGui::PopStyleVar();
            ImGui::NewLine();
        }

        return true;
    }

    GuiNodeParcel *store(const TreeNode *node) {
        const chk_node_t *base = dynamic_cast<chk_node_t *>(ex::logic::state::get_node(node->id_node));
        if (base == nullptr)
            return nullptr;

        io::byte_array data = base->serialize();

        GuiNodeParcel *parcel = static_cast<GuiNodeParcel *>(::malloc(sizeof(GuiNodeParcel)));
        parcel->free          = &parcel_free_;
        parcel->size          = data.size;
        parcel->data          = nullptr;

        if (!data.empty()) {
            parcel->data = static_cast<unsigned char *>(::malloc(data.size));
            ::memcpy(parcel->data, data.data, data.size);
        }

        io::byte_array::free(data);
        return parcel;
    }

    void restore(TreeNode *node, GuiNodeParcel *parcel) {
        if (parcel == nullptr)
            return;

        if (auto *trs_node = dynamic_cast<chk_node_t *>(ex::logic::state::get_node(node->id_node))) {
            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        auto instance = ex::logic::node_mappers()[node->id_type].instance(node);
        if (auto *trs_node = dynamic_cast<chk_node_t *>(instance)) {
            ex::logic::state::store_node(node->id_node, instance);

            auto byte_data = io::byte_array::from(parcel->data, parcel->size);
            trs_node->deserialize(byte_data);
            return;
        }

        abort_("Invalid node mapper type");
    }

    void update_tree_node(const TreeNode *before, TreeNode *after) {
        if (!(before->state >>= node_state::disabled) &&
            (after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<chk_node_t *>(logic::state::get_node(after->id_node)))
                base->disable();
            return;
        }
        if ((before->state >>= node_state::disabled) &&
            !(after->state >>= node_state::disabled)) {
            if (const auto base = dynamic_cast<chk_node_t *>(logic::state::get_node(after->id_node)))
                base->enable();
        }
    }

}