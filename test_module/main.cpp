
//#include <stdio.h>
//
//#include <Windows.h>
//
//#include <imgui.h>
//#include <apparatus.h>
//
//#define _export __declspec(dllexport)
//
//constexpr char vert_shader_source[] = R"(
//#version 450 core
//
//layout (location = 0) in vec3 pos;
//layout (location = 1) in vec2 uv;
//
//layout (std140) uniform UniformBuffer
//{
//    vec4 color;
//    int texture_index;
//    mat4 model;
//    mat4 view;
//    mat4 projection;
//};
//
//out vec2 vs_uv;
//out vec4 vs_color;
//out flat int vs_texture_index;
//
//void main() {
//    vs_uv = uv;
//    vs_color = color;
//    vs_texture_index = texture_index;
//
//    mat4 transform = transpose(projection) * transpose(view) * transpose(model);
//    gl_Position = transform * vec4(pos.x, pos.y, pos.z, 1.0);
//}
//
//)";
//
//constexpr char frag_shader_source[] = R"(
//#version 450 core
//
//out vec4 output_color;
//
//in vec2 vs_uv;
//in vec4 vs_color;
//in flat int vs_texture_index;
//
//uniform sampler2D samplers[32];
//
//void main()
//{
//    vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
//    output_color = texture_color * vs_color;
//}
//
//)";
//
//
//constexpr char dx11_vertex_shader_source[] = 
//R"(
//cbuffer CBuf {
//    float4 color;
//    int texture_index;
//    matrix model;
//    matrix view;
//    matrix projection;
//};
//struct VSOut
//{
//    float4 color : Color;
//    int texture_index : TextureIndex;
//    float2 uv : UV;
//
//    float4 pos : SV_POSITION;
//};
//VSOut main(float3 pos : Position, float2 uv : UV)
//{
//    VSOut vso;
//    vso.color = color;
//    vso.uv = uv;
//    vso.texture_index = texture_index;
//
//    float4 output_position = float4(pos.x, pos.y, pos.z, 1.0f);
//    output_position = mul(output_position, model);
//    output_position = mul(output_position, view);
//    output_position = mul(output_position, projection);
//
//    vso.pos = output_position;
//    
//    return vso;
//})";
//
//
//constexpr char dx11_pixel_shader_source[] = 
//R"(
//Texture2D textures[32] : register(t0);
//
//SamplerState samp;
//
//float4 main(float4 color : Color, int texture_index : TextureIndex, float2 uv : UV) : SV_TARGET
//{
//    float4 output_color = color;
//    switch (texture_index) {
//        case 0:  output_color *= textures[0].Sample(samp, uv);  break;
//        case 1:  output_color *= textures[1].Sample(samp, uv);  output_color = float4(1, 1, 1, 1); break;
//        case 2:  output_color *= textures[2].Sample(samp, uv);  break;
//        case 3:  output_color *= textures[3].Sample(samp, uv);  break;
//        case 4:  output_color *= textures[4].Sample(samp, uv);  break;
//        case 5:  output_color *= textures[5].Sample(samp, uv);  break;
//        case 6:  output_color *= textures[6].Sample(samp, uv);  break;
//        case 7:  output_color *= textures[7].Sample(samp, uv);  break;
//        case 8:  output_color *= textures[8].Sample(samp, uv);  break;
//        case 9:  output_color *= textures[9].Sample(samp, uv);  break;
//        case 10: output_color *= textures[10].Sample(samp, uv); break;
//        case 11: output_color *= textures[11].Sample(samp, uv); break;
//        case 12: output_color *= textures[12].Sample(samp, uv); break;
//        case 13: output_color *= textures[13].Sample(samp, uv); break;
//        case 14: output_color *= textures[14].Sample(samp, uv); break;
//        case 15: output_color *= textures[15].Sample(samp, uv); break;
//        case 16: output_color *= textures[16].Sample(samp, uv); break;
//        case 17: output_color *= textures[17].Sample(samp, uv); break;
//        case 18: output_color *= textures[18].Sample(samp, uv); break;
//        case 19: output_color *= textures[19].Sample(samp, uv); break;
//        case 20: output_color *= textures[20].Sample(samp, uv); break;
//        case 21: output_color *= textures[21].Sample(samp, uv); break;
//        case 22: output_color *= textures[22].Sample(samp, uv); break;
//        case 23: output_color *= textures[23].Sample(samp, uv); break;
//        case 24: output_color *= textures[24].Sample(samp, uv); break;
//        case 25: output_color *= textures[25].Sample(samp, uv); break;
//        case 26: output_color *= textures[26].Sample(samp, uv); break;
//        case 27: output_color *= textures[27].Sample(samp, uv); break;
//        case 28: output_color *= textures[28].Sample(samp, uv); break;
//        case 29: output_color *= textures[29].Sample(samp, uv); break;
//        case 30: output_color *= textures[30].Sample(samp, uv); break;
//        case 31: output_color *= textures[31].Sample(samp, uv); break;
//    }
//
//    return output_color;
//})";
//
//constexpr char graphics_shader_source[] = R"(
//
//layout {
//    fvec3 pos;
//    fvec2 uv;
//}
//
//uniform_buffer {
//    color16 color;
//    int texture_index;
//    mat4 model;
//    mat4 view;
//    mat4 projection;
//}
//
//fvec4 vertex() {
//    mat4 transform = transpose(projection) * transpose(view) * transpose(model);
//    fvec4 output_position = transform * vec4(pos.x, pos.y, pos.z, 1.0);
//
//    return output_position;
//}
//
//fcolor16 fragment() {
//    fcolor16 texture_color = sample(texture_index, uv);
//
//    return texture_color * color;
//}
//
//)";
//
//Windows_Context* g_windows;
//void* g_wnd;
//
//graphics_id_t g_shader;
//graphics_id_t g_ibo;
//graphics_id_t g_vao;
//graphics_id_t g_vbo;
//graphics_id_t g_ubo;
//graphics_id_t g_texture;
//
//mz::color16 g_color;
//mz::fmat4 g_model, g_view, g_projection, g_ortho;
//
//thread_id_t g_submit_thread;
//
//entt::registry g_reg;
//
//struct ShaderData {
//    alignas(16) mz::color16 g_color = mz::color16(0.f, 1.f, 1.f, 1.f);
//    alignas(16) graphics_id_t g_texture;
//    alignas(16) mz::fmat4 g_model;
//    alignas(16) mz::fmat4 g_view;
//    alignas(16) mz::fmat4 g_projection;
//};
//
//struct Position2D {
//    mz::fvec2 value;
//};
//struct Scale2D {
//    mz::fvec2 value = mz::fvec2(1);
//};
//struct Rotation2D {
//    f32 value;
//};
//struct Rotate {
//    f32 speed = 1.f;
//};
//
//graphics_id_t load_shader(Graphics_Context* graphics, const char* vert_src, const char* frag_src, const Buffer_Layout_Specification& input_layout) {
//    graphics_id_t vert_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_VERTEX,   vert_src);
//    graphics_id_t frag_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_FRAGMENT, frag_src);
//
//    graphics_id_t shader = graphics->make_shader(vert_shader, frag_shader, input_layout);
//
//    graphics->destroy_shader_source(vert_shader);
//    graphics->destroy_shader_source(frag_shader);
//        
//    return shader;
//}
//
//extern "C" {
//	_export void __cdecl on_load(Graphics_Context* graphics) {
//		log_info("on_load called in test_module!");
//
//        g_submit_thread = get_thread_server().make_thread();
//
//        const char* vert = "";
//        const char* frag = "";
//
//        #ifdef AP_SUPPORT_DX11
//        vert = dx11_vertex_shader_source;
//        frag = dx11_pixel_shader_source;
//        log_info("Running DirectX11!");
//        #elif defined(AP_SUPPORT_OPENGL45)
//        vert = vert_shader_source;
//        frag = frag_shader_source;
//        log_info("Running OpenGL45!");
//        #endif
//        
//
//        graphics->set_culling(G_CULL_FRONT);
//        graphics->set_blending(true);
//
//        Buffer_Layout_Specification layout({
//            { "Position", G_DATA_TYPE_FVEC3 },
//            { "UV", G_DATA_TYPE_FVEC2 }
//            });
//
//        g_shader = load_shader(graphics, vert, frag, layout);
//
//        g_vao = graphics->make_vertex_array(layout);
//
//        struct Vertex {
//            mz::fvec3 position;
//            mz::fvec2 uv;
//        };
//
//        f32 sz = 50.f;
//        Vertex vertices[] = {
//            { mz::fvec3(-sz,  sz, 0.0f), mz::fvec2(0, 1) }, // top left
//            { mz::fvec3( sz,  sz, 0.0f), mz::fvec2(1, 1) }, // top right
//            { mz::fvec3( sz, -sz, 0.0f), mz::fvec2(1, 0) }, // bot right
//            { mz::fvec3(-sz, -sz, 0.0f), mz::fvec2(0, 0) }, // bot left
//        };
//
//        g_vbo = graphics->make_vertex_buffer(vertices, sizeof(vertices), G_BUFFER_USAGE_STATIC_WRITE);
//
//        u32 indices[] = {
//            0, 1, 2,
//            2, 3, 0
//        };
//
//        g_ibo = graphics->make_index_buffer(indices, sizeof(indices) / sizeof(u32), G_BUFFER_USAGE_STATIC_WRITE);
//
//        graphics->associate_vertex_buffer(g_vbo, g_vao);
//        graphics->associate_index_buffer (g_ibo, g_vao);
//
//        graphics->set_clear_color(mz::color16(.1f, .3f, .8f, 1.f));
//        graphics->set_viewport(mz::viewport(0, 0, 1280, 720));
//
//        path_str_t asset_dir = "";
//        sprintf(asset_dir, "%s/../../assets", get_executable_directory());
//
//        path_str_t cat_dir = "";
//        sprintf(cat_dir, "%s/%s", asset_dir, "cat.png");
//
//        set_vertical_flip_on_load(true);
//        s32 width = 0, height = 0, channels = 0;
//        byte* image_data = load_image_from_file(cat_dir, &width, &height, &channels, 4);
//
//        ap_assert(image_data != NULL, "Failed loading image. Reason: {}", get_failure_reason());
//
//        g_texture = graphics->make_texture(G_BUFFER_USAGE_STATIC_WRITE);
//
//        graphics->set_texture_wrapping(g_texture, G_WRAP_CLAMP_TO_EDGE);
//        graphics->set_texture_filtering(g_texture, G_MIN_FILTER_LINEAR, G_MAG_FILTER_LINEAR);
//        graphics->set_texture_data(g_texture, image_data, mz::ivec2(width, height), G_TEXTURE_FORMAT_RGBA);
//
//        
//
//        g_ortho = mz::projection::ortho<float>(0, 1280, 0, 720, -1, 1);
//        g_view = mz::transformation::translation<float>(mz::fvec3(0, 0, 0));
//        g_view.invert();
//        g_model = mz::transformation::translation(mz::fvec3(600, 330, 0));
//        g_color = mz::color(0, 1, 1, 1);
//
//        Buffer_Layout_Specification shader_data_layout = {
//            { "color", G_DATA_TYPE_FVEC4 },
//            { "texture", G_DATA_TYPE_TEXTURE },
//            { "model", G_DATA_TYPE_F32MAT4 },
//            { "view", G_DATA_TYPE_F32MAT4 },
//            { "projection", G_DATA_TYPE_F32MAT4 }
//        };
//        ShaderData dummy;
//        g_ubo = graphics->make_uniform_buffer(&dummy, shader_data_layout, G_BUFFER_USAGE_DYNAMIC_WRITE);
//
//        g_windows = graphics->get_windows_context();
//        g_wnd = g_windows->main_window_handle;
//
//        free_image(image_data);
//
//        auto ent = g_reg.create();
//        g_reg.emplace<Position2D>(ent).value = mz::fvec2(600, 350);
//        g_reg.emplace<Scale2D>(ent).value = mz::fvec2(2.f, 1.f);
//        g_reg.emplace<Rotation2D>(ent).value = 0.f;
//        g_reg.emplace<Rotate>(ent);
//
//        ent = g_reg.create();
//        g_reg.emplace<Position2D>(ent).value = mz::fvec2(100, 100);
//	}
//
//    _export void __cdecl on_unload(Graphics_Context* graphics) {
//
//        get_thread_server().wait_for_thread(g_submit_thread);
//        get_thread_server().kill_thread(g_submit_thread);
//
//        // Free all graphics resources
//        graphics->destroy_shader(g_shader);
//        graphics->destroy_index_buffer(g_ibo);
//        graphics->destroy_vertex_array(g_vao);
//        graphics->destroy_vertex_buffer(g_vbo);
//        graphics->destroy_uniform_buffer(g_ubo);
//        graphics->destroy_texture(g_texture);
//    }
//
//    _export void __cdecl on_update(float delta) {
//        if (g_windows->should_close(g_wnd)) quit();
//        // Slowly rotate
//        g_model.rotate(delta, mz::fvec3(0, 0, 1));
//
//        // Fade into red then reset
//        g_color.r += delta;
//        if (g_color.r > 1.f) g_color.r = 0.f;
//
//        g_reg.view<Rotate, Rotation2D>().each([delta](Rotate& rot, Rotation2D& rotation) {
//            rotation.value += rot.speed * delta;
//        });
//    }
//
//	_export void __cdecl on_render(Graphics_Context* graphics) {
//        // Bind texture to slot/index 0
//        graphics->bind_texture_to_slot(g_texture, G_TEXTURE_SLOT_0);
//
//        get_thread_server().queue_task(g_submit_thread, [graphics]() {
//            
//            g_reg.view<Position2D>().each([graphics](entt::entity entity, Position2D& pos) {
//                mz::fmat4 model = mz::transformation::translation<float>(pos.value);
//                if (g_reg.has<Rotation2D>(entity)) {
//                    model.rotate(g_reg.get<Rotation2D>(entity).value, mz::fvec3(0, 0, 1));
//                }
//                if (g_reg.has<Scale2D>(entity)) {
//                    model.scale(g_reg.get<Scale2D>(entity).value);
//                }
//                // Set shader data
//                ShaderData* shader_data = (ShaderData*)graphics->map_uniform_buffer_data(g_ubo);
//                shader_data->g_color = g_color;
//                shader_data->g_texture = G_TEXTURE_SLOT_0; // set texture to slot/index 0
//                shader_data->g_model = model;
//                shader_data->g_view = g_view;
//                shader_data->g_projection = g_ortho;
//                graphics->unmap_uniform_buffer_data(g_ubo); 
//
//                graphics->draw_indices(g_vao, g_shader, g_ubo, G_DRAW_MODE_TRIANGLES);
//            });
//        });
//
//        get_thread_server().wait_for_thread(g_submit_thread);
//	}
//
//	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
//		(void)graphics; (void)imgui_ctx;
//		ImGui::SetCurrentContext(imgui_ctx);
//		ImGui::ShowDemoWindow();
//		
//		ImGui::Begin("Hey hi");
//		
//		ImGui::End();
//	}
//}

#define _export __declspec(dllexport)

#include "apparatus.h"

#include "test.h"

#define something

extern "C" {
	_export void __cdecl on_load(Graphics_Context* graphics) {
		log_info("on_load called in test_module!");
		(void)graphics;
        
		graphics->set_clear_color(mz::color(.35f, .1f, .65f, 1.f));
	}

    _export void __cdecl on_unload(Graphics_Context* graphics) {
		(void)graphics;
        
    }

    _export void __cdecl on_update(float delta) {
		(void)delta;

		get_entity_registry().view<Transform>().each([](Transform& transform) {
			log_trace("Position: {}\n Scale: {}\nColor: {}", 
				       transform.position, transform.scale, transform.color);
		});
    }

	_export void __cdecl on_render(Graphics_Context* graphics) {
		graphics->clear(G_COLOR_BUFFER_BIT);

		auto windows = graphics->get_windows_context();
		auto wnd = windows->main_window_handle;

		if (windows->should_close(wnd)) {
			quit();
		}
	}

	_export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
		(void)graphics;(void)imgui_ctx;
	}
}