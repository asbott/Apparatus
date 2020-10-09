#pragma once

#include <imgui.h>

struct ImGuiExtensionStyle {
    f32 right_align = .35f; // 0-1
    f32 right_align_padding = 0.03f; // 0-1
};

namespace ImGui {

    AP_API bool RInputText(const char* label, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    AP_API bool RInputTextMultiline(const char* label, char* buf, size_t buf_size, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    AP_API bool RInputTextWithHint(const char* label, const char* hint, char* buf, size_t buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    AP_API bool RInputFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    AP_API bool RInputFloat2(const char* label, float v[2], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    AP_API bool RInputFloat3(const char* label, float v[3], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    AP_API bool RInputFloat4(const char* label, float v[4], const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
    AP_API bool RInputInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
    AP_API bool RInputInt2(const char* label, int v[2], ImGuiInputTextFlags flags = 0);
    AP_API bool RInputInt3(const char* label, int v[3], ImGuiInputTextFlags flags = 0);
    AP_API bool RInputInt4(const char* label, int v[4], ImGuiInputTextFlags flags = 0);
    AP_API bool RInputDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

    AP_API bool RDragFloat(const char* label, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    AP_API bool RDragFloat2(const char* label, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    AP_API bool RDragFloat3(const char* label, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    AP_API bool RDragFloat4(const char* label, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", ImGuiSliderFlags flags = 0);
    AP_API bool RDragFloatRange2(const char* label, float* v_current_min, float* v_current_max, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", const char* format_max = NULL, ImGuiSliderFlags flags = 0);
    AP_API bool RDragInt(const char* label, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    AP_API bool RDragInt2(const char* label, int v[2], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    AP_API bool RDragInt3(const char* label, int v[3], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    AP_API bool RDragInt4(const char* label, int v[4], float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", ImGuiSliderFlags flags = 0);
    AP_API bool RDragIntRange2(const char* label, int* v_current_min, int* v_current_max, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d", const char* format_max = NULL, ImGuiSliderFlags flags = 0);

    AP_API bool RCheckbox(const char* label, bool* v);

    AP_API bool RBeginCombo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0);
    AP_API void REndCombo();

    AP_API bool RColorEdit3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    AP_API bool RColorEdit4(const char* label, float col[4], ImGuiColorEditFlags flags = 0);
    AP_API bool RColorPicker3(const char* label, float col[3], ImGuiColorEditFlags flags = 0);
    AP_API bool RColorPicker4(const char* label, float col[4], ImGuiColorEditFlags flags = 0, const float* ref_col = NULL);

    AP_API ImGuiExtensionStyle& GetExtensionStyle();

    AP_API str_ptr_t GetStyleVarName(ImGuiStyleVar var);
}