#include "2d_tilemap_renderer.h"

#include "2d_viewport/2d_viewport.h"
#include "asset_manager/asset_manager.h"
#include "2d_sprite_renderer/2d_sprite_renderer.h"

constexpr char vert_shader_source[] = R"(
#version 450 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 uv;

layout (std140) uniform UniformBuffer {
	mat4 cam_transform;
};

out vec2 vs_uv;

void main() {
    vs_uv = uv;

    gl_Position = transpose(cam_transform) * vec4(pos.x, pos.y, 0.0, 1.0);
}

)";

constexpr char frag_shader_source[] = R"(
#version 450 core

out vec4 output_color;

in vec2 vs_uv;

uniform sampler2D samplers[32];

void main()
{
    output_color = texture(samplers[0], vs_uv);
}

)";

struct Vertex {
    fvec2 pos;
    fvec2 uv;
};

constexpr size_t MAX_BUDGET = 1024 * 1000 * 20;
constexpr size_t QUAD_SIZE = sizeof(Vertex) * 4;
constexpr size_t TRI_SIZE = sizeof(Vertex) * 3;
constexpr size_t MAX_VERTICES = MAX_BUDGET / sizeof(Vertex);
constexpr size_t MAX_QUADS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t MAX_TRIS = MAX_BUDGET / QUAD_SIZE;
constexpr size_t INDEX_COUNT = MAX_QUADS * 6;

constexpr size_t BUDGET = MAX_VERTICES * sizeof(Vertex);

Render_Context<Vertex, BUDGET> g_render_context;
Gizmo_Render_Context gizmo_context;

Gui_Window g_tilemap_editor = { false, "Tilemap editor" };

Ordered_Set<graphics_id_t> g_render_targets_to_destroy;

entt::entity g_selected_tilemap_entity = entt::null;

fmat4 g_ortho;
fmat4 g_view = 1;

Module* g_asset_module = NULL;
Asset_Manager_Function_Library* g_asset_functions = NULL;


bool is_valid_tilemap_entity(entt::registry& reg, entt::entity entity) {
    return reg.valid(entity) && reg.has<TileMap2D>(entity);
}

module_scope {
	module_function(void) on_load() {
		
        auto graphics = get_graphics();

        register_gui_window(&g_tilemap_editor);

        Buffer_Layout_Specification ubo_layout = {
			{ "cam_transform", G_DATA_TYPE_F32MAT4 },
		};

		Buffer_Layout_Specification layout({
			{ "Position", G_DATA_TYPE_FVEC2 },
			{ "UV", G_DATA_TYPE_FVEC2 }
		});

        Dynamic_Array<u32> indices;
		indices.resize(INDEX_COUNT);

		u32 offset = 0U;
		for (u32 i = 0U; i < indices.size(); i += 6U) {
			indices[i] = offset + 0U;
			indices[i + 1U] = offset + 1U;
			indices[i + 2U] = offset + 2U;
			indices[i + 3U] = offset + 2U;
			indices[i + 4U] = offset + 3U;
			indices[i + 5U] = offset + 0U;

			offset += 4;
		}

        g_render_context.set_shader(graphics, vert_shader_source, frag_shader_source, layout);
        g_render_context.set_buffers(graphics, layout, indices, ubo_layout);

        g_asset_module = get_module("asset_manager");
        g_asset_functions = (Asset_Manager_Function_Library*)g_asset_module->get_function_library();
	}

    module_function(void) on_unload() {
        Graphics_Context* graphics = get_graphics();
		for (graphics_id_t render_target : g_render_targets_to_destroy) {
			graphics->destroy_render_target(render_target);
		}
		g_render_targets_to_destroy.clear();

		unregister_gui_window(&g_tilemap_editor);
    }

	module_function(void) on_play_begin() {
		Graphics_Context* graphics = get_graphics();
		for (graphics_id_t render_target : g_render_targets_to_destroy) {
			graphics->destroy_render_target(render_target);
		}
		g_render_targets_to_destroy.clear();
	}

	module_function(void) on_play_end() {
		
	}

    module_function(void) save_to_disk(str_ptr_t dir) {
		
	}

	module_function(void) load_from_disk(str_ptr_t dir) {
		
	}

    module_function(void) on_update(float delta) {
		(void)delta;

		
    }

	module_function(void) on_render() {
		
        auto& reg = get_entity_registry();

        for (auto entity : get_selected_entities()) {
            if (reg.has<TileMap2D>(entity)) {
                g_selected_tilemap_entity = entity;
                break;
            }
        }

        auto graphics = get_graphics();

        reg.view<Transform2D, TileMap2D>().each([graphics](entt::entity entity, Transform2D& transform, TileMap2D& tilemap){
            if (tilemap.render_target == G_NULL_ID) {
                tilemap.render_target = graphics->make_render_target({1, 1});
                g_render_targets_to_destroy.emplace(tilemap.render_target);
            }

            if (g_selected_tilemap_entity == entity && g_asset_functions->validate(&tilemap.source_texture)) {
                graphics->set_clear_color(mz::COLOR_GREEN);
                graphics->clear(G_COLOR_BUFFER_BIT, tilemap.render_target);

                fmat4 view_inverted = g_view.invert();
                fmat4 cam_transform = g_ortho * view_inverted;
                graphics->set_uniform_buffer_data(g_render_context.ubo, "cam_transform", cam_transform.ptr);

                graphics->set_viewport(viewport(ivec2(0, 0), graphics->get_render_target_size(tilemap.render_target)));

                
            }
        });



	}

	module_function(void) on_gui() {
		
        auto& reg = get_entity_registry();
        auto graphics = get_graphics();

        ImGui::DoGuiWindow(&g_tilemap_editor, [&reg, graphics]() {
            if (!is_valid_tilemap_entity(reg, g_selected_tilemap_entity)) {
                ImGui::Text("No entity with TileMap2D selected");
                return;
            }

            auto& tilemap = reg.get<TileMap2D>(g_selected_tilemap_entity);

            if (tilemap.render_target == G_NULL_ID) return;

            fvec2 content_size = (fvec2)ImGui::GetWindowContentRegionMax() - (fvec2)ImGui::GetWindowContentRegionMin();
            fvec2 content_pos = (fvec2)ImGui::GetWindowContentRegionMin();
            frect content_region = frect(content_pos, content_size);

            ImGui::SetCursorPos({content_region.pos.x, content_region.pos.y });
            fvec2 frame_size = { content_region.width * .3f, content_region.height };

            auto col = ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
            col.x *= .8f; col.y *= .8f; col.z *= .8f; col.w *= .8f;
            ImGui::PushStyleColor(ImGuiCol_WindowBg, col);
            ImGui::BeginChild(ImGui::GetID(&tilemap), frame_size, true, ImGuiWindowFlags_HorizontalScrollbar);
            ImGui::Text("Look, some tools ! sssssssssssssssssss");
            ImGui::EndChild();
            ImGui::PopStyleColor();

            

            
            ImGui::SetCursorPos({content_region.pos.x + content_region.width * .3f, content_region.pos.y });
            frame_size = { content_region.width * .7f, content_region.height };
            ImGui::PushStyleColor(ImGuiCol_WindowBg, col);
            ImGui::BeginChild(ImGui::GetID(&g_selected_tilemap_entity), frame_size);
            if (frame_size.x > 0 && frame_size.y > 0) {
                

                if (graphics->get_render_target_size(tilemap.render_target) != (ivec2)frame_size) {
                    log_trace("{}, {}", graphics->get_render_target_size(tilemap.render_target), (ivec2)frame_size);
                    graphics->set_render_target_size(tilemap.render_target, frame_size);
                    g_ortho = mz::projection::ortho<f32>(-frame_size.x / 2.f, frame_size.x / 2.f, -frame_size.y / 2.f, frame_size.y / 2.f, -1000, 1000);
                }

                ImGui::SetCursorPos({0, 0});
                auto texture = graphics->get_render_target_texture(tilemap.render_target);
                ImGui::Image(graphics->get_native_texture_handle(texture), frame_size, { 0, 1 }, { 1, 0 });
            }

            ImGui::EndChild();
            ImGui::PopStyleColor();
        }, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);


	}
}