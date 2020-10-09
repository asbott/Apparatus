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
        str_t<128> id = "";
        sprintf(id, "##%llu", (uintptr_t)buf);
        bool ret = InputText(id, buf, buf_size, flags, callback, user_data);
        return ret;
    }
    bool RInputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        str_t<128> id = "";
        sprintf(id, "##%llu", (uintptr_t)buf);
        bool ret = InputTextMultiline(id, buf, buf_size, size, flags, callback, user_data);
        return ret;
    }
    bool RInputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data) {
        set_layout(label);
        str_t<128> id = "";
        sprintf(id, "##%llu", (uintptr_t)buf);
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

    void SaveStyleToDisk(str_ptr_t path) {
        std::ofstream ostr;
        ostr.open(path);
        if (!ostr.good()) return;

        byte* style_data = (byte*)&ImGui::GetStyle();

        for (int i = 0; i < sizeof(ImGuiStyle); i++) {
            ostr << (int)style_data[i] << " ";
        }

        ostr.close();
    }
    void LoadStyleFromDisk(str_ptr_t path) {
        if (!Path::can_open(path)) return;
        std::ifstream istr;
        istr.open(path);

        byte* style_data = (byte*)&ImGui::GetStyle();

        Dynamic_Array<byte> bytes;

        while (!istr.eof()) {
            str_t<4> byte_str = "";

            istr >> byte_str;

            bool is_num = true;
            for (int i = 0; i < strlen(byte_str); i++) if (!isdigit(byte_str[i]) ) is_num = false;

            if (!is_num) continue;

            bytes.push_back((byte)atoi(byte_str));
        }

        memcpy(style_data, bytes.data(), bytes.size());

        istr.close();
    }
    
}