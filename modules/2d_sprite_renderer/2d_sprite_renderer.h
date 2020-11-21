#pragma once

#include "apparatus.h"

#include "asset_manager/asset_manager.h"

#include "texture_asset.h"
#include "sprite_animation_asset_2d.h"
#include "sprite_animation_2d_preset_asset.h"

tag(component)
struct Sprite2D {

	tag(property, asset("Texture"))
	asset_id_t texture = G_NULL_ID;

	tag(property, color)
	color tint = COLOR_WHITE;

	tag(property)
	fvec2 origin = 0;

	tag(property)
	int depth_level = 0;
};

tag(component, custom_gui)
struct SpriteAnimation2D {
	bool is_playing = true;
	int depth_level = 0;
	fvec2 origin = 0;
	color16 tint = COLOR_WHITE;

	asset_id_t animation_preset = NULL_ASSET_ID;

	asset_id_t texture_sheet;
	f32 frames_per_second = 14;
	irange target_frames;

	bool xflip = false;

	s32 current_frame = 0;
	f32 time = 0.f;

	s32 preview_frame = 0;
	f32 preview_time = 0;

	bool get_sheet_data(Texture_Sheet_Runtime_Data* out);
};

inline bool SpriteAnimation2D::get_sheet_data(Texture_Sheet_Runtime_Data* out) {
	auto* asset_manager = get_module("asset_manager");
	Asset_Manager_Function_Library* functions = NULL;
	Asset* sheet_asset = NULL;
	if (asset_manager) {
		functions = (Asset_Manager_Function_Library*)asset_manager->get_function_library();

		if (functions->validate(&animation_preset)) {
			if (Asset* asset = functions->begin_use(animation_preset)) {
				if (asset->is("SpriteAnimation2DPreset")) {
					auto* preset_data = asset->get_runtime_data<Sprite_Animation_2D_Preset_Data>();
					if (functions->validate(&preset_data->texture_sheet)) {
						sheet_asset = functions->begin_use(preset_data->texture_sheet);
						preset_data = asset->get_runtime_data<Sprite_Animation_2D_Preset_Data>();
						frames_per_second = preset_data->frames_per_second;
						target_frames = preset_data->target_frames;
					}
					functions->end_use(animation_preset);
				}
			}
		} else if (functions->validate(&texture_sheet)) {
			sheet_asset = functions->begin_use(texture_sheet);
		}
	}

	if (sheet_asset && sheet_asset->is("TextureSheet2D")) {
		*out = *sheet_asset->get_runtime_data<Texture_Sheet_Runtime_Data>();
		functions->end_use(sheet_asset->id);
		return true;
	}
	return false;
}

inline void on_gui(SpriteAnimation2D* anim) {
	auto* asset_manager = get_module("asset_manager");
	Asset_Manager_Function_Library* functions = NULL;

	ImGui::InputAsset("Preset", &anim->animation_preset, "SpriteAnimation2DPreset");

	bool use_preset = false;

	if (asset_manager) {
		functions = (Asset_Manager_Function_Library*)asset_manager->get_function_library();
		use_preset = functions->validate(&anim->animation_preset);
	}

	ImGui::RCheckbox("Play", &anim->is_playing);
	ImGui::RDragInt("Depth level", &anim->depth_level);
	ImGui::RColorEdit4("Tint", anim->tint.ptr);
	ImGui::RDragFvec2("Origin", &anim->origin, 0.1f);
	ImGui::RCheckbox("Flip X", &anim->xflip);
	ImGui::RSliderInt("Current frame", &anim->current_frame, 0, anim->target_frames.max - anim->target_frames.min);

	if (!use_preset) {
		ImGui::Spacing();
		ImGui::InputAsset("Texture sheet", &anim->texture_sheet, "TextureSheet2D");
	}

	Texture_Sheet_Runtime_Data sheet_data;
	if (asset_manager && anim->get_sheet_data(&sheet_data)) {

		f32 frames_per_second = anim->frames_per_second;
		irange target_frames = anim->target_frames;

		Asset* texture_asset = NULL;
		Texture_Data* tex_data = NULL;
		if (!functions->validate(&sheet_data.texture)) {
			return;		
		}

		if (!(texture_asset = functions->begin_use(sheet_data.texture))) {
			return;
		}

		if (!texture_asset->is("Texture")) {
			functions->end_use(sheet_data.texture);
			return;
		}

		tex_data = texture_asset->get_runtime_data<Texture_Data>();

		

		if (!use_preset) {
			ImGui::RDragFloat("Frames per second", &anim->frames_per_second, 0.05f);
		}

		uvec2 ncells = (tex_data->size / sheet_data.cell_size);

		if (!use_preset) {
			ImGui::RDragIntRange2("Target frames", &anim->target_frames, .1f, 0, ncells.x * ncells.y);
		}

		if (ImGui::TreeNode("Preview##1")) {
			u32 xindex = (target_frames.min + anim->preview_frame) % ncells.width;
			u32 yindex = (target_frames.min + anim->preview_frame) / ncells.width;
			fvec2 offset(xindex * sheet_data.cell_size.width, yindex * sheet_data.cell_size.height);


			f32 left = offset.x / tex_data->size.width;
			f32 right = (offset.x + sheet_data.cell_size.width) / tex_data->size.width;
			f32 bottom = offset.y / tex_data->size.height;
			f32 top = (offset.y + sheet_data.cell_size.height) / tex_data->size.height;

			ImGui::Image(get_graphics()->get_native_texture_handle(tex_data->graphics_id), { 128, 128 }, { anim->xflip ? right : left, bottom }, { anim->xflip ? left : right, top }, anim->tint, mz::COLOR_WHITE);

			ImGui::Text("Frame: %i/%i (%i/%i)", 
					anim->preview_frame, target_frames.max - target_frames.min,
					target_frames.min + anim->preview_frame, target_frames.max);

			ImGui::TreePop();
		}

		functions->end_use(sheet_data.texture);
		auto windows = get_graphics()->get_windows_context();
		auto wnd = windows->main_window_handle;
		f32 delta = (f32)windows->window_info[wnd].delta_time;
		if (frames_per_second <= 0) {
			anim->preview_time = 0;
		}

		anim->preview_time += delta;

		if (anim->preview_time >= 1.f / frames_per_second) {
			anim->preview_time -= 1.f / frames_per_second;
			anim->preview_frame++;
			if (anim->preview_frame > target_frames.max - target_frames.min) anim->preview_frame = 0;
		}
		
	}
}