#include "pch.h"

#include "imgui_extension.h"

#include <imgui_internal.h>

#include "input_codes.h"

ImGuiExtensionStyle g_ext_style;



namespace ImGui {

    void set_layout(str_ptr_t label) {
        f32 right_offset = GetWindowWidth() * g_ext_style.right_align;
        AlignTextToFramePadding();
        Text(label); 
        if (ImGui::IsKeyDown(AP_KEY_LEFT_CONTROL) && ImGui::IsItemHovered()) {
            ImGui::SetTooltip(label);
        }
        auto wnd = ImGui::GetCurrentWindow();
        log_trace(wnd->MenuBarHeight());
        if ((wnd->Flags & ImGuiWindowFlags_Popup) != 0 || wnd->MenuBarHeight() > 0) {
            SameLine();
        } else {
            SameLine(right_offset);
            f32 item_width = GetWindowWidth() - right_offset - GetWindowWidth() * g_ext_style.right_align_padding;
            SetNextItemWidth(item_width);
        }
        
    }

    bool RInputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        PushID(buf);
        bool ret = InputText("", buf, buf_size, flags, callback, user_data);
        PopID();
        return ret;
    }
    bool RInputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        PushID(buf);
        bool ret = InputTextMultiline("", buf, buf_size, size, flags, callback, user_data);
        PopID();
        return ret;
    }
    bool RInputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        PushID(buf);
        bool ret = InputTextWithHint("", hint, buf, buf_size, flags, callback, user_data);
        PopID();
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
    bool RDragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed, float v_min, float v_max, const char* format, const char* format_max, ImGuiSliderFlags flags) {
        set_layout(label);
        PushID(v_current_min);
        bool ret = DragFloatRange2("", v_current_min, v_current_max, v_speed, v_min, v_max, format, format_max, flags);
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

    bool RCheckbox(const char* label, bool* v) {
        set_layout(label);
        PushID(v);
        bool ret = Checkbox("", v);
        PopID();
        return ret;
    }

    bool RBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags) {
        set_layout(label);
        str_t<22> id = "";
        sprintf(id, "##%llu", (u64)preview_value + (u64)label);
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

    str_ptr_t GetStyleVarName(ImGuiStyleVar var) {
        switch (var)
        {
            case ImGuiStyleVar_Alpha:               return "Alpha";
            case ImGuiStyleVar_WindowPadding:       return "Window Padding";
            case ImGuiStyleVar_WindowRounding:      return "Window Rounding";
            case ImGuiStyleVar_WindowBorderSize:    return "Window Border Size"; 
            case ImGuiStyleVar_WindowMinSize:       return "Window Min Size";
            case ImGuiStyleVar_WindowTitleAlign:    return "Window Title Align";
            case ImGuiStyleVar_ChildRounding:       return "Child Rounding";
            case ImGuiStyleVar_ChildBorderSize:     return "Child Border Size";
            case ImGuiStyleVar_PopupRounding:       return "Popup Rounding";
            case ImGuiStyleVar_PopupBorderSize:     return "Popup Border Size";
            case ImGuiStyleVar_FramePadding:        return "Frame Padding";
            case ImGuiStyleVar_FrameRounding:       return "Frame Rounding";
            case ImGuiStyleVar_FrameBorderSize:     return "Frame Border Size";
            case ImGuiStyleVar_ItemSpacing:         return "Item Spacing";
            case ImGuiStyleVar_ItemInnerSpacing:    return "Item Inner Spacing";
            case ImGuiStyleVar_IndentSpacing:       return "Indent Spacing";
            case ImGuiStyleVar_ScrollbarSize:       return "Scrollbar Size";
            case ImGuiStyleVar_ScrollbarRounding:   return "Scrollbar Rounding";
            case ImGuiStyleVar_GrabMinSize:         return "Grab Min Size";
            case ImGuiStyleVar_GrabRounding:        return "Grab Rounding";
            case ImGuiStyleVar_TabRounding:         return "Tab Rounding";
            case ImGuiStyleVar_ButtonTextAlign:     return "Button Text Align";
            case ImGuiStyleVar_SelectableTextAlign: return "Selectable Text Align";
        
        default:
            return "N/A";
        }
    }
    
}