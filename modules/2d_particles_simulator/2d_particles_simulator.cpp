#include "2d_particles_simulator.h"

#include "2d_sprite_renderer/2d_sprite_renderer.h"

#include "2d_viewport/2d_viewport.h"
#include "2d_editor/2d_editor.h"

#include <random>

constexpr char primitive_vert_shader[] = R"(
#version 450 core
layout (location = 0) in vec2 pos;
layout (location = 1) in vec4 color;

layout (std140) uniform UniformBuffer {
	mat4 cam_transform;
};

out vec4 vs_color;

void main() {
    vs_color = color;

    gl_Position = transpose(cam_transform) * vec4(pos.x, pos.y, 0.0, 1.0);
}

)";

constexpr char primitive_frag_shader[] = R"(
#version 450 core

out vec4 output_color;

in vec4 vs_color;

void main()
{
    output_color = vs_color;
}

)";

constexpr char sprite_vert_shader[] = R"(
#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 color;
layout (location = 3) in float texture_index;

layout (std140) uniform UniformBuffer {
	mat4 cam_transform;
};

out vec2 vs_uv;
out vec4 vs_color;
out flat int vs_texture_index;

void main() {
    vs_uv = uv;
    vs_color = color;
    vs_texture_index = int(texture_index);

    gl_Position = transpose(cam_transform) * vec4(pos.x, pos.y, pos.z, 1.0);
}

)";

constexpr char sprite_frag_shader[] = R"(
#version 450 core

out vec4 output_color;

in vec2 vs_uv;
in vec4 vs_color;
in flat int vs_texture_index;

uniform sampler2D samplers[32];

void main()
{
    //vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
    //output_color = texture_color * vs_color;
	if (vs_texture_index == -1)
		output_color = vs_color;
	else {
		vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
		output_color = vs_color * texture_color;
	}
}

)";

struct Basic_Vertex {
    fvec2 pos;
    fcolor16 color;
};

struct Sprite_Vertex {
    fvec2 pos;
    fvec2 uv;
	color16 color;
	float texture_index;
};

typedef Render_Context<Basic_Vertex, sizeof(Basic_Vertex) * 200000> Points_Context;

Points_Context point_render_context;
Render_Context<Basic_Vertex, sizeof(Basic_Vertex) * 100000 * 3>   tri_render_context;
Render_Context<Basic_Vertex, sizeof(Basic_Vertex) * 100000 * 4>   quad_render_context;
Render_Context<Sprite_Vertex, sizeof(Sprite_Vertex) * 100000 * 4> sprite_render_context;

Module* g_editor_module = NULL;

std::mt19937_64 rng;

f32 rand_ratio() {
    f32 ret = (f32)(rng() % 10000) / 10000.f;
    return ret;
}

f32 rand_in_range(f32 min, f32 max) {
    f32 ret = (rand_ratio() * (max - min)) + min;
    return ret;
}

f32 rand_in_range(fvec2 minmax) {
    return (rand_ratio() * (minmax.max - minmax.min)) + minmax.min;
}

void render_points(Graphics_Context* graphics, Points_Context& context, u64 nparticles, const fmat4& view, const fmat4& ortho, graphics_id_t render_target, ParticleSimulation2D& sim, Transform2D& transform) {

    context.data_ptr = context.data;
	u32 vertex_count = 0;
	
	auto cam_transform = view;
	cam_transform.invert();
	cam_transform = ortho * cam_transform;
	graphics->set_uniform_buffer_data(context.ubo, "cam_transform", cam_transform.ptr);

    
    rng.seed((u32)(uintptr_t)&sim);
    u64 start_particle = 0;
    if (sim.state.time_passed > sim.life_time) {
        start_particle = (u64)((sim.state.time_passed - sim.life_time) * sim.spawn_rate);
        rng.discard(start_particle * 2);
    }
    sim.state.num_active_particles = nparticles - start_particle;
    for (u64 i = start_particle; i < nparticles; i++) {
        fvec2 dir = fvec2::from_angle(rand_in_range(sim.angle_range));
        f32 speed = rand_in_range(sim.speed_range);

        fvec2 dist_per_sec = dir * speed;

        f32 max_particles = sim.spawn_rate * sim.duration;
        f32 birth_time = (i / (f32)max_particles) * sim.duration;
        f32 time_lived = sim.state.time_passed - birth_time;

        fvec2 sim_pos_offset = (dist_per_sec * time_lived);

        context.data_ptr->pos = transform.to_mat4().translate(sim_pos_offset).get_translation();
        context.data_ptr->color = mz::COLOR_WHITE;
        context.data_ptr++;

        vertex_count++;

        if (vertex_count * sizeof(Basic_Vertex) >= context.BUDGET) {
            get_thread_server().wait_for_thread(get_graphics_thread());
            graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Basic_Vertex));
            auto render_target_size = graphics->get_render_target_size(render_target);
            graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
            graphics->draw_indices(context.vao, context.shader, vertex_count, context.ubo, G_DRAW_MODE_POINTS, render_target);

            context.data_ptr = context.data;
            vertex_count = 0;
        }    
    }

    if (vertex_count > 0) {
		get_thread_server().wait_for_thread(get_graphics_thread());
		graphics->set_vertex_buffer_data(context.vbo, context.data, 0, vertex_count * sizeof(Basic_Vertex));
        auto render_target_size = graphics->get_render_target_size(render_target);
        graphics->set_viewport(mz::viewport(0, 0, render_target_size.width, render_target_size.height));
		graphics->draw_indices(context.vao, context.shader, vertex_count, context.ubo, G_DRAW_MODE_POINTS, render_target);
	}
}

extern "C" {
	_export void __cdecl on_load() {
        g_editor_module = get_module("2d_editor");

        Graphics_Context* graphics = get_graphics();
        Buffer_Layout_Specification primitive_shader_layout = {
            { "pos",   G_DATA_TYPE_FVEC2 },
            { "color", G_DATA_TYPE_FVEC4 }
        };
        Buffer_Layout_Specification sprite_shader_layout = {
            { "pos",   G_DATA_TYPE_FVEC2 },
            { "uv",   G_DATA_TYPE_FVEC2 },
            { "color", G_DATA_TYPE_FVEC4 },
            { "texture_index",   G_DATA_TYPE_F32 }
        };

        Buffer_Layout_Specification ubo_layout = {
            { "cam_transform", G_DATA_TYPE_F32MAT4 }  
        };

		point_render_context.set_shader(graphics, primitive_vert_shader, primitive_frag_shader, primitive_shader_layout);
        tri_render_context.set_shader(graphics, primitive_vert_shader, primitive_frag_shader, primitive_shader_layout);
        quad_render_context.set_shader(graphics, primitive_vert_shader, primitive_frag_shader, primitive_shader_layout);
        sprite_render_context.set_shader(graphics, sprite_vert_shader, sprite_frag_shader, sprite_shader_layout);

        Dynamic_Array<u32> indices;
        indices.resize(point_render_context.BUDGET / sizeof(Basic_Vertex));
        for (int i = 0; i < indices.size(); i++) indices[i] = i;
        point_render_context.set_buffers(graphics, primitive_shader_layout, indices, ubo_layout);

        tri_render_context.set_buffers(graphics, primitive_shader_layout, indices, ubo_layout);


        indices.resize((quad_render_context.BUDGET / (sizeof(Basic_Vertex) * 4)) * 6);

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
        quad_render_context.set_buffers(graphics, primitive_shader_layout, indices, ubo_layout);

        sprite_render_context.set_buffers(graphics, sprite_shader_layout, indices, ubo_layout);

	}

    _export void __cdecl on_unload() {
		
    }

	_export void __cdecl on_play_begin() {
        auto& reg = get_entity_registry();
		reg.view<ParticleSimulation2D, Transform2D>().each([](ParticleSimulation2D& sim, Transform2D&) {
            sim.state.reset();
            sim.state.is_playing = sim.play_on_start;
        });
	}

	_export void __cdecl on_play_end() {
		
	}

    _export void __cdecl on_update(float delta) {
		(void)delta;
    }

	_export void __cdecl on_render() {
        Graphics_Context* graphics = get_graphics();
        auto windows = graphics->get_windows_context();
        auto wnd = windows->main_window_handle;
        f32 delta = (f32)windows->window_info[wnd].delta_time;

        auto& reg = get_entity_registry();
        

        // First need to update each sim once each (not per view)
        reg.view<ParticleSimulation2D, Transform2D>().each([delta, graphics, &reg](ParticleSimulation2D& sim, Transform2D& transform) {

            if (sim.state.is_playing) sim.state.time_passed += delta;
            if (sim.state.time_passed >= sim.duration) {
                if (sim.loop) {
                    sim.state.time_passed -= sim.duration;
                } else {
                    sim.state.is_playing = false;
                    sim.state.time_passed = 0.f;
                }
            }

            u64 nparticles = (u64)(sim.state.time_passed * sim.spawn_rate);

            if (g_editor_module && g_editor_module->is_loaded && sim.preview_in_editor) {
                auto functions = (Editor_2D_Function_Library*)g_editor_module->get_function_library();
                auto editor_view = functions->get_view();

                switch (sim.type) {
                    case PARTICLE_POINTS: render_points(graphics, point_render_context, nparticles, editor_view->transform, editor_view->ortho, editor_view->render_target, sim, transform); break;
                    default: break;
                }
            } 
        });

        reg.view<View2D, Transform2D>().each([&reg, delta, graphics](View2D& view, Transform2D& view_transform) {
            reg.view<ParticleSimulation2D, Transform2D>().each([delta, graphics, &reg, &view, &view_transform](ParticleSimulation2D& sim, Transform2D& transform) {
                if (is_playing() || sim.preview_in_editor) {
                    u64 nparticles = (u64)(sim.state.time_passed * sim.spawn_rate);
                    switch (sim.type) {
                        case PARTICLE_POINTS: render_points(graphics, point_render_context, nparticles, view_transform.to_mat4(), view.ortho, view.render_target, sim, transform); break;
                        default: break;
                    }
                }
            });
        });
	}


	_export void __cdecl on_gui() {
		
	}
}