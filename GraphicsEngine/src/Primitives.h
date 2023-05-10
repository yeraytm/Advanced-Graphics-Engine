#pragma once

#include "Engine.h"

enum class PrimitiveType
{
    PLANE,
    CUBE,
    SPHERE
};

u32 CreateQuad(App* app, Material& material, Model* model);

u32 CreatePrimitive(PrimitiveType type, App* app, Model* model, Material& material, u32 xNumSegments = 64, u32 yNumSegments = 128);