#include "apparatus.h"

#include <box2d/box2d.h>

#include "2d_sprite_renderer/2d_sprite_renderer.h"
#include "2d_viewport/2d_viewport.h"

#include "2d_physics.h"

class ContactListener : public b2ContactListener
{
public:
    void solve_contact(b2Contact* contact, collision_callback_t& callback1, collision_callback_t callback2) {
        entt::entity entity1 = (entt::entity)(uintptr_t)contact->GetFixtureA()->GetUserData().pentity;
        auto& body1 = *(PhysicsBody2D*)contact->GetFixtureA()->GetBody()->GetUserData().pbody;
        auto& shape1 = *(CollisionShape2D*)contact->GetFixtureA()->GetUserData().pshape;

        entt::entity entity2 = (entt::entity)(uintptr_t)contact->GetFixtureB()->GetUserData().pentity;
        auto& body2 = *(PhysicsBody2D*)contact->GetFixtureB()->GetBody()->GetUserData().pbody;
        auto& shape2 = *(CollisionShape2D*)contact->GetFixtureB()->GetUserData().pshape;

        auto& reg = *(entt::registry*)contact->GetFixtureA()->GetUserData().preg;

        bool y = reg.has<PhysicsBody2D, CollisionShape2D>(entity1); ap_assert(y, "Incorrect entity1");
        y = &reg.get<PhysicsBody2D>(entity1) == &body1; ap_assert(y, "Incorrect body1");
        y = &reg.get<CollisionShape2D>(entity1) == &shape1; ap_assert(y,  "Incorrect shape1");
        y = reg.has<PhysicsBody2D, CollisionShape2D>(entity2); ap_assert(y,  "Incorrect entity2");
        y = &reg.get<PhysicsBody2D>(entity2) == &body2; ap_assert(y,  "Incorrect body2");
        y = &reg.get<CollisionShape2D>(entity2) == &shape2; ap_assert(y,  "Incorrect shape2");

        Contact2D contact_1to2;
        contact_1to2.other_entity = entity2;
        contact_1to2.other_body = &body2;
        contact_1to2.other_shape = &shape2;

        Contact2D contact_2to1;
        contact_2to1.other_entity = entity1;
        contact_2to1.other_body = &body1;
        contact_2to1.other_shape = &shape1;

        if (callback1) callback1(contact_1to2);
        if (callback2) callback2(contact_2to1);
    }
    
    void BeginContact(b2Contact* contact) {
        auto& body1 = *(PhysicsBody2D*)contact->GetFixtureA()->GetBody()->GetUserData().pbody;
        auto& body2 = *(PhysicsBody2D*)contact->GetFixtureB()->GetBody()->GetUserData().pbody;

        solve_contact(contact, body1.on_contact_begin, body2.on_contact_begin);
    }   

    void EndContact(b2Contact* contact) {
        auto& body1 = *(PhysicsBody2D*)contact->GetFixtureA()->GetBody()->GetUserData().pbody;
        auto& body2 = *(PhysicsBody2D*)contact->GetFixtureB()->GetBody()->GetUserData().pbody;

        solve_contact(contact, body1.on_contact_end, body2.on_contact_end);
    }
};


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

ContactListener contact_listener;


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

void update_sim_body(entt::entity entity, PhysicsBody2D& body, Transform2D& transform, CollisionShape2D& shape) {
    b2Body* b2body = (b2Body*)body._body;
    log_trace("A physics body is being updated");
    if (body._fixture) {
        b2body->DestroyFixture((b2Fixture*)body._fixture);
        body._fixture = NULL;
    }
    
    b2FixtureDef fix_def;
    b2PolygonShape poly_shape;
    b2CircleShape circle_shape;
    switch (shape.shape_type) {
    case SHAPE_2D_RECT:
        poly_shape.SetAsBox(to_simulation_coord(shape.half_extents.x * transform.scale.x), to_simulation_coord(shape.half_extents.y * transform.scale.y));
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
    fix_def.userData.pshape = &shape;
    fix_def.userData.pentity = (void*)(uintptr_t)entity;
    fix_def.userData.preg = &get_entity_registry();

    body._fixture = b2body->CreateFixture(&fix_def);
    b2body->GetUserData().pbody = &body;
    b2body->GetUserData().pentity = (void*)(uintptr_t)entity;
    b2body->GetUserData().preg = &get_entity_registry();

    body.apply_force = [b2body](fvec2 force) {
        auto sim_force = to_simulation_vec2(force);
        b2body->ApplyForceToCenter({ sim_force.x, sim_force.y }, true);
    };
    body.apply_force_to_point = [b2body](fvec2 force, fvec2 point) {
        auto sim_force = to_simulation_vec2(force);
        auto sim_point = to_simulation_vec2(point);
        b2body->ApplyForce({ sim_force.x, sim_force.y }, { sim_point.x, sim_point.y }, true);
    };

    body.apply_impulse = [b2body](fvec2 impulse) {
        auto sim_impulse = to_simulation_vec2(impulse);
        b2body->ApplyLinearImpulseToCenter({ sim_impulse.x, sim_impulse.y }, true);
    };
    body.apply_impulse_to_point = [b2body](fvec2 impulse, fvec2 point) {
        auto sim_impulse = to_simulation_vec2(impulse);
        auto sim_point = to_simulation_vec2(point);
        b2body->ApplyLinearImpulse({ sim_impulse.x, sim_impulse.y }, { sim_point.x, sim_point.y }, true);
    };
}

module_scope {
    module_function(void) on_load() {
        simulation_thread = get_thread_server().make_thread();

        should_apply_transforms.store(false);
    }

    module_function(void) on_unload() {
        get_thread_server().kill_thread(simulation_thread);

        if (g_world) delete g_world;
    }

    

    module_function(void) on_play_begin() {
        g_world = new b2World(b2Vec2(0, 0));
        g_world->SetContactListener(&contact_listener);
        auto& reg = get_entity_registry();
        {
            
            const auto& view = reg.view<PhysicsBody2D, Transform2D, CollisionShape2D>();
            view.each([&reg](entt::entity entity, PhysicsBody2D& body, Transform2D& transform, CollisionShape2D& shape) {
                (void)entity;

                b2BodyDef body_def;
                auto sim_pos = to_simulation_vec2(transform.position) + to_simulation_vec2(shape.offset);
                body_def.position.Set(sim_pos.x, sim_pos.y);
                body_def.type = to_b2_body_type(body.body_type);
                auto b2body = g_world->CreateBody(&body_def);
                body._body = b2body;

                update_sim_body(entity, body, transform, shape);

                body._last_world_scale = transform.scale;
                shape._last_half_extents = shape.half_extents;
            });
        }
    }

    void step() {
        g_world->Step(1.f / tick_rate, velocity_iterations, position_iterations);
    }	

    module_function(void) on_update(float delta) {
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
                fvec2 last_vel = body._last_sim_vel;

                fvec2 now_sim_pos = { to_game_coord(b2b->GetPosition().x), to_game_coord(b2b->GetPosition().y) };
                f32   now_sim_rot = -b2b->GetAngle();
                fvec2 now_vel = { b2b->GetLinearVelocity().x, b2b->GetLinearVelocity().y };

                transform.position += (now_sim_pos - last_sim_pos);
                transform.rotation += (now_sim_rot - last_sim_rot);
                body.velocity      += to_game_vec2((now_vel - last_vel));
            });    
        }

        if (time_waited >= 1.f / tick_rate) {
            time_waited -= 1.f / tick_rate;
            
            get_thread_server().wait_for_thread(simulation_thread);
            reg.view<PhysicsBody2D, Transform2D, CollisionShape2D>().each([](entt::entity entity, PhysicsBody2D& body, Transform2D& transform, CollisionShape2D& shape) {
                b2Body* b2b = (b2Body*)body._body;
                
                auto pos_in_sim = to_simulation_vec2(transform.position) + to_simulation_vec2(shape.offset);
                b2b->SetTransform({ pos_in_sim.x, pos_in_sim.y }, -transform.rotation);

                if (body.body_type == BODY_TYPE_DYNAMIC) {
                    body.velocity +=  g_gravity;
                    auto vel_in_sim = to_simulation_vec2(body.velocity);
                    b2b->SetLinearVelocity(b2Vec2(vel_in_sim.x, vel_in_sim.y));
                }

                if (transform.scale != body._last_world_scale 
                 || shape.half_extents != shape._last_half_extents
                 || body.friction    != body._last_friction
                 || body.density     != body._last_density
                 || body.restitution != body._last_restitution) {
                    update_sim_body(entity, body, transform, shape);
                }

                body._last_sim_pos = { b2b->GetPosition().x, b2b->GetPosition().y };
                body._last_sim_rot = b2b->GetAngle();
                body._last_sim_vel = { b2b->GetLinearVelocity().x, b2b->GetLinearVelocity().y };

                body._last_friction = body.friction;
                body._last_density = body.density;
                body._last_restitution = body.restitution;

                body._last_world_scale = transform.scale;
                shape._last_half_extents = shape.half_extents;
            });
            get_thread_server().queue_task(simulation_thread, []() { 
                step();
                should_apply_transforms.store(true);
            });
        }
    }

    module_function(void) on_play_stop() {
        get_thread_server().wait_for_thread(simulation_thread);
        delete g_world;
        g_world = NULL;
        should_apply_transforms.store(false);
    }

    module_function(void) on_render() {
        
        
    }

    module_function(void) on_gui() {
        
    }
}