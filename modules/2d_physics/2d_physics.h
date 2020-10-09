#pragma once

#include "apparatus.h"

enum Physics_Body_Type {
    BODY_TYPE_DYNAMIC, BODY_TYPE_KINEMATIC, BODY_TYPE_STATIC
};

enum Collision_Shape_Type_2D {
    SHAPE_2D_RECT, SHAPE_2D_CIRCLE,
};

constexpr inline str_ptr_t body_type_str(Physics_Body_Type body_type) {
    switch (body_type)
    {
        case BODY_TYPE_DYNAMIC:   return "Dynamic Body";
        case BODY_TYPE_KINEMATIC: return "Kinematic Body";
        case BODY_TYPE_STATIC:    return "Static Body";
        default: return "N/A"; break;
    }
}
constexpr inline str_ptr_t shape_type_str(Collision_Shape_Type_2D body_type) {
    switch (body_type)
    {
        case SHAPE_2D_RECT:   return "Rectangle shape";
        case SHAPE_2D_CIRCLE: return "Circle shape";
        default: return "N/A"; break;
    }
}

tag(component, custom_gui)
struct PhysicsBody2D {
    f32 friction = .3f;
    f32 density = 1.f;
    f32 restitution = .0f;
    fvec2 velocity = 0;
    
    Physics_Body_Type body_type = BODY_TYPE_DYNAMIC;

    fvec2 _last_sim_pos;
    f32 _last_sim_rot;
    fvec2 _last_sim_vel;
    fvec2 _last_world_scale;

    void* _body;
    void* _fixture;
};

tag(component, custom_gui)
struct CollisionShape2D {
    fvec2 offset = 0;
    bool is_trigger = false;
    fvec2 half_extents = 32;
    Collision_Shape_Type_2D shape_type = SHAPE_2D_RECT;


    fvec2 _last_half_extents;
};

inline void on_gui(PhysicsBody2D* body, ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);

    if (ImGui::RBeginCombo("Body type", body_type_str(body->body_type))) {
        if (ImGui::Selectable(body_type_str(BODY_TYPE_DYNAMIC))) {
            body->body_type = BODY_TYPE_DYNAMIC;
        }
        if (ImGui::Selectable(body_type_str(BODY_TYPE_KINEMATIC))) {
            body->body_type = BODY_TYPE_KINEMATIC;
        }
        if (ImGui::Selectable(body_type_str(BODY_TYPE_STATIC))) {
            body->body_type = BODY_TYPE_STATIC;
        }
        ImGui::REndCombo();
    }

    if (body->body_type == BODY_TYPE_DYNAMIC) {
        ImGui::RDragFloat2("Velocity", body->velocity.ptr, .1f);
        ImGui::RDragFloat("Friction", &body->friction, .01f, 0.f, 1.f);
        ImGui::RDragFloat("Density", &body->density, .01f, 0.f, 100000.f);
        ImGui::RDragFloat("Restitution", &body->restitution, .01f, 0.f, 1.f);
    }
}

inline void on_gui(CollisionShape2D* shape, ImGuiContext* ctx) {
    ImGui::SetCurrentContext(ctx);

    ImGui::RDragFloat2("Offset", shape->offset.ptr, .1f);
    ImGui::RCheckbox("Is trigger", &shape->is_trigger);

    if (ImGui::RBeginCombo("Shape type", shape_type_str(shape->shape_type))) {
        if (ImGui::Selectable(shape_type_str(SHAPE_2D_RECT))) {
            shape->shape_type = SHAPE_2D_RECT;
        }
        if (ImGui::Selectable(shape_type_str(SHAPE_2D_CIRCLE))) {
            shape->shape_type = SHAPE_2D_CIRCLE;
        }
        ImGui::REndCombo();
    }

    switch (shape->shape_type) {
    case SHAPE_2D_RECT:
        ImGui::RDragFloat2("Half-extents", shape->half_extents.ptr, .1f, .001f, 99999999999999.f);
        break;
    case SHAPE_2D_CIRCLE:
        ImGui::RDragFloat("Radius", shape->half_extents.ptr, .1f, .001f, 99999999999999.f);
        break;
    }
}