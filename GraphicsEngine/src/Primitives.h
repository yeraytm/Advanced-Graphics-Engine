#pragma once

#include "Engine.h"

enum class PrimitiveType
{
    PLANE,
    CUBE,
    SPHERE
};

// Returns the VAO of the Screen-Filling Quad
u32 CreateQuad();

Model* CreatePrimitive(App* app, PrimitiveType type, Material& material, u32 xNumSegments = 16, u32 yNumSegments = 32);