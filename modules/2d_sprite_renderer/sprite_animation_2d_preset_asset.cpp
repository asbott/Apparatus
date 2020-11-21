#include "sprite_animation_2d_preset_asset.h"
#include "sprite_animation_asset_2d.h"

#include "texture_asset.h"

Gizmo_Render_Context gizmo_list;
graphics_id_t render_target = G_NULL_ID;
mz::fmat4 ortho;
mz::fmat4 view = 1;

AP_NS_BEGIN(asset)
AP_NS_BEGIN(sprite_animation_2d_preset)

size_t tell_size(str_ptr_t path, void* parameter) {
    (void)path; (void) parameter;
    return sizeof(Sprite_Animation_2D_Preset_Data);
}
byte* load(byte* stream, str_ptr_t path, void* parameter) {
    Params& params = *(Params*)parameter;
    (void)params;
    (void)path;

    Sprite_Animation_2D_Preset_Data* data = new(stream) Sprite_Animation_2D_Preset_Data;

    data->texture_sheet = params.texture_sheet;
    data->frames_per_second = params.frames_per_second;
    data->target_frames = params.target_frames;

    return stream + sizeof(Sprite_Animation_2D_Preset_Data);
}
void unload(byte* stream) {
    Sprite_Animation_2D_Preset_Data* data = (Sprite_Animation_2D_Preset_Data*)stream;

    (void)stream;
    (void)data;
}

void on_gui(void* parameter) {
    Params& params = *(Params*)parameter;

    ImGui::InputAsset("Texture sheet", &params.texture_sheet, "TextureSheet2D");

    Texture_Sheet_Runtime_Data* sheet_data = NULL;

    auto* asset_module = get_module("asset_manager");
    Asset_Manager_Function_Library* functions = NULL;
    if (asset_module) {
        functions = (Asset_Manager_Function_Library*)asset_module->get_function_library();

        if (functions->validate(&params.texture_sheet)) {
            Asset* asset = functions->begin_use(params.texture_sheet);

            if (asset) {
                sheet_data = asset->get_runtime_data<Texture_Sheet_Runtime_Data>();
            }
        }
    }

    if (!sheet_data) {
        return;
    }

    if (sheet_data->cell_size.x <= 0.f) sheet_data->cell_size.x = 1.f;
    if (sheet_data->cell_size.y <= 0.f) sheet_data->cell_size.y = 1.f;

    ImGui::InputAsset("Source texture", &sheet_data->texture, "Texture");
    
    if (functions->validate(&sheet_data->texture)) {

        if (Asset* asset = functions->begin_use(sheet_data->texture)) {

            
            Texture_Data* tex = asset->get_runtime_data<Texture_Data>();

            uvec2 ncells = (tex->size / sheet_data->cell_size);

            ImGui::RDragFloat("Frames per second", &params.frames_per_second, .05f, 0, 10000);
            ImGui::RDragIntRange2("Target frames", &params.target_frames, .05f, 0, ncells.x * ncells.y);
            
            if (ImGui::TreeNode("Preview##2")) {
                u32 xindex = (params.target_frames.min + params.preview_frame) % ncells.width;
                u32 yindex = (params.target_frames.min + params.preview_frame) / ncells.width;
                fvec2 offset(xindex * sheet_data->cell_size.width, yindex * sheet_data->cell_size.height);


                f32 left = offset.x / tex->size.width;
                f32 right = (offset.x + sheet_data->cell_size.width) / tex->size.width;
                f32 bottom = offset.y / tex->size.height;
                f32 top = (offset.y + sheet_data->cell_size.height) / tex->size.height;

                ImGui::Image(get_graphics()->get_native_texture_handle(tex->graphics_id), { 128, 128 }, { left, bottom }, { right, top }, mz::COLOR_WHITE);

                ImGui::Text("Frame: %i/%i (%i/%i)", 
                        params.preview_frame, params.target_frames.max - params.target_frames.min,
                        params.target_frames.min + params.preview_frame, params.target_frames.max);

                ImGui::TreePop();
            }

            ImGui::Spacing();
            ImGui::Separator();

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

            auto img_pos = ImGui::GetCursorPos();

            f32 xoffset = ImGui::GetMousePos().x - ImGui::GetWindowPos().x + ImGui::GetScrollX() - img_pos.x;
            f32 yoffset = ImGui::GetMousePos().y - ImGui::GetWindowPos().y + ImGui::GetScrollY() - img_pos.y;

            u32 xindex = (u32)((xoffset / width)  * ncells.width);
            u32 yindex = (u32)((yoffset / height) * ncells.height);
            u32 mouse_index = yindex * ncells.x + xindex;

            // TODO: #Refactor

            for (u32 nx = 0; nx < ncells.width; nx++) {
                for (u32 ny = 0; ny < ncells.height; ny++) {
                    f32 x = (f32)nx * sheet_data->cell_size.width;
                    f32 y = (f32)ny * sheet_data->cell_size.height;

                    mz::fquad quad = mz::fquad(
                        fvec2 { x, y },
                        fvec2 { x, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y }
                    );

                    gizmo_list.add_wire_quad(quad);
                }
            }

            for (u32 nx = 0; nx < ncells.width; nx++) {
                for (u32 ny = 0; ny < ncells.height; ny++) {
                    f32 x = (f32)nx * sheet_data->cell_size.width;
                    f32 y = (f32)ny * sheet_data->cell_size.height;

                    mz::fquad quad = mz::fquad(
                        fvec2 { x, y },
                        fvec2 { x, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y }
                    );
                    u32 index = ny * ncells.x + nx;

                    if (index >= params.target_frames.min && index <= params.target_frames.max) {
                        gizmo_list.add_wire_quad(quad, mz::COLOR_RED);
                    }
                }
            }

            for (u32 nx = 0; nx < ncells.width; nx++) {
                for (u32 ny = 0; ny < ncells.height; ny++) {
                    f32 x = (f32)nx * sheet_data->cell_size.width;
                    f32 y = (f32)ny * sheet_data->cell_size.height;

                    mz::fquad quad = mz::fquad(
                        fvec2 { x, y },
                        fvec2 { x, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y + sheet_data->cell_size.height },
                        fvec2 { x + sheet_data->cell_size.width, y }
                    );
                    u32 index = ny * ncells.x + nx;

                    if (index == mouse_index || index == params.target_frames.min + params.preview_frame) {
                        gizmo_list.add_wire_quad(quad, mz::COLOR_YELLOW);
                    }
                }
            }

            mz::fmat4 cam_transform = view;
            cam_transform.invert();
            cam_transform = ortho * cam_transform;
            gizmo_list.flush(cam_transform, render_target, 1);

            auto gizmo_texture = get_graphics()->get_render_target_texture(render_target);

            ImGui::Image(get_graphics()->get_native_texture_handle(tex->graphics_id), { width, height });

            ImGui::SetCursorPos(img_pos);
            ImGui::Image(get_graphics()->get_native_texture_handle(gizmo_texture), { width, height });

            if (ImGui::IsItemHovered()) {
                

                ImGui::BeginTooltip();
                ImGui::Text("{ x: %u, y: %u } (index: %u)", xindex, yindex, mouse_index);
                ImGui::EndTooltip();
            }

            ImGui::Spacing();

            

            functions->end_use(sheet_data->texture);

            auto windows = get_graphics()->get_windows_context();
            auto wnd = windows->main_window_handle;
            f32 delta = (f32)windows->window_info[wnd].delta_time;

            if (params.frames_per_second <= 0) {
                params.preview_time = 0;
            }

            params.preview_time += delta;

            if (params.preview_time >= 1.f / params.frames_per_second) {
                params.preview_time -= 1.f / params.frames_per_second;
                params.preview_frame++;
                if (params.preview_frame > params.target_frames.max - params.target_frames.min) params.preview_frame = 0;
            }
        }
    }

    if (sheet_data) {
        functions->end_use(params.texture_sheet);
    }
}

void set_default_params(void* parameter) {
    *(Params*)parameter = Params();
}



AP_NS_END(asset)
AP_NS_END(sprite_animation_2d_preset)