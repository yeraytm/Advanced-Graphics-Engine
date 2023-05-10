#pragma once

#include "Platform.h"

struct Image
{
    void* pixels;
    glm::ivec2 size;
    int nchannels;
    int stride;
};

struct Texture
{
    u32 handle;
    std::string filepath;
};

u32 LoadTexture2D(std::vector<Texture>& textures, const char* filepath, bool isFliped = true);