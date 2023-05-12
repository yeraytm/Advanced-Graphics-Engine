#pragma once

#include "Platform.h"

typedef int GLint;
typedef unsigned int GLenum;

class Framebuffer
{
public:
    Framebuffer();
    ~Framebuffer();

    //void Draw();

    void Bind();
    void Unbind();

    void Generate();
    void Delete();

    void AttachColorTexture(GLenum attachmentType, glm::ivec2 size);
    void AttachDepthTexture(glm::ivec2 size);
    void CheckStatus();

private:
    u32 AttachTexture(GLenum attachmentType, GLint internalFormat, GLenum dataFormat, GLenum dataType, glm::ivec2 size);

public:
    u32 handle;

    std::vector<u32> colorAttachmentHandles;
};