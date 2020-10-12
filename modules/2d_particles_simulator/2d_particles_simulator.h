#pragma once

#include "apparatus.h"

enum Particle_Type {
    PARTICLE_POINTS,
    PARTICLE_TRIANGLES,
    PARTICLE_QUADS,
    PARTICLE_SPRITES
};

struct Particle_Simulation_State {
    bool is_playing = true;
    f32 time_passed = 0;

    u64 num_active_particles = 0;

    inline void reset() {
        time_passed = 0;
    }
    inline void play() {
        is_playing = true;
    }
    inline void pause() {
        is_playing = false;
    }
};

tag(component, custom_gui)
struct ParticleSimulation2D {

    Particle_Type type;
    bool loop = true;
    f32 duration = 5.f;
    f32 spawn_rate = 100;
    
    fvec2 angle_range = fvec2(0, mz::PI * 2);
    fvec2 speed_range = fvec2(100, 1000);

    f32 life_time = 1.f;

    bool preview_in_editor = true;
    bool play_on_start = true;

    Particle_Simulation_State state;
};

constexpr inline str_ptr_t particle_type_str(Particle_Type type) {
    switch (type)
    {
        case PARTICLE_POINTS:    return "Points";
        case PARTICLE_TRIANGLES: return "Triangles";
        case PARTICLE_QUADS:     return "Static Quads";
        case PARTICLE_SPRITES:   return "Sprites";
        default: return "N/A"; break;
    }
}

inline void on_gui(ParticleSimulation2D* sim) {
    auto do_selectable = [&](Particle_Type type) {
        if (ImGui::Selectable(particle_type_str(type), sim->type == type)) {
            sim->type = type;   
        }
    };

    if (ImGui::RBeginCombo("Particle type", particle_type_str(sim->type))) {

        do_selectable(PARTICLE_POINTS);
        do_selectable(PARTICLE_TRIANGLES);
        do_selectable(PARTICLE_QUADS);
        do_selectable(PARTICLE_SPRITES);

        ImGui::REndCombo();
    }

    ImGui::Spacing();ImGui::Spacing();
    ImGui::Text("State");
    ImGui::Spacing();ImGui::Spacing();
    ImGui::Text("Time passed: %f", sim->state.time_passed);
    u64 start_particle = 0;
    if (sim->state.time_passed > sim->life_time) {
        start_particle = (u64)((sim->state.time_passed - sim->life_time) * sim->spawn_rate);
    }
    ImGui::Text("Particle count: %lu", sim->state.num_active_particles);
    ImGui::RCheckbox("Preview", &sim->preview_in_editor);
    ImGui::RCheckbox("Play", &sim->state.is_playing);
    ImGui::Spacing();ImGui::Spacing();
    ImGui::Text("Config");
    ImGui::Spacing();ImGui::Spacing();
    ImGui::RCheckbox("Play on start", &sim->play_on_start);
    ImGui::RCheckbox("Loop", &sim->loop);
    ImGui::RDragFloat("Duration", &sim->duration, .05f, .01f);
    ImGui::RDragFloat("Spawn rate", &sim->spawn_rate, .1f, 1.f);
    ImGui::RDragFloatRange2("Start angle range", &sim->angle_range, .05f);
    ImGui::RDragFloatRange2("Start speed range", &sim->speed_range);
    ImGui::RDragFloat("Life time", &sim->life_time, .05f, .01f, 1000000.f);


    /*
    f32 duration = 5.f;
    f32 spawn_rate = 100;
    
    fvec2 angle_range = fvec2(0, mz::PI * 2);
    fvec2 speed_range = fvec2(100, 1000);
    */
}