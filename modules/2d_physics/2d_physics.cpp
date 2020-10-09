#include "apparatus.h"

#include <box2d/box2d.h>

#include "ecs_2d_renderer/ecs_2d_renderer.h"

#include "2d_physics.h"

void* wnd;
Windows_Context* windows;

fvec2 g_gravity(0.0f, -10.0f);
b2World* g_world;
f32 tick_rate = 60.f;

thread_id_t simulation_thread;
f32 time_waited = 0;

s32 velocity_iterations = 6;
s32 position_iterations = 2;

std::atomic_bool should_apply_transforms = false;


f32 to_simulation_coord(f32 c) {
    return c / 100.f;
}

f32 to_game_coord(f32 c) {
    return c * 100.f;
}

fvec2 to_simulation_vec2(fvec2 v) {
    return { to_simulation_coord(v.x), to_simulation_coord(v.y) };
}

fvec2 to_game_vec2(fvec2 v) {
    return { to_game_coord(v.x), to_game_coord(v.y) };
}

constexpr b2BodyType to_b2_body_type(Physics_Body_Type type) {
    switch (type)
    {
        case BODY_TYPE_DYNAMIC: return b2_dynamicBody;
        case BODY_TYPE_KINEMATIC: return b2_kinematicBody;
        case BODY_TYPE_STATIC: return b2_staticBody;
        default: return b2_dynamicBody;;
    }
}

extern "C" {
    _export void __cdecl on_load(Graphics_Context* graphics) {
        log_info("on_load called in test_module!");
        (void)graphics;

        windows = graphics->get_windows_context();
        wnd = windows->main_window_handle;

        simulation_thread = get_thread_server().make_thread();

        should_apply_transforms.store(false);
    }

    _export void __cdecl on_unload(Graphics_Context* graphics) {
        (void)graphics;

        get_thread_server().kill_thread(simulation_thread);
    }

    

    _export void __cdecl on_play_begin() {
        g_world = new b2World(b2Vec2(0, 0));
        auto& reg = get_entity_registry();
        {
            
            const auto& view = reg.view<PhysicsBody2D, Transform2D, CollisionShape2D>();
            view.each([&reg](entt::entity entity, PhysicsBody2D& body2d, Transform2D& transform, CollisionShape2D& shape2d) {
                (void)entity;
                

                b2BodyDef body_def;

                b2FixtureDef fix_def;
                b2PolygonShape poly_shape;
                b2CircleShape circle_shape;
                switch (shape2d.shape_type) {
                case SHAPE_2D_RECT:
                    poly_shape.SetAsBox(to_simulation_coord(shape2d.half_extents.x * transform.scale.x), to_simulation_coord(shape2d.half_extents.y * transform.scale.y));
                    fix_def.shape = &poly_shape;
                    break;
                case SHAPE_2D_CIRCLE:
                    circle_shape.m_radius = to_simulation_coord(shape2d.half_extents.x * transform.scale.average());
                    fix_def.shape = &circle_shape;
                    break;
                }
                fix_def.friction = body2d.friction;
                fix_def.density = body2d.density;
                fix_def.restitution = body2d.restitution;

                auto sim_pos = to_simulation_vec2(transform.position) + to_simulation_vec2(shape2d.offset);
                body_def.position.Set(sim_pos.x, sim_pos.y);
                body_def.type = to_b2_body_type(body2d.body_type);

                auto body = g_world->CreateBody(&body_def);

                body2d._fixture = body->CreateFixture(&fix_def);
                body2d._body = body;

                body2d._last_world_scale = transform.scale;
                shape2d._last_half_extents = shape2d.half_extents;
            });
        }
    }

    void step() {
        g_world->Step(1.f / tick_rate, velocity_iterations, position_iterations);
    }	

    _export void __cdecl on_update(float delta) {
        (void)delta;
        time_waited += delta;
        auto& reg = get_entity_registry();
        
        if (!get_thread_server().is_thread_busy(simulation_thread) && should_apply_transforms.load()) {

            should_apply_transforms.store(false);

            reg.view<PhysicsBody2D, Transform2D, CollisionShape2D>().each([](PhysicsBody2D& body, Transform2D& transform, CollisionShape2D& shape) {
                (void)shape;
                b2Body* b2b = (b2Body*)body._body;

                fvec2 last_sim_pos = to_game_vec2(body._last_sim_pos);
                f32   last_sim_rot = -body._last_sim_rot;
                fvec2 last_sim_vel = to_game_vec2(body._last_sim_vel);

                fvec2 now_sim_pos = { to_game_coord(b2b->GetPosition().x), to_game_coord(b2b->GetPosition().y) };
                f32   now_sim_rot = -b2b->GetAngle();
                fvec2 now_sim_vel = { to_game_coord(b2b->GetLinearVelocity().x), to_game_coord(b2b->GetLinearVelocity().y) };

                transform.position += (now_sim_pos - last_sim_pos);
                transform.rotation += (now_sim_rot - last_sim_rot);
                body.velocity      += (now_sim_vel - last_sim_vel);
            });    
        }

        if (time_waited >= 1.f / tick_rate) {
            time_waited -= 1.f / tick_rate;
            
            get_thread_server().wait_for_thread(simulation_thread);
            reg.view<PhysicsBody2D, Transform2D, CollisionShape2D>().each([](PhysicsBody2D& body, Transform2D& transform, CollisionShape2D& shape) {
                b2Body* b2b = (b2Body*)body._body;
                
                auto pos_in_sim = to_simulation_vec2(transform.position) + to_simulation_vec2(shape.offset);
                b2b->SetTransform({ pos_in_sim.x, pos_in_sim.y }, -transform.rotation);

                if (body.body_type == BODY_TYPE_DYNAMIC) {
                    body.velocity +=  g_gravity;
                    auto vel_in_sim = to_simulation_vec2(body.velocity);
                    b2b->SetLinearVelocity(b2Vec2(vel_in_sim.x, vel_in_sim.y));
                }

                if (transform.scale != body._last_world_scale || shape.half_extents != shape.half_extents) {
                    b2FixtureDef fix_def;
                    b2PolygonShape poly_shape;
                    b2CircleShape circle_shape;
                    switch (shape.shape_type) {
                    case SHAPE_2D_RECT:
                        poly_shape.SetAsBox(to_simulation_coord(shape.half_extents.x * transform.scale.x), to_simulation_coord(shape.half_extents.y * transform.scale.x));
                        fix_def.shape = &poly_shape;
                        break;
                    case SHAPE_2D_CIRCLE:
                        circle_shape.m_radius = to_simulation_coord(shape.half_extents.x * transform.scale.average());
                        fix_def.shape = &circle_shape;
                        break;
                    }
                    fix_def.friction = body.friction;
                    fix_def.density = body.density;
                    fix_def.restitution = body.restitution;

                    b2b->DestroyFixture((b2Fixture*)body._fixture);
                    body._fixture = b2b->CreateFixture(&fix_def);
                }

                body._last_sim_pos = { b2b->GetPosition().x, b2b->GetPosition().y };
                body._last_sim_rot = b2b->GetAngle();
                body._last_sim_vel = { b2b->GetLinearVelocity().x, b2b->GetLinearVelocity().y };

                body._last_world_scale = transform.scale;
                shape._last_half_extents = shape.half_extents;
            });
            get_thread_server().queue_task(simulation_thread, []() { 
                step();
                should_apply_transforms.store(true);
            });
        }
    }

    _export void __cdecl on_play_stop() {
        get_thread_server().wait_for_thread(simulation_thread);
        delete g_world;
        should_apply_transforms.store(false);
    }

    _export void __cdecl on_render(Graphics_Context* graphics) {
        (void)graphics;
        
    }

    _export void __cdecl on_gui(Graphics_Context* graphics, ImGuiContext* imgui_ctx) {
        (void)graphics;(void)imgui_ctx;
    }
}