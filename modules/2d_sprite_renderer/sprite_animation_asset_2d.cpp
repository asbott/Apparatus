#include "sprite_animation_asset_2d.h"

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture_sheet_2d)

size_t tell_size(str_ptr_t path, void* parameter) {
    (void)path; (void) parameter;
    return sizeof(Texture_Sheet_Runtime_Data);
}
byte* load(byte* stream, str_ptr_t path, void* parameter) {
    Params& params = *(Params*)parameter;

    (void)params;
    (void)path;

    Texture_Sheet_Runtime_Data data;

    data.preview_render_target = get_graphics()->make_render_target({1, 1});

    return stream + sizeof(Texture_Sheet_Runtime_Data);
}
void unload(byte* stream) {
    (void)stream;
}

void on_gui(void* parameter) {
    Params& params = *(Params*)parameter;
    if (params.cell_size.x <= 0.f) params.cell_size.x = 1.f;
    if (params.cell_size.y <= 0.f) params.cell_size.y = 1.f;

    ImGui::InputAsset("Source texture", &params.texture, "Texture");
    
    auto* asset_module = get_module("asset_manager");
    auto* asset_functions = (Asset_Manager_Function_Library*)asset_module->get_function_library();
    if (asset_module && asset_functions && asset_functions->validate(&params.texture)) {
        ImGui::RDragFloat2("Cell size", params.cell_size.ptr, 0.1f, 1.f, 100000000.f);

        if (Asset* asset = asset_functions->begin_use(params.texture)) {
            
            Texture_Data* tex = (Texture_Data*)asset->get_runtime_data();

            uvec2 ncells = (tex->size / params.cell_size);

            ImGui::RDragInt2("Empties", &params.nempty, 0, ncells.x);

            auto& tex_data = *(Texture_Data*)asset->get_runtime_data();

            f32 width = ImGui::GetWindowContentRegionWidth();
            /*f32 height = 

            data.ortho = mz::projection::ortho<f32>(-width / 2.f, width / 2.f, );*/

            asset_functions->end_use(params.texture);
        }
    }
}

void set_default_params(void* parameter) {
    *(Params*)parameter = Params();
}



AP_NS_END(asset)
AP_NS_END(texture_sheet_2d)