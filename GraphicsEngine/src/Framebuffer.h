#pragma once

#include "Platform.h"

typedef int GLint;
typedef unsigned int GLenum;

enum class FBAttachmentType
{
    COLOR_BYTE,
    COLOR_FLOAT,
    DEPTH
};

class Framebuffer
{
public:
    void Bind();
    void Unbind();

    void Generate();
    void Delete();

    void CheckStatus();

    void AttachDepthTexture(glm::ivec2& size);
    void AttachColorTexture(FBAttachmentType attachmentType, glm::ivec2& size);
    void SetColorBuffers();

private:
    u32 CreateAttachment(GLenum attachmentType, GLint internalFormat, GLenum dataFormat, GLenum dataType, glm::ivec2& size);

public:
    u32 handle;
    u32 depthAttachment;
    std::vector<u32> colorAttachmentHandles;
};