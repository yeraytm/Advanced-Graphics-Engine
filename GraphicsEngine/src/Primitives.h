#pragma once

#include "Engine.h"

enum class PrimitiveType
{
    PLANE,
    CUBE,
    SPHERE
};

Model* CreateQuad(App* app, Material& material);

Model* CreatePrimitive(App* app, PrimitiveType type, Material& material, u32 xNumSegments = 64, u32 yNumSegments = 128);