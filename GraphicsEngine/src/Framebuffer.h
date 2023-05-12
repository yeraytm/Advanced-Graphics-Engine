#pragma once

#include "Platform.h"

typedef int GLint;
typedef unsigned int GLenum;

enum class FBAttachmentType
{
    COLOR,
    DEPTH,
    NORMALS
};

class Framebuffer
{
public:
    void Bind();
    void Unbind();

    void Generate();
    void Delete();

    void CheckStatus();

    void AttachTexture(FBAttachmentType attachmentType, glm::ivec2& size);
    void SetBuffers();

private:
    u32 CreateAttachment(GLenum attachmentType, GLint internalFormat, GLenum dataFormat, GLenum dataType, glm::ivec2& size);

public:
    u32 handle;

    std::vector<u32> colorAttachmentHandles;
};