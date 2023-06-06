#pragma once

#include "platform.h"

typedef int GLint;
typedef unsigned int GLenum;

enum class FBAttachmentType
{
    COLOR_BYTE,
    COLOR_FLOAT,
    COLOR_R,
    DEPTH
};

class Framebuffer
{
public:
    void Generate();
    void Delete();

    void Bind();

    void AttachDepthTexture(const glm::ivec2& size);
    void AttachColorTexture(FBAttachmentType attachmentType, const glm::ivec2& size, bool clamp = false);
    void SetColorBuffers();

private:
    u32 CreateAttachment(GLenum attachmentType, GLint internalFormat, GLenum dataFormat, GLenum dataType, const glm::ivec2& size, bool clamp) const;
    void CheckStatus();

public:
    u32 handle;
    std::vector<u32> colorAttachmentHandles;
    u32 depthAttachment;
};