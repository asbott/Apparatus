#pragma once

#include "apparatus.h"

tag(component)
struct KeyboardMovement {
    tag(property)
    float vspeed = 100.f;
    tag(property)
    float hspeed = 100.f;
};