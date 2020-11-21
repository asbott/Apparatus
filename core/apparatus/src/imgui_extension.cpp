#include "pch.h"

#include "imgui_extension.h"

#include <imgui_internal.h>

#include "input_codes.h"

#include "archive.h"

ImGuiExtensionStyle g_ext_style;

namespace ImGui {

    bool can_align_to_right() {
        auto wnd = ImGui::GetCurrentWindow(); 
        // Can only align if NOT in a popup, NOT in a child menu, NOT in a menu bar
        return !(wnd->Flags & ImGuiWindowFlags_Popup) && !(wnd->Flags & ImGuiWindowFlags_ChildMenu) && !wnd->DC.MenuBarAppending;
    }

    void set_layout(str_ptr_t label) {
        f32 right_offset = GetWindowWidth() * g_ext_style.right_align;
        AlignTextToFramePadding();
        Text("%s", label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", label);
        }
        
        if (can_align_to_right()) {
            SameLine(right_offset);
            f32 item_width = GetWindowWidth() - right_offset - GetWindowWidth() * g_ext_style.right_align_padding;
            SetNextItemWidth(item_width);
        } else {
            SameLine();
        }
        
    }

    void set_layout_for_known_width(str_ptr_t label, f32 width) {
        f32 right_offset = GetWindowContentRegionWidth() - width - g_ext_style.right_align_padding * GetWindowWidth();
        AlignTextToFramePadding();
        Text("%s", label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", label);
        } 
        if (can_align_to_right()) {
            SameLine(right_offset);
        } else {
            SameLine();
        }
        
    }

    bool RInputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        str_t<128> id = "";
        sprintf(id, "##%llu", (unsigned long long)buf);
        bool ret = InputText(id, buf, buf_size, flags, callback, user_data);
        return ret;
    }
    bool RInputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        str_t<128> id = "";
        sprintf(id, "##%llu", (unsigned long long)buf);
        bool ret = InputTextMultiline(id, buf, buf_size, size, flags, callback, user_data);
        return ret;
    }
    bool RInputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        str_t<128> id = "";
        sprintf(id, "##%llu", (unsigned long long)buf);
        bool ret = InputTextWithHint(id, hint, buf, buf_size, flags, callback, user_data);
        return ret;
    }
    bool RInputFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputFloat("", v, step, step_fast, format, flags);
        PopID();
        return ret;
    }
    bool RInputFloat2(const char* label, float v[2], const char* format, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputFloat2("", v, format, flags);
        PopID();
        return ret;
    }
    bool RInputFloat3(const char* label, float v[3], const char* format, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputFloat3("", v, format, flags);
        PopID();
        return ret;
    }
    bool RInputFloat4(const char* label, float v[4], const char* format, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputFloat4("", v, format, flags);
        PopID();
        return ret;
    }
    bool RInputInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputInt("", v, step, step_fast, flags);
        PopID();
        return ret;
    }
    bool RInputInt2(const char* label, int v[2], ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputInt2("", v, flags);
        PopID();
        return ret;
    }
    bool RInputInt3(const char* label, int v[3], ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputInt3("", v, flags);
        PopID();
        return ret;
    }
    bool RInputInt4(const char* label, int v[4], ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputInt4("", v, flags);
        PopID();
        return ret;
    }
    bool RInputDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = InputDouble("", v, step, step_fast, format, flags);
        PopID();
        return ret;
    }

    bool RDragFloat(const char* label, float* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragFloat("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragFloat2(const char* label, float v[2], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragFloat2("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragFloat3(const char* label, float v[3], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragFloat3("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragFloat4(const char* label, float v[4], float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragFloat4("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RDragFvec2(const char* label, mz::fvec2* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        f32 right_offset = GetWindowWidth() * g_ext_style.right_align;
        AlignTextToFramePadding();
        Text("%s", label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", label);
        }
        auto wnd = ImGui::GetCurrentWindow();
        if ((wnd->Flags & ImGuiWindowFlags_Popup) != 0 || wnd->MenuBarHeight() > 0) {
            SameLine();
        } else {
            SameLine(right_offset - ImGui::GetStyle().ItemSpacing.x * 2.f);
        }
        f32 item_width = GetWindowWidth() - right_offset - GetWindowWidth() * g_ext_style.right_align_padding;
        f32 labelx_width = ImGui::CalcTextSize("X").x;
        f32 labely_width = ImGui::CalcTextSize("Y").x;
        f32 width_for_fields = item_width - (labelx_width + labely_width);
        f32 width_per_field = width_for_fields / 2.f - ImGui::GetStyle().ItemSpacing.x / 2.f;

        ImGui::PushID(v);
        ImGui::Text("X");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesx = DragFloat("##x", &v->x, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesy = DragFloat("##y", &v->y, v_speed, v_min, v_max, format, flags);
        ImGui::PopID();

        return yesx || yesy;
    }
    bool RDragFvec3(const char* label, mz::fvec3* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        f32 right_offset = GetWindowWidth() * g_ext_style.right_align;
        AlignTextToFramePadding();
        Text("%s", label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", label);
        }
        auto wnd = ImGui::GetCurrentWindow();
        if ((wnd->Flags & ImGuiWindowFlags_Popup) != 0 || wnd->MenuBarHeight() > 0) {
            SameLine();
        } else {
            SameLine(right_offset - ImGui::GetStyle().ItemSpacing.x * 3.f);
        }
        f32 item_width = GetWindowWidth() - right_offset - GetWindowWidth() * g_ext_style.right_align_padding;
        f32 labelx_width = ImGui::CalcTextSize("X").x;
        f32 labely_width = ImGui::CalcTextSize("Y").x;
        f32 labelz_width = ImGui::CalcTextSize("Z").x;
        f32 width_for_fields = item_width - (labelx_width + labely_width + labelz_width);
        f32 width_per_field = width_for_fields / 3.f - ImGui::GetStyle().ItemSpacing.x / 3.f;

        ImGui::PushID(v);
        ImGui::Text("X");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesx = DragFloat("##x", &v->x, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesy = DragFloat("##y", &v->y, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("Z");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesz = DragFloat("##z", &v->z, v_speed, v_min, v_max, format, flags);
        ImGui::PopID();

        return yesx || yesy || yesz;
    }
    bool RDragFvec4(const char* label, mz::fvec4* v, float v_speed, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        f32 right_offset = GetWindowWidth() * g_ext_style.right_align;
        AlignTextToFramePadding();
        Text("%s", label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip("%s", label);
        }
        auto wnd = ImGui::GetCurrentWindow();
        if ((wnd->Flags & ImGuiWindowFlags_Popup) != 0 || wnd->MenuBarHeight() > 0) {
            SameLine();
        } else {
            SameLine(right_offset - ImGui::GetStyle().ItemSpacing.x * 4.f);
        }
        f32 item_width = GetWindowWidth() - right_offset - GetWindowWidth() * g_ext_style.right_align_padding;
        f32 labelx_width = ImGui::CalcTextSize("X").x;
        f32 labely_width = ImGui::CalcTextSize("Y").x;
        f32 labelz_width = ImGui::CalcTextSize("Z").x;
        f32 labelw_width = ImGui::CalcTextSize("W").x;
        f32 width_for_fields = item_width - (labelx_width + labely_width + labelz_width + labelw_width);
        f32 width_per_field = width_for_fields / 4.f - ImGui::GetStyle().ItemSpacing.x / 4.f;

        ImGui::PushID(v);
        ImGui::Text("X");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesx = DragFloat("##x", &v->x, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("Y");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesy = DragFloat("##y", &v->y, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("Z");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesz = DragFloat("##z", &v->z, v_speed, v_min, v_max, format, flags);
        ImGui::SameLine();
        ImGui::Text("W");
        ImGui::SameLine();
        SetNextItemWidth(width_per_field);
        bool yesw = DragFloat("##w", &v->w, v_speed, v_min, v_max, format, flags);
        ImGui::PopID();

        return yesx || yesy || yesz || yesw;
    }

    bool RDragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_current_min);
        bool ret = DragFloatRange2("", v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, flags);
        PopID();
        return ret;
    }
    bool RDragFloatRange2(const char* label, mz::fvec2* v_current_minmax, float v_speed, float v_min, float v_max, const char* format, const char* format_max, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_current_minmax);
        bool ret = DragFloatRange2("", &v_current_minmax->min, &v_current_minmax->max, v_speed, v_min, v_max, format, format_max, flags);
        PopID();
        return ret;
    }
    bool RDragInt(const char* label, int* v, float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragInt("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragInt2(const char* label, int v[2], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragInt2("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragInt3(const char* label, int v[3], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragInt3("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragInt4(const char* label, int v[4], float v_speed, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = DragInt3("", v, v_speed, v_min, v_max, format, flags);
        PopID();
        return ret;
    }
    bool RDragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed, int v_min, int v_max, const char* format, const char* format_max, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_current_min);
        bool ret = DragIntRange2("", v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, flags);
        PopID();
        return ret;
    }
    bool RDragIntRange2(const char* label, mz::ivec2* v_current_minmax, float v_speed, s32 v_min, s32 v_max, const char* format, const char* format_max, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_current_minmax);
        bool ret = DragIntRange2("", &v_current_minmax->min, &v_current_minmax->max, v_speed, v_min, v_max, format, format_max, flags);
        PopID();
        return ret;
    }

    bool RSliderFloat(const char* label, float* v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderFloat("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderFloat2(const char* label, float v[2], float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderFloat2("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderFloat3(const char* label, float v[3], float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderFloat3("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderFloat4(const char* label, float v[4], float v_min, float v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderFloat4("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderAngle(const char* label, float* v_rad, float v_degrees_min, float v_degrees_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_rad);
        bool ret = SliderAngle("", v_rad, v_degrees_min, v_degrees_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderInt(const char* label, int* v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderInt("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderInt2(const char* label, int v[2], int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderInt2("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderInt3(const char* label, int v[3], int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderInt3("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }

    bool RSliderInt4(const char* label, int v[4], int v_min, int v_max, const char* format, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v);
        bool ret = SliderInt4("", v, v_min, v_max, format, flags);
        PopID();
        return ret;
    }


    bool RButton(str_ptr_t label) {
        auto& p = GetExtensionStyle().min_button_padding;
        PushStyleVar(ImGuiStyleVar_FramePadding, { p.x, p.y });

        bool pressed = Button(label);

        PopStyleVar();

        return pressed;
    }

    bool RSection(str_ptr_t label) {
        ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow 
                                | ImGuiTreeNodeFlags_DefaultOpen 
                                | ImGuiTreeNodeFlags_OpenOnDoubleClick 
                                | ImGuiTreeNodeFlags_Framed 
                                /*| ImGuiTreeNodeFlags_NoIndentOnOpen*/ 
                                | ImGuiTreeNodeFlags_NoAutoOpenOnLog
                                | ImGuiTreeNodeFlags_AllowItemOverlap
                                | ImGuiTreeNodeFlags_SpanAvailWidth;
    
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { ImGui::GetStyle().ItemSpacing.x, 0 });

    f32 border = ImGui::GetStyle().FrameBorderSize;
    SetCursorPosX(-border * 2.f);
    SetNextItemWidth(GetWindowContentRegionWidth() + border * 2.f * 2.f);

    bool open = ImGui::TreeNodeEx(label, flags);;

    ImGui::PopStyleVar();
    ImGui::PopStyleVar();

    return open;
}




    bool RCheckbox(const char* label, bool* v) {
        set_layout_for_known_width(label, ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetStyle().FrameBorderSize * 2);
        PushID(v);
        bool ret = Checkbox("", v);
        PopID();
        return ret;
    }

    void RCheckboxReadonly(const char* label, bool v) {
        set_layout_for_known_width(label, ImGui::GetStyle().FramePadding.y * 2 + ImGui::GetStyle().FrameBorderSize * 2);
        static name_str_t id = "";
        sprintf(id, "##%s", label);
        Checkbox(id, &v);
    }

    bool RBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags) {
        set_layout(label);
        str_t<22> id = "";
        sprintf(id, "##%llu", (unsigned long long)preview_value + (unsigned long long)label);
        bool ret = BeginCombo(id, preview_value, flags);
        return ret;
    }

    void REndCombo() {
        ImGui::EndCombo();
    }

    bool RColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags ) {
        set_layout(label);
        PushID((s32)(uintptr_t)label + (s32)(uintptr_t)col);
        bool ret = ColorEdit3("", col, flags);
        PopID();
        return ret;
    }
    bool RColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags){
        set_layout(label);
        PushID((s32)(uintptr_t)label + (s32)(uintptr_t)col);
        bool ret = ColorEdit4("", col, flags);
        PopID();
        return ret;
    }
    bool RColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags){
        set_layout(label);
        PushID((s32)(uintptr_t)label + (s32)(uintptr_t)col);
        bool ret = ColorPicker3("", col, flags);
        PopID();
        return ret;
    }
    bool RColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags, const float* ref_col){
        set_layout(label);
        PushID((s32)(uintptr_t)label + (s32)(uintptr_t)col);
        bool ret = ColorPicker4("", col, flags, ref_col);
        PopID();
        return ret;
    }

    ImGuiExtensionStyle& GetExtensionStyle() {
        return g_ext_style;
    }

    void SaveStyleToDisk(str_ptr_t path) {
        Binary_Archive archive(path);

        archive.write<ImGuiStyle>("imgui_style", ImGui::GetStyle());
        archive.write<ImGuiExtensionStyle>("imgui_ext_style", ImGui::GetExtensionStyle());

        archive.flush();
    }
    void LoadStyleFromDisk(str_ptr_t path) {
        Binary_Archive archive(path);

        if (archive.is_valid_id("imgui_style")) {
            ImGuiStyle& loaded_style = archive.read<ImGuiStyle>("imgui_style");
            memcpy(&ImGui::GetStyle(), &loaded_style, sizeof(ImGuiStyle));
        }
            
        if (archive.is_valid_id("imgui_ext_style")) {
            ImGuiExtensionStyle& loaded_style = archive.read<ImGuiExtensionStyle>("imgui_ext_style");
            memcpy(&ImGui::GetExtensionStyle(), &loaded_style, sizeof(ImGuiExtensionStyle));
        }
    }

    namespace Filters {
        int AlphaNumericNoSpace(ImGuiTextEditCallbackData* data) {
            if ((data->EventChar >= '0' && data->EventChar <= '9') 
             || (data->EventChar >= 'A' && data->EventChar <= 'Z')
             || (data->EventChar >= 'a' && data->EventChar <= 'z')
             || (data->EventChar == '_')) 
                 return 0;
            return 1;
        }
        int AlphaNumeric(ImGuiTextEditCallbackData* data) {
            if ((data->EventChar >= '0' && data->EventChar <= '9') 
             || (data->EventChar >= 'A' && data->EventChar <= 'Z')
             || (data->EventChar >= 'a' && data->EventChar <= 'z')
             || (data->EventChar == '_')
             || (data->EventChar == ' ')) 
                 return 0;
            return 1;
        }
    }
    
}