#pragma once

#include "platform.h"

class Shader;

struct Image
{
    void* pixels;
    glm::ivec2 size;
    int nchannels;
    int stride;
    bool isHDR;
};

struct Texture
{
    u32 handle;
    std::string filepath;
};

u32 LoadTexture2D(std::vector<Texture>& textures, const char* filepath, bool isFlipped = true);

u32 LoadCubemap(std::vector<Texture>& textures, const char* filepath, Shader& equirectToCubemapShader, u32 skyboxVAO);
u32 LoadCubemap(std::vector<std::string>& faces);