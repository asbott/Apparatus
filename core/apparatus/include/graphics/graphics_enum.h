#pragma once

typedef s16 graphics_enum_t;

// Bit flags
constexpr graphics_enum_t G_COLOR_BUFFER_BIT   = BIT6;
constexpr graphics_enum_t G_DEPTH_BUFFER_BIT   = BIT7;
constexpr graphics_enum_t G_STENCIL_BUFFER_BIT = BIT8;

// Culling modes
constexpr graphics_enum_t G_CULL_FRONT = 3000;
constexpr graphics_enum_t G_CULL_BACK  = 3001;
constexpr graphics_enum_t G_CULL_NONE  = 3002;

// Shader source types
constexpr graphics_enum_t G_SHADER_SOURCE_TYPE_VERTEX   = 5000;
constexpr graphics_enum_t G_SHADER_SOURCE_TYPE_FRAGMENT = 5001;

// Data types
constexpr graphics_enum_t G_DATA_TYPE_S8      = 6000;
constexpr graphics_enum_t G_DATA_TYPE_U8      = 6001;
constexpr graphics_enum_t G_DATA_TYPE_S16     = 6002;
constexpr graphics_enum_t G_DATA_TYPE_U16     = 6003;
constexpr graphics_enum_t G_DATA_TYPE_S32     = 6004;
constexpr graphics_enum_t G_DATA_TYPE_U32     = 6005;
constexpr graphics_enum_t G_DATA_TYPE_S64     = 6006;
constexpr graphics_enum_t G_DATA_TYPE_U64     = 6007;

constexpr graphics_enum_t G_DATA_TYPE_S8VEC2  = 6008;
constexpr graphics_enum_t G_DATA_TYPE_U8VEC2  = 6009;
constexpr graphics_enum_t G_DATA_TYPE_S8VEC3  = 6010;
constexpr graphics_enum_t G_DATA_TYPE_U8VEC3  = 6011;
constexpr graphics_enum_t G_DATA_TYPE_S8VEC4  = 6012;
constexpr graphics_enum_t G_DATA_TYPE_U8VEC4  = 6013;

constexpr graphics_enum_t G_DATA_TYPE_S16VEC2 = 6014;
constexpr graphics_enum_t G_DATA_TYPE_U16VEC2 = 6015;
constexpr graphics_enum_t G_DATA_TYPE_S16VEC3 = 6016;
constexpr graphics_enum_t G_DATA_TYPE_U16VEC3 = 6017;
constexpr graphics_enum_t G_DATA_TYPE_S16VEC4 = 6018;
constexpr graphics_enum_t G_DATA_TYPE_U16VEC4 = 6019;

constexpr graphics_enum_t G_DATA_TYPE_S32VEC2 = 6020;
constexpr graphics_enum_t G_DATA_TYPE_U32VEC2 = 6021;
constexpr graphics_enum_t G_DATA_TYPE_S32VEC3 = 6022;
constexpr graphics_enum_t G_DATA_TYPE_U32VEC3 = 6023;
constexpr graphics_enum_t G_DATA_TYPE_S32VEC4 = 6024;
constexpr graphics_enum_t G_DATA_TYPE_U32VEC4 = 6025;

constexpr graphics_enum_t G_DATA_TYPE_S64VEC2 = 6026;
constexpr graphics_enum_t G_DATA_TYPE_U64VEC2 = 6027;
constexpr graphics_enum_t G_DATA_TYPE_S64VEC3 = 6028;
constexpr graphics_enum_t G_DATA_TYPE_U64VEC3 = 6029;
constexpr graphics_enum_t G_DATA_TYPE_S64VEC4 = 6030;
constexpr graphics_enum_t G_DATA_TYPE_U64VEC4 = 6031;

constexpr graphics_enum_t G_DATA_TYPE_F32     = 6032;
constexpr graphics_enum_t G_DATA_TYPE_F64     = 6033;

constexpr graphics_enum_t G_DATA_TYPE_F32VEC2 = 6034;
constexpr graphics_enum_t G_DATA_TYPE_F64VEC2 = 6035;
constexpr graphics_enum_t G_DATA_TYPE_F32VEC3 = 6036;
constexpr graphics_enum_t G_DATA_TYPE_F64VEC3 = 6037;
constexpr graphics_enum_t G_DATA_TYPE_F32VEC4 = 6038;
constexpr graphics_enum_t G_DATA_TYPE_F64VEC4 = 6039;

constexpr graphics_enum_t G_DATA_TYPE_F32MAT4 = 6040;

constexpr graphics_enum_t G_DATA_TYPE_TEXTURE = 6041;

constexpr graphics_enum_t G_DATA_TYPE_FVEC2 = G_DATA_TYPE_F32VEC2;
constexpr graphics_enum_t G_DATA_TYPE_FVEC3 = G_DATA_TYPE_F32VEC3;
constexpr graphics_enum_t G_DATA_TYPE_FVEC4 = G_DATA_TYPE_F32VEC4;

constexpr graphics_enum_t G_DATA_TYPE_DVEC2 = G_DATA_TYPE_F64VEC2;
constexpr graphics_enum_t G_DATA_TYPE_DVEC3 = G_DATA_TYPE_F64VEC3;
constexpr graphics_enum_t G_DATA_TYPE_DVEC4 = G_DATA_TYPE_F64VEC4;

constexpr graphics_enum_t G_DATA_TYPE_SVEC2 = G_DATA_TYPE_S32VEC2;
constexpr graphics_enum_t G_DATA_TYPE_SVEC3 = G_DATA_TYPE_S32VEC3;
constexpr graphics_enum_t G_DATA_TYPE_SVEC4 = G_DATA_TYPE_S32VEC4;
constexpr graphics_enum_t G_DATA_TYPE_IVEC2 = G_DATA_TYPE_SVEC2;
constexpr graphics_enum_t G_DATA_TYPE_IVEC3 = G_DATA_TYPE_SVEC3;
constexpr graphics_enum_t G_DATA_TYPE_IVEC4 = G_DATA_TYPE_SVEC4;

constexpr graphics_enum_t G_DATA_TYPE_UVEC2 = G_DATA_TYPE_U32VEC2;
constexpr graphics_enum_t G_DATA_TYPE_UVEC3 = G_DATA_TYPE_U32VEC3;
constexpr graphics_enum_t G_DATA_TYPE_UVEC4 = G_DATA_TYPE_U32VEC4;

// Buffer usage
constexpr graphics_enum_t G_BUFFER_USAGE_STREAM_WRITE  = 7000;
constexpr graphics_enum_t G_BUFFER_USAGE_STREAM_READ   = 7001;
constexpr graphics_enum_t G_BUFFER_USAGE_STREAM_COPY   = 7002;
constexpr graphics_enum_t G_BUFFER_USAGE_STATIC_WRITE  = 7003;
constexpr graphics_enum_t G_BUFFER_USAGE_STATIC_READ   = 7004;
constexpr graphics_enum_t G_BUFFER_USAGE_STATIC_COPY   = 7005;
constexpr graphics_enum_t G_BUFFER_USAGE_DYNAMIC_WRITE = 7006;
constexpr graphics_enum_t G_BUFFER_USAGE_DYNAMIC_READ  = 7007;
constexpr graphics_enum_t G_BUFFER_USAGE_DYNAMIC_COPY  = 7008;

// Draw modes
constexpr graphics_enum_t G_DRAW_MODE_POINTS                   = 9000;
constexpr graphics_enum_t G_DRAW_MODE_LINE_STRIP               = 9001;
constexpr graphics_enum_t G_DRAW_MODE_LINE_LOOP                = 9002;
constexpr graphics_enum_t G_DRAW_MODE_LINES                    = 9003;
constexpr graphics_enum_t G_DRAW_MODE_LINE_STRIP_ADJACENCY     = 9004;
constexpr graphics_enum_t G_DRAW_MODE_LINES_ADJACENCY          = 9005;
constexpr graphics_enum_t G_DRAW_MODE_TRIANGLE_STRIP           = 9006;
constexpr graphics_enum_t G_DRAW_MODE_TRIANGLE_FAN             = 9007;
constexpr graphics_enum_t G_DRAW_MODE_TRIANGLES                = 9008;
constexpr graphics_enum_t G_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY = 9009;
constexpr graphics_enum_t G_DRAW_MODE_TRIANGLES_ADJACENCY      = 9010;
constexpr graphics_enum_t G_DRAW_MODE_PATCHES                  = 9011;

// Debug message severity
constexpr graphics_enum_t G_DEBUG_MESSAGE_SEVERITY_NOTIFY   = 10000;
constexpr graphics_enum_t G_DEBUG_MESSAGE_SEVERITY_WARNING  = 10001;
constexpr graphics_enum_t G_DEBUG_MESSAGE_SEVERITY_ERROR    = 10002;
constexpr graphics_enum_t G_DEBUG_MESSAGE_SEVERITY_CRITICAL = 10003;

// Texture wrapping
constexpr graphics_enum_t G_WRAP_REPEAT          = 11000;
constexpr graphics_enum_t G_WRAP_MIRRORED_REPEAT = 11001;
constexpr graphics_enum_t G_WRAP_CLAMP_TO_EDGE   = 11002;
constexpr graphics_enum_t G_WRAP_CLAMP_TO_BORDER = 11003;

// Texture filtering
constexpr graphics_enum_t G_MIN_FILTER_NEAREST                = 12000;
constexpr graphics_enum_t G_MIN_FILTER_LINEAR                 = 12001;
constexpr graphics_enum_t G_MIN_FILTER_NEAREST_MIPMAP_NEAREST = 12002;
constexpr graphics_enum_t G_MIN_FILTER_NEAREST_MIPMAP_LINEAR  = 12003;
constexpr graphics_enum_t G_MIN_FILTER_LINEAR_MIPMAP_NEAREST  = 12004;
constexpr graphics_enum_t G_MIN_FILTER_LINEAR_MIPMAP_LINEAR   = 12005;

constexpr graphics_enum_t G_MAG_FILTER_NEAREST                = 12006;
constexpr graphics_enum_t G_MAG_FILTER_LINEAR                 = 12007;

// Texture format
constexpr graphics_enum_t G_TEXTURE_FORMAT_RED          = 13000;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RG           = 13001;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RGB          = 13002;
constexpr graphics_enum_t G_TEXTURE_FORMAT_BGR          = 13003;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RGBA         = 13004;
constexpr graphics_enum_t G_TEXTURE_FORMAT_BGRA         = 13005;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RED_INTEGER  = 13006;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RG_INTEGER   = 13007;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RGB_INTEGER  = 13008;
constexpr graphics_enum_t G_TEXTURE_FORMAT_BGR_INTEGER  = 13009;
constexpr graphics_enum_t G_TEXTURE_FORMAT_RGBA_INTEGER = 13010;
constexpr graphics_enum_t G_TEXTURE_FORMAT_BGRA_INTEGER = 13011;

// Texture slots
constexpr graphics_enum_t G_TEXTURE_SLOT_0              = 0;
constexpr graphics_enum_t G_TEXTURE_SLOT_1              = 1;
constexpr graphics_enum_t G_TEXTURE_SLOT_2              = 2;
constexpr graphics_enum_t G_TEXTURE_SLOT_3              = 3;
constexpr graphics_enum_t G_TEXTURE_SLOT_4              = 4;
constexpr graphics_enum_t G_TEXTURE_SLOT_5              = 5;
constexpr graphics_enum_t G_TEXTURE_SLOT_6              = 6;
constexpr graphics_enum_t G_TEXTURE_SLOT_7              = 7;
constexpr graphics_enum_t G_TEXTURE_SLOT_8              = 8;
constexpr graphics_enum_t G_TEXTURE_SLOT_9              = 9;
constexpr graphics_enum_t G_TEXTURE_SLOT_10             = 10;
constexpr graphics_enum_t G_TEXTURE_SLOT_11             = 11;
constexpr graphics_enum_t G_TEXTURE_SLOT_12             = 12;
constexpr graphics_enum_t G_TEXTURE_SLOT_13             = 13;
constexpr graphics_enum_t G_TEXTURE_SLOT_14             = 14;
constexpr graphics_enum_t G_TEXTURE_SLOT_15             = 15;
constexpr graphics_enum_t G_TEXTURE_SLOT_16             = 16;
constexpr graphics_enum_t G_TEXTURE_SLOT_17             = 17;
constexpr graphics_enum_t G_TEXTURE_SLOT_18             = 18;
constexpr graphics_enum_t G_TEXTURE_SLOT_19             = 19;
constexpr graphics_enum_t G_TEXTURE_SLOT_20             = 20;
constexpr graphics_enum_t G_TEXTURE_SLOT_21             = 21;
constexpr graphics_enum_t G_TEXTURE_SLOT_22             = 22;
constexpr graphics_enum_t G_TEXTURE_SLOT_23             = 23;
constexpr graphics_enum_t G_TEXTURE_SLOT_24             = 24;
constexpr graphics_enum_t G_TEXTURE_SLOT_25             = 25;
constexpr graphics_enum_t G_TEXTURE_SLOT_26             = 26;
constexpr graphics_enum_t G_TEXTURE_SLOT_27             = 27;
constexpr graphics_enum_t G_TEXTURE_SLOT_28             = 28;
constexpr graphics_enum_t G_TEXTURE_SLOT_29             = 29;
constexpr graphics_enum_t G_TEXTURE_SLOT_30             = 30;
constexpr graphics_enum_t G_TEXTURE_SLOT_31             = 31;



template <typename T>
inline graphics_enum_t compile_time_type_to_graphics_type() {

    if      constexpr (std::is_same<T, s8>())  return G_DATA_TYPE_S8;
    else if constexpr (std::is_same<T, u8>())  return G_DATA_TYPE_U8;
    else if constexpr (std::is_same<T, s16>()) return G_DATA_TYPE_S16;
    else if constexpr (std::is_same<T, u16>()) return G_DATA_TYPE_U16;
    else if constexpr (std::is_same<T, s32>()) return G_DATA_TYPE_S32;
    else if constexpr (std::is_same<T, u32>()) return G_DATA_TYPE_U32;
    else if constexpr (std::is_same<T, s64>()) return G_DATA_TYPE_S64;
    else if constexpr (std::is_same<T, u64>()) return G_DATA_TYPE_U64;

    else if constexpr (std::is_same<T, mz::s8vec2>())  return G_DATA_TYPE_S8VEC2;
    else if constexpr (std::is_same<T, mz::u8vec2>())  return G_DATA_TYPE_U8VEC2;
    else if constexpr (std::is_same<T, mz::s8vec3>())  return G_DATA_TYPE_S8VEC3;
    else if constexpr (std::is_same<T, mz::u8vec3>())  return G_DATA_TYPE_U8VEC3;
    else if constexpr (std::is_same<T, mz::s8vec4>())  return G_DATA_TYPE_S8VEC4;
    else if constexpr (std::is_same<T, mz::u8vec4>())  return G_DATA_TYPE_U8VEC4;

    else if constexpr (std::is_same<T, mz::s16vec2>()) return G_DATA_TYPE_S16VEC2;
    else if constexpr (std::is_same<T, mz::u16vec2>()) return G_DATA_TYPE_U16VEC2;
    else if constexpr (std::is_same<T, mz::s16vec3>()) return G_DATA_TYPE_S16VEC3;
    else if constexpr (std::is_same<T, mz::u16vec3>()) return G_DATA_TYPE_U16VEC3;
    else if constexpr (std::is_same<T, mz::s16vec4>()) return G_DATA_TYPE_S16VEC4;
    else if constexpr (std::is_same<T, mz::u16vec4>()) return G_DATA_TYPE_U16VEC4;

    else if constexpr (std::is_same<T, mz::s32vec2>()) return G_DATA_TYPE_S32VEC2;
    else if constexpr (std::is_same<T, mz::u32vec2>()) return G_DATA_TYPE_U32VEC2;
    else if constexpr (std::is_same<T, mz::s32vec3>()) return G_DATA_TYPE_S32VEC3;
    else if constexpr (std::is_same<T, mz::u32vec3>()) return G_DATA_TYPE_U32VEC3;
    else if constexpr (std::is_same<T, mz::s32vec4>()) return G_DATA_TYPE_S32VEC4;
    else if constexpr (std::is_same<T, mz::u32vec4>()) return G_DATA_TYPE_U32VEC4;

    else if constexpr (std::is_same<T, mz::s64vec2>()) return G_DATA_TYPE_S64VEC2;
    else if constexpr (std::is_same<T, mz::u64vec2>()) return G_DATA_TYPE_U64VEC2;
    else if constexpr (std::is_same<T, mz::s64vec3>()) return G_DATA_TYPE_S64VEC3;
    else if constexpr (std::is_same<T, mz::u64vec3>()) return G_DATA_TYPE_U64VEC3;
    else if constexpr (std::is_same<T, mz::s64vec4>()) return G_DATA_TYPE_S64VEC4;
    else if constexpr (std::is_same<T, mz::u64vec4>()) return G_DATA_TYPE_U64VEC4;

    else if constexpr (std::is_same<T, f32>()) return G_DATA_TYPE_F32;
    else if constexpr (std::is_same<T, f64>()) return G_DATA_TYPE_F64;

    else if constexpr (std::is_same<T, mz::f32vec2>()) return G_DATA_TYPE_F32VEC2;
    else if constexpr (std::is_same<T, mz::f64vec2>()) return G_DATA_TYPE_F64VEC2;
    else if constexpr (std::is_same<T, mz::f32vec3>()) return G_DATA_TYPE_F32VEC3;
    else if constexpr (std::is_same<T, mz::f64vec3>()) return G_DATA_TYPE_F64VEC3;
    else if constexpr (std::is_same<T, mz::f32vec4>()) return G_DATA_TYPE_F32VEC4;
    else if constexpr (std::is_same<T, mz::f64vec4>()) return G_DATA_TYPE_F64VEC4;

    else if constexpr (std::is_same<T, mz::fmat4>()) return G_DATA_TYPE_F32MAT4;

    else {
        assert(false && "Unhandled type to size");
    }
}

inline size_t get_graphics_data_type_size(graphics_enum_t data_type) {
    switch (data_type) {
        case G_DATA_TYPE_S8:      return sizeof(s8);  break;
        case G_DATA_TYPE_U8:      return sizeof(u8);  break;
        case G_DATA_TYPE_S16:     return sizeof(s16); break;
        case G_DATA_TYPE_U16:     return sizeof(u16); break;
        case G_DATA_TYPE_S32:     return sizeof(s32); break;
        case G_DATA_TYPE_U32:     return sizeof(u32); break;
        case G_DATA_TYPE_S64:     return sizeof(s64); break;
        case G_DATA_TYPE_U64:     return sizeof(u64); break;
                                  
        case G_DATA_TYPE_S8VEC2:  return sizeof(mz::s8vec2); break;
        case G_DATA_TYPE_U8VEC2:  return sizeof(mz::u8vec2); break;
        case G_DATA_TYPE_S8VEC3:  return sizeof(mz::s8vec3); break;
        case G_DATA_TYPE_U8VEC3:  return sizeof(mz::u8vec3); break;
        case G_DATA_TYPE_S8VEC4:  return sizeof(mz::s8vec4); break;
        case G_DATA_TYPE_U8VEC4:  return sizeof(mz::u8vec4); break;
                                  
        case G_DATA_TYPE_S16VEC2: return sizeof(mz::s16vec2); break;
        case G_DATA_TYPE_U16VEC2: return sizeof(mz::u16vec2); break;
        case G_DATA_TYPE_S16VEC3: return sizeof(mz::s16vec3); break;
        case G_DATA_TYPE_U16VEC3: return sizeof(mz::u16vec3); break;
        case G_DATA_TYPE_S16VEC4: return sizeof(mz::s16vec4); break;
        case G_DATA_TYPE_U16VEC4: return sizeof(mz::u16vec4); break;

        case G_DATA_TYPE_S32VEC2: return sizeof(mz::s32vec2); break;
        case G_DATA_TYPE_U32VEC2: return sizeof(mz::u32vec2); break;
        case G_DATA_TYPE_S32VEC3: return sizeof(mz::s32vec3); break;
        case G_DATA_TYPE_U32VEC3: return sizeof(mz::u32vec3); break;
        case G_DATA_TYPE_S32VEC4: return sizeof(mz::s32vec4); break;
        case G_DATA_TYPE_U32VEC4: return sizeof(mz::u32vec4); break;

        case G_DATA_TYPE_S64VEC2: return sizeof(mz::s64vec2); break;
        case G_DATA_TYPE_U64VEC2: return sizeof(mz::u64vec2); break;
        case G_DATA_TYPE_S64VEC3: return sizeof(mz::s64vec3); break;
        case G_DATA_TYPE_U64VEC3: return sizeof(mz::u64vec3); break;
        case G_DATA_TYPE_S64VEC4: return sizeof(mz::s64vec4); break;
        case G_DATA_TYPE_U64VEC4: return sizeof(mz::u64vec4); break;
                                         
        case G_DATA_TYPE_F32:     return sizeof(f32); break;
        case G_DATA_TYPE_F64:     return sizeof(f64); break;
                                         
        case G_DATA_TYPE_F32VEC2: return sizeof(mz::f32vec2); break;
        case G_DATA_TYPE_F64VEC2: return sizeof(mz::f64vec2); break;
        case G_DATA_TYPE_F32VEC3: return sizeof(mz::f32vec3); break;
        case G_DATA_TYPE_F64VEC3: return sizeof(mz::f64vec3); break;
        case G_DATA_TYPE_F32VEC4: return sizeof(mz::f32vec4); break;
        case G_DATA_TYPE_F64VEC4: return sizeof(mz::f64vec4); break;
                                         
        case G_DATA_TYPE_F32MAT4: return sizeof(mz::fmat4); break;

        case G_DATA_TYPE_TEXTURE: return sizeof(s32); break;

        default:
            _AP_BREAK;
            break;
    }

    return 0;
}
inline count_t get_graphics_data_type_component_count(graphics_enum_t data_type) {
    switch (data_type) {
        case G_DATA_TYPE_S8:
        case G_DATA_TYPE_U8:
        case G_DATA_TYPE_S16:
        case G_DATA_TYPE_U16:
        case G_DATA_TYPE_S32:
        case G_DATA_TYPE_U32:
        case G_DATA_TYPE_S64:
        case G_DATA_TYPE_U64:
        case G_DATA_TYPE_F32:
        case G_DATA_TYPE_F64:
            return 1; break;
                                  
        case G_DATA_TYPE_S8VEC2:
        case G_DATA_TYPE_U8VEC2:
        case G_DATA_TYPE_S16VEC2:
        case G_DATA_TYPE_U16VEC2:
        case G_DATA_TYPE_S32VEC2:
        case G_DATA_TYPE_U32VEC2:
        case G_DATA_TYPE_S64VEC2:
        case G_DATA_TYPE_U64VEC2:
        case G_DATA_TYPE_F32VEC2:
        case G_DATA_TYPE_F64VEC2:
            return 2;

        case G_DATA_TYPE_F32VEC3:
        case G_DATA_TYPE_F64VEC3:
        case G_DATA_TYPE_S64VEC3:
        case G_DATA_TYPE_U64VEC3:
        case G_DATA_TYPE_S32VEC3:
        case G_DATA_TYPE_U32VEC3:
        case G_DATA_TYPE_S8VEC3:
        case G_DATA_TYPE_U8VEC3:
        case G_DATA_TYPE_S16VEC3:
        case G_DATA_TYPE_U16VEC3:
            return 3;
                                  
        case G_DATA_TYPE_S8VEC4:
        case G_DATA_TYPE_U8VEC4:
        case G_DATA_TYPE_S16VEC4:
        case G_DATA_TYPE_U16VEC4:
        case G_DATA_TYPE_S32VEC4:
        case G_DATA_TYPE_U32VEC4:
        case G_DATA_TYPE_S64VEC4:
        case G_DATA_TYPE_U64VEC4:
        case G_DATA_TYPE_F32VEC4:
        case G_DATA_TYPE_F64VEC4:
            return 4;
                                         
        case G_DATA_TYPE_F32MAT4:
            return 16; break;

        case G_DATA_TYPE_TEXTURE:
            return 1; break;

        default:
            _AP_BREAK;
            break;
    }

    return 0;
}

inline const char* get_graphics_enum_string(graphics_enum_t value) {

    switch (value) {
        case G_COLOR_BUFFER_BIT:                   return "G_COLOR_BUFFER_BIT";
        case G_DEPTH_BUFFER_BIT:                   return "G_DEPTH_BUFFER_BIT";
        case G_STENCIL_BUFFER_BIT:                 return "G_STENCIL_BUFFER_BIT";
        case G_CULL_FRONT:                         return "G_CULL_FRONT";
        case G_CULL_BACK:                          return "G_CULL_BACK";
        case G_CULL_NONE:                          return "G_CULL_NONE";
        case G_SHADER_SOURCE_TYPE_VERTEX:          return "G_SHADER_SOURCE_TYPE_VERTEX";
        case G_SHADER_SOURCE_TYPE_FRAGMENT:        return "G_SHADER_SOURCE_TYPE_FRAGMENT";
        case G_DATA_TYPE_S8:                       return "G_DATA_TYPE_S8";
        case G_DATA_TYPE_U8:                       return "G_DATA_TYPE_U8";
        case G_DATA_TYPE_S16:                      return "G_DATA_TYPE_S16";
        case G_DATA_TYPE_U16:                      return "G_DATA_TYPE_U16";
        case G_DATA_TYPE_S32:                      return "G_DATA_TYPE_S32";
        case G_DATA_TYPE_U32:                      return "G_DATA_TYPE_U32";
        case G_DATA_TYPE_S64:                      return "G_DATA_TYPE_S64";
        case G_DATA_TYPE_U64:                      return "G_DATA_TYPE_U64";
        case G_DATA_TYPE_S8VEC2:                   return "G_DATA_TYPE_S8VEC2";
        case G_DATA_TYPE_U8VEC2:                   return "G_DATA_TYPE_U8VEC2";
        case G_DATA_TYPE_S8VEC3:                   return "G_DATA_TYPE_S8VEC3";
        case G_DATA_TYPE_U8VEC3:                   return "G_DATA_TYPE_U8VEC3";
        case G_DATA_TYPE_S8VEC4:                   return "G_DATA_TYPE_S8VEC4";
        case G_DATA_TYPE_U8VEC4:                   return "G_DATA_TYPE_U8VEC4";
        case G_DATA_TYPE_S16VEC2:                  return "G_DATA_TYPE_S16VEC2";
        case G_DATA_TYPE_U16VEC2:                  return "G_DATA_TYPE_U16VEC2";
        case G_DATA_TYPE_S16VEC3:                  return "G_DATA_TYPE_S16VEC3";
        case G_DATA_TYPE_U16VEC3:                  return "G_DATA_TYPE_U16VEC3";
        case G_DATA_TYPE_S16VEC4:                  return "G_DATA_TYPE_S16VEC4";
        case G_DATA_TYPE_U16VEC4:                  return "G_DATA_TYPE_U16VEC4";
        case G_DATA_TYPE_S32VEC2:                  return "G_DATA_TYPE_S32VEC2";
        case G_DATA_TYPE_U32VEC2:                  return "G_DATA_TYPE_U32VEC2";
        case G_DATA_TYPE_S32VEC3:                  return "G_DATA_TYPE_S32VEC3";
        case G_DATA_TYPE_U32VEC3:                  return "G_DATA_TYPE_U32VEC3";
        case G_DATA_TYPE_S32VEC4:                  return "G_DATA_TYPE_S32VEC4";
        case G_DATA_TYPE_U32VEC4:                  return "G_DATA_TYPE_U32VEC4";
        case G_DATA_TYPE_S64VEC2:                  return "G_DATA_TYPE_S64VEC2";
        case G_DATA_TYPE_U64VEC2:                  return "G_DATA_TYPE_U64VEC2";
        case G_DATA_TYPE_S64VEC3:                  return "G_DATA_TYPE_S64VEC3";
        case G_DATA_TYPE_U64VEC3:                  return "G_DATA_TYPE_U64VEC3";
        case G_DATA_TYPE_S64VEC4:                  return "G_DATA_TYPE_S64VEC4";
        case G_DATA_TYPE_U64VEC4:                  return "G_DATA_TYPE_U64VEC4";
        case G_DATA_TYPE_F32:                      return "G_DATA_TYPE_F32";
        case G_DATA_TYPE_F64:                      return "G_DATA_TYPE_F64";
        case G_DATA_TYPE_F32VEC2:                  return "G_DATA_TYPE_F32VEC2";
        case G_DATA_TYPE_F64VEC2:                  return "G_DATA_TYPE_F64VEC2";
        case G_DATA_TYPE_F32VEC3:                  return "G_DATA_TYPE_F32VEC3";
        case G_DATA_TYPE_F64VEC3:                  return "G_DATA_TYPE_F64VEC3";
        case G_DATA_TYPE_F32VEC4:                  return "G_DATA_TYPE_F32VEC4";
        case G_DATA_TYPE_F64VEC4:                  return "G_DATA_TYPE_F64VEC4";
        case G_DATA_TYPE_F32MAT4:                  return "G_DATA_TYPE_F32MAT4";
        case G_DATA_TYPE_TEXTURE:                  return "G_DATA_TYPE_TEXTURE";
        case G_BUFFER_USAGE_STREAM_WRITE:          return "G_BUFFER_USAGE_STREAM_WRITE";
        case G_BUFFER_USAGE_STREAM_READ:           return "G_BUFFER_USAGE_STREAM_READ";
        case G_BUFFER_USAGE_STREAM_COPY:           return "G_BUFFER_USAGE_STREAM_COPY";
        case G_BUFFER_USAGE_STATIC_WRITE:          return "G_BUFFER_USAGE_STATIC_WRITE";
        case G_BUFFER_USAGE_STATIC_READ:           return "G_BUFFER_USAGE_STATIC_READ";
        case G_BUFFER_USAGE_STATIC_COPY:           return "G_BUFFER_USAGE_STATIC_COPY";
        case G_BUFFER_USAGE_DYNAMIC_WRITE:         return "G_BUFFER_USAGE_DYNAMIC_WRITE";
        case G_BUFFER_USAGE_DYNAMIC_READ:          return "G_BUFFER_USAGE_DYNAMIC_READ";
        case G_BUFFER_USAGE_DYNAMIC_COPY:          return "G_BUFFER_USAGE_DYNAMIC_COPY";
        case G_DRAW_MODE_POINTS:                   return "G_DRAW_MODE_POINTS";
        case G_DRAW_MODE_LINE_STRIP:               return "G_DRAW_MODE_LINE_STRIP";
        case G_DRAW_MODE_LINE_LOOP:                return "G_DRAW_MODE_LINE_LOOP";
        case G_DRAW_MODE_LINES:                    return "G_DRAW_MODE_LINES";
        case G_DRAW_MODE_LINE_STRIP_ADJACENCY:     return "G_DRAW_MODE_LINE_STRIP_ADJACENCY";
        case G_DRAW_MODE_LINES_ADJACENCY:          return "G_DRAW_MODE_LINES_ADJACENCY";
        case G_DRAW_MODE_TRIANGLE_STRIP:           return "G_DRAW_MODE_TRIANGLE_STRIP";
        case G_DRAW_MODE_TRIANGLE_FAN:             return "G_DRAW_MODE_TRIANGLE_FAN";
        case G_DRAW_MODE_TRIANGLES:                return "G_DRAW_MODE_TRIANGLES";
        case G_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY: return "G_DRAW_MODE_TRIANGLE_STRIP_ADJACENCY";
        case G_DRAW_MODE_TRIANGLES_ADJACENCY:      return "G_DRAW_MODE_TRIANGLES_ADJACENCY";
        case G_DRAW_MODE_PATCHES:                  return "G_DRAW_MODE_PATCHES";
        case G_DEBUG_MESSAGE_SEVERITY_NOTIFY:      return "G_DEBUG_MESSAGE_SEVERITY_NOTIFY";
        case G_DEBUG_MESSAGE_SEVERITY_WARNING:     return "G_DEBUG_MESSAGE_SEVERITY_WARNING";
        case G_DEBUG_MESSAGE_SEVERITY_ERROR:       return "G_DEBUG_MESSAGE_SEVERITY_ERROR";
        case G_DEBUG_MESSAGE_SEVERITY_CRITICAL:    return "G_DEBUG_MESSAGE_SEVERITY_CRITICAL"; 
        case G_WRAP_REPEAT:                        return "G_WRAP_REPEAT";
        case G_WRAP_MIRRORED_REPEAT:               return "G_WRAP_MIRRORED_REPEAT";
        case G_WRAP_CLAMP_TO_EDGE:                 return "G_WRAP_CLAMP_TO_EDGE";
        case G_WRAP_CLAMP_TO_BORDER:               return "G_WRAP_CLAMP_TO_BORDER";
        case G_MIN_FILTER_NEAREST:                 return "G_MIN_FILTER_NEAREST";
        case G_MIN_FILTER_LINEAR:                  return "G_MIN_FILTER_LINEAR";
        case G_MIN_FILTER_NEAREST_MIPMAP_NEAREST:  return "G_MIN_FILTER_NEAREST_MIPMAP_NEAREST";
        case G_MIN_FILTER_NEAREST_MIPMAP_LINEAR:   return "G_MIN_FILTER_NEAREST_MIPMAP_LINEAR";
        case G_MIN_FILTER_LINEAR_MIPMAP_NEAREST:   return "G_MIN_FILTER_LINEAR_MIPMAP_NEAREST";
        case G_MIN_FILTER_LINEAR_MIPMAP_LINEAR:    return "G_MIN_FILTER_LINEAR_MIPMAP_LINEAR";
        case G_MAG_FILTER_NEAREST:                 return "G_MAG_FILTER_NEAREST";
        case G_MAG_FILTER_LINEAR:                  return "G_MAG_FILTER_LINEAR";
        case G_TEXTURE_FORMAT_RED:                 return "G_TEXTURE_FORMAT_RED";
        case G_TEXTURE_FORMAT_RG:                  return "G_TEXTURE_FORMAT_RG";
        case G_TEXTURE_FORMAT_RGB:                 return "G_TEXTURE_FORMAT_RGB";
        case G_TEXTURE_FORMAT_BGR:                 return "G_TEXTURE_FORMAT_BGR";
        case G_TEXTURE_FORMAT_RGBA:                return "G_TEXTURE_FORMAT_RGBA";
        case G_TEXTURE_FORMAT_BGRA:                return "G_TEXTURE_FORMAT_BGRA";
        case G_TEXTURE_FORMAT_RED_INTEGER:         return "G_TEXTURE_FORMAT_RED_INTEGER";
        case G_TEXTURE_FORMAT_RG_INTEGER:          return "G_TEXTURE_FORMAT_RG_INTEGER";
        case G_TEXTURE_FORMAT_RGB_INTEGER:         return "G_TEXTURE_FORMAT_RGB_INTEGER";
        case G_TEXTURE_FORMAT_BGR_INTEGER:         return "G_TEXTURE_FORMAT_BGR_INTEGER";
        case G_TEXTURE_FORMAT_RGBA_INTEGER:        return "G_TEXTURE_FORMAT_RGBA_INTEGER";
        case G_TEXTURE_FORMAT_BGRA_INTEGER:        return "G_TEXTURE_FORMAT_BGRA_INTEGER";
        case G_TEXTURE_SLOT_0 :                    return "G_TEXTURE_SLOT_0";
        case G_TEXTURE_SLOT_1 :                    return "G_TEXTURE_SLOT_1";
        case G_TEXTURE_SLOT_2 :                    return "G_TEXTURE_SLOT_2";
        case G_TEXTURE_SLOT_3 :                    return "G_TEXTURE_SLOT_3";
        case G_TEXTURE_SLOT_4 :                    return "G_TEXTURE_SLOT_4";
        case G_TEXTURE_SLOT_5 :                    return "G_TEXTURE_SLOT_5";
        case G_TEXTURE_SLOT_6 :                    return "G_TEXTURE_SLOT_6";
        case G_TEXTURE_SLOT_7 :                    return "G_TEXTURE_SLOT_7";
        case G_TEXTURE_SLOT_8 :                    return "G_TEXTURE_SLOT_8";
        case G_TEXTURE_SLOT_9 :                    return "G_TEXTURE_SLOT_9";
        case G_TEXTURE_SLOT_10:                    return "G_TEXTURE_SLOT_10";
        case G_TEXTURE_SLOT_11:                    return "G_TEXTURE_SLOT_11";
        case G_TEXTURE_SLOT_12:                    return "G_TEXTURE_SLOT_12";
        case G_TEXTURE_SLOT_13:                    return "G_TEXTURE_SLOT_13";
        case G_TEXTURE_SLOT_14:                    return "G_TEXTURE_SLOT_14";
        case G_TEXTURE_SLOT_15:                    return "G_TEXTURE_SLOT_15";
        case G_TEXTURE_SLOT_16:                    return "G_TEXTURE_SLOT_16";
        case G_TEXTURE_SLOT_17:                    return "G_TEXTURE_SLOT_17";
        case G_TEXTURE_SLOT_18:                    return "G_TEXTURE_SLOT_18";
        case G_TEXTURE_SLOT_19:                    return "G_TEXTURE_SLOT_19";
        case G_TEXTURE_SLOT_20:                    return "G_TEXTURE_SLOT_20";
        case G_TEXTURE_SLOT_21:                    return "G_TEXTURE_SLOT_21";
        case G_TEXTURE_SLOT_22:                    return "G_TEXTURE_SLOT_22";
        case G_TEXTURE_SLOT_23:                    return "G_TEXTURE_SLOT_23";
        case G_TEXTURE_SLOT_24:                    return "G_TEXTURE_SLOT_24";
        case G_TEXTURE_SLOT_25:                    return "G_TEXTURE_SLOT_25";
        case G_TEXTURE_SLOT_26:                    return "G_TEXTURE_SLOT_26";
        case G_TEXTURE_SLOT_27:                    return "G_TEXTURE_SLOT_27";
        case G_TEXTURE_SLOT_28:                    return "G_TEXTURE_SLOT_28";
        case G_TEXTURE_SLOT_29:                    return "G_TEXTURE_SLOT_29";
        case G_TEXTURE_SLOT_30:                    return "G_TEXTURE_SLOT_30";
        case G_TEXTURE_SLOT_31:                    return "G_TEXTURE_SLOT_31";
        default:                                   return "Graphics enumeration string N/A";
    }
}