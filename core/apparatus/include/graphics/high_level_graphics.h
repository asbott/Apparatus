#pragma once

#include "graphics/graphics_api.h"

constexpr char vert_shader_source[] = R"(
#version 450 core

layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 uv;

layout (std140) uniform UniformBuffer
{
    vec4 color;
    int texture_index;
    mat4 model;
    mat4 view;
    mat4 projection;
};

out vec2 vs_uv;
out vec4 vs_color;
out flat int vs_texture_index;

void main() {
    vs_uv = uv;
    vs_color = color;
    vs_texture_index = texture_index;

    mat4 transform = transpose(projection) * transpose(view) * transpose(model);
    gl_Position = transform * vec4(pos.x, pos.y, pos.z, 1.0);
}

)";

constexpr char frag_shader_source[] = R"(
#version 450 core

out vec4 output_color;

in vec2 vs_uv;
in vec4 vs_color;
in flat int vs_texture_index;

uniform sampler2D samplers[32];

void main()
{
    vec4 texture_color = texture(samplers[vs_texture_index], vs_uv);
    output_color = texture_color * vs_color;
}

)";


constexpr char dx11_vertex_shader_source[] = 
R"(
cbuffer CBuf {
    float4 color;
    int texture_index;
    matrix model;
    matrix view;
    matrix projection;
};
struct VSOut
{
    float4 color : Color;
    int texture_index : TextureIndex;
    float2 uv : UV;

    float4 pos : SV_POSITION;
};
VSOut main(float3 pos : Position, float2 uv : UV)
{
    VSOut vso;
    vso.color = color;
    vso.uv = uv;
    vso.texture_index = texture_index;

    float4 output_position = float4(pos.x, pos.y, pos.z, 1.0f);
    output_position = mul(output_position, model);
    output_position = mul(output_position, view);
    output_position = mul(output_position, projection);

    vso.pos = output_position;
    
    return vso;
})";


constexpr char dx11_pixel_shader_source[] = 
R"(
Texture2D textures[32] : register(t0);

SamplerState samp;

float4 main(float4 color : Color, int texture_index : TextureIndex, float2 uv : UV) : SV_TARGET
{
    float4 output_color = color;
    switch (texture_index) {
        case 0:  output_color *= textures[0].Sample(samp, uv);  break;
        case 1:  output_color *= textures[1].Sample(samp, uv);  output_color = float4(1, 1, 1, 1); break;
        case 2:  output_color *= textures[2].Sample(samp, uv);  break;
        case 3:  output_color *= textures[3].Sample(samp, uv);  break;
        case 4:  output_color *= textures[4].Sample(samp, uv);  break;
        case 5:  output_color *= textures[5].Sample(samp, uv);  break;
        case 6:  output_color *= textures[6].Sample(samp, uv);  break;
        case 7:  output_color *= textures[7].Sample(samp, uv);  break;
        case 8:  output_color *= textures[8].Sample(samp, uv);  break;
        case 9:  output_color *= textures[9].Sample(samp, uv);  break;
        case 10: output_color *= textures[10].Sample(samp, uv); break;
        case 11: output_color *= textures[11].Sample(samp, uv); break;
        case 12: output_color *= textures[12].Sample(samp, uv); break;
        case 13: output_color *= textures[13].Sample(samp, uv); break;
        case 14: output_color *= textures[14].Sample(samp, uv); break;
        case 15: output_color *= textures[15].Sample(samp, uv); break;
        case 16: output_color *= textures[16].Sample(samp, uv); break;
        case 17: output_color *= textures[17].Sample(samp, uv); break;
        case 18: output_color *= textures[18].Sample(samp, uv); break;
        case 19: output_color *= textures[19].Sample(samp, uv); break;
        case 20: output_color *= textures[20].Sample(samp, uv); break;
        case 21: output_color *= textures[21].Sample(samp, uv); break;
        case 22: output_color *= textures[22].Sample(samp, uv); break;
        case 23: output_color *= textures[23].Sample(samp, uv); break;
        case 24: output_color *= textures[24].Sample(samp, uv); break;
        case 25: output_color *= textures[25].Sample(samp, uv); break;
        case 26: output_color *= textures[26].Sample(samp, uv); break;
        case 27: output_color *= textures[27].Sample(samp, uv); break;
        case 28: output_color *= textures[28].Sample(samp, uv); break;
        case 29: output_color *= textures[29].Sample(samp, uv); break;
        case 30: output_color *= textures[30].Sample(samp, uv); break;
        case 31: output_color *= textures[31].Sample(samp, uv); break;
    }

    return output_color;
})";

constexpr char graphics_shader_source[] = R"(

layout {
    fvec3 pos;
    fvec2 uv;
}

uniform_buffer {
    color16 color;
    int texture_index;
    mat4 model;
    mat4 view;
    mat4 projection;
}

fvec4 vertex() {
    mat4 transform = transpose(projection) * transpose(view) * transpose(model);
    fvec4 output_position = transform * vec4(pos.x, pos.y, pos.z, 1.0);

    return output_position;
}

fcolor16 fragment() {
    fcolor16 texture_color = sample(texture_index, uv);

    return texture_color * color;
}

)";

struct High_Level_2D_Graphics {
	virtual ~High_Level_2D_Graphics() {}
	virtual void draw_quad(mz::frect quad, mz::color color) = 0;
	virtual void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color) = 0;
};

template <class Graphics_Type = Graphics_Context>
struct High_Level_2D_Graphics_Impl : public High_Level_2D_Graphics {
	
	struct ShaderData {
		alignas(16) mz::color16 color = mz::color16(0.f, 1.f, 1.f, 1.f);
		alignas(16) graphics_id_t texture;
		alignas(16) mz::fmat4 model;
		alignas(16) mz::fmat4 view;
		alignas(16) mz::fmat4 projection;
	};
	// Free all graphics resources
	
	inline High_Level_2D_Graphics_Impl(Graphics_Type* graphics) 
		: _graphics(graphics) {

        Buffer_Layout_Specification layout({
            { "Position", G_DATA_TYPE_FVEC3 },
            { "UV", G_DATA_TYPE_FVEC2 }
        });

		_quad_vao = graphics->make_vertex_array(layout);

        const char* vert = "";
        const char* frag = "";

        #ifdef AP_SUPPORT_DX11
        vert = dx11_vertex_shader_source;
        frag = dx11_pixel_shader_source;
        log_info("Running DirectX11!");
        #elif defined(AP_SUPPORT_OPENGL45)
        vert = vert_shader_source;
        frag = frag_shader_source;
        log_info("Running OpenGL45!");
        #endif

        graphics_id_t vert_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_VERTEX,   vert);
        graphics_id_t frag_shader = graphics->make_shader_source(G_SHADER_SOURCE_TYPE_FRAGMENT, frag);

        _shader = graphics->make_shader(vert_shader, frag_shader, layout);

		struct Vertex {
			fvec3 position;
			fvec2 uv;
		};

		Vertex vertices[] = {
			{ fvec3(0.0f, 1.0f, 0.0f), fvec2(0, 1) }, // top left
			{ fvec3(1.0f, 1.0f, 0.0f), fvec2(1, 1) }, // top right
			{ fvec3(1.0f, 0.0f, 0.0f), fvec2(1, 0) }, // bot right
			{ fvec3(0.0f, 0.0f, 0.0f), fvec2(0, 0) }, // bot left
		};

		_quad_vbo = graphics->make_vertex_buffer(vertices, sizeof(vertices), G_BUFFER_USAGE_STATIC_WRITE);

		u32 indices[] = {
			0, 1, 2,
			2, 3, 0
		};

		_quad_ibo = graphics->make_index_buffer(indices, sizeof(indices) / sizeof(u32), G_BUFFER_USAGE_STATIC_WRITE);
	
		graphics->associate_vertex_buffer(_quad_vbo, _quad_vao);
		graphics->associate_index_buffer (_quad_ibo, _quad_vao);

		graphics->set_clear_color(mz::color16(.1f, .3f, .8f, 1.f));
		graphics->set_viewport(mz::viewport(0, 0, 1280, 720));

		ortho = mz::projection::ortho<float>(0, 1280, 0, 720, -1, 1);
		view = mz::transformation::translation<float>(mz::fvec3(0, 0, 0));
		view.invert();

		Buffer_Layout_Specification shader_data_layout = {
			{ "color", G_DATA_TYPE_FVEC4 },
			{ "texture", G_DATA_TYPE_TEXTURE },
			{ "model", G_DATA_TYPE_F32MAT4 },
			{ "view", G_DATA_TYPE_F32MAT4 },
			{ "projection", G_DATA_TYPE_F32MAT4 }
		};
		ShaderData dummy;
		_quad_ubo = graphics->make_uniform_buffer(&dummy, shader_data_layout, G_BUFFER_USAGE_DYNAMIC_WRITE);
	}

	~High_Level_2D_Graphics_Impl() {
		_graphics->destroy_shader(_shader);
		_graphics->destroy_index_buffer(_quad_ibo);
		_graphics->destroy_vertex_array(_quad_vao);
		_graphics->destroy_vertex_buffer(_quad_vbo);
		_graphics->destroy_uniform_buffer(_quad_ubo);
	}

	inline void draw_quad(mz::frect quad, mz::color color) override {

		mz::fmat4 transform = mz::transformation::translation(mz::fvec3(quad.x, quad.y, 0));
		transform.scale(mz::fvec3(quad.z, quad.w, 0));

		// Set shader data
		ShaderData* shader_data = (ShaderData*)_graphics->map_uniform_buffer_data(_quad_ubo);
		shader_data->color = color;
		shader_data->texture = G_TEXTURE_SLOT_0; // set texture to slot/index 0
		shader_data->model = transform;
		shader_data->view = view;
		shader_data->projection = ortho;
		_graphics->unmap_uniform_buffer_data(_quad_ubo);

		// Draw the vao with the shader and shader data (ubo)
		_graphics->draw_indices(_quad_vao, _shader, _quad_ubo, G_DRAW_MODE_TRIANGLES);
	}
	inline void draw_line(mz::fvec2 a, mz::fvec2 b, mz::color color) override {
		(void)a;(void)b;(void)color;
	}

	Graphics_Type* _graphics;
	graphics_id_t _shader;

	graphics_id_t _quad_vao, _quad_vbo, _quad_ibo, _quad_ubo;
	mz::fmat4 ortho, view;
};