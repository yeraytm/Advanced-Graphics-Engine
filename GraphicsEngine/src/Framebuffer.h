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

    void AttachDepthTexture(const glm::ivec2& size);
    void AttachColorTexture(FBAttachmentType attachmentType, const glm::ivec2& size);
    void SetColorBuffers();

private:
    u32 CreateAttachment(GLenum attachmentType, GLint internalFormat, GLenum dataFormat, GLenum dataType, const glm::ivec2& size);
    void CheckStatus();

public:
    u32 handle;
    u32 depthAttachment;
    std::vector<u32> colorAttachmentHandles;
};