#pragma once

#include "Platform.h"

typedef unsigned int GLenum;

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    void Draw();

    void Bind();
    void Unbind();

    void Generate();
    void Delete();

    void AttachTexture(GLenum attachmentType, GLenum internalFormat, GLenum dataFormat, GLenum dataType, int sizeX, int sizeY);

public:
    u32 handle;
};