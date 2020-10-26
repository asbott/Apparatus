#include "sprite_animation_asset_2d.h"


#include "texture_asset.h"

namespace {
    Gizmo_Render_Context gizmo_list;
    graphics_id_t render_target = G_NULL_ID;
    mz::fmat4 ortho;
    mz::fmat4 view = 1;
}

AP_NS_BEGIN(asset)
AP_NS_BEGIN(texture_sheet_2d)

size_t tell_size(str_ptr_t path, void* parameter) {
    (void)path; (void) parameter;
    return sizeof(Texture_Sheet_Runtime_Data);
}
byte* load(byte* stream, str_ptr_t path, void* parameter) {
    Params& params = *(Params*)parameter;
    log_debug("Loading texture sheet");
    (void)params;
    (void)path;

    Texture_Sheet_Runtime_Data* data = new(stream) Texture_Sheet_Runtime_Data;

    data->texture = params.texture;
    data->cell_size = params.cell_size;

    return stream + sizeof(Texture_Sheet_Runtime_Data);
}
void unload(byte* stream) {
    Texture_Sheet_Runtime_Data* data = (Texture_Sheet_Runtime_Data*)stream;

    (void)stream;
    (void)data;
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
            
            Texture_Data* tex = asset->get_runtime_data<Texture_Data>();

            uvec2 ncells = (tex->size / params.cell_size);
            
            f32 width = ImGui::GetWindowContentRegionWidth();
            f32 height = (width / tex->size.width) * tex->size.height;

            if (render_target == G_NULL_ID) {
                render_target = get_graphics()->make_render_target(tex->size);
            }

            if (get_graphics()->get_render_target_size(render_target) != mz::ivec2(width, height)) {
                get_graphics()->set_render_target_size(render_target, { width, height });
            }
            ortho = mz::projection::ortho<f32>(0, tex->size.width, 0, tex->size.height, -1000, 1000);

            get_graphics()->set_clear_color(mz::COLOR_TRANSPARENT);
            get_graphics()->set_viewport(mz::viewport(0, 0, width, height));
            get_graphics()->clear(G_COLOR_BUFFER_BIT, render_target);

            for (u32 nx = 0; nx < ncells.width; nx++) {
                for (u32 ny = 0; ny < ncells.height; ny++) {
                    f32 x = (f32)nx * params.cell_size.width;
                    f32 y = (f32)ny * params.cell_size.height;

                    mz::fquad quad = mz::fquad(
                        fvec2 { x, y },
                        fvec2 { x, y + params.cell_size.height },
                        fvec2 { x + params.cell_size.width, y + params.cell_size.height },
                        fvec2 { x + params.cell_size.width, y }
                    );
                    gizmo_list.add_wire_quad(quad);
                }
            }

            mz::fmat4 cam_transform = view;
            cam_transform.invert();
            cam_transform = ortho * cam_transform;
            gizmo_list.flush(cam_transform, render_target, 1);

            auto gizmo_texture = get_graphics()->get_render_target_texture(render_target);

            ImGui::Spacing();
            ImGui::Separator();

            auto img_pos = ImGui::GetCursorPos();
            ImGui::Image(get_graphics()->get_native_texture_handle(tex->graphics_id), { width, height });

            ImGui::SetCursorPos(img_pos);
            ImGui::Image(get_graphics()->get_native_texture_handle(gizmo_texture), { width, height });

            if (ImGui::IsItemHovered()) {
                f32 xoffset = ImGui::GetMousePos().x - ImGui::GetWindowPos().x + ImGui::GetScrollX() - img_pos.x;
                f32 yoffset = ImGui::GetMousePos().y - ImGui::GetWindowPos().y + ImGui::GetScrollY() - img_pos.y;

                u32 xindex = (u32)((xoffset / width)  * ncells.width);
				u32 yindex = (u32)((yoffset / height) * ncells.height);

                ImGui::BeginTooltip();
                ImGui::Text("{ x: %u, y: %u } (index: %u)", xindex, yindex, (yindex) * ncells.x + xindex);
                ImGui::EndTooltip();
            }

            ImGui::Spacing();

            asset_functions->end_use(params.texture);
        }
    }
}

void set_default_params(void* parameter) {
    *(Params*)parameter = Params();
}



AP_NS_END(asset)
AP_NS_END(texture_sheet_2d)