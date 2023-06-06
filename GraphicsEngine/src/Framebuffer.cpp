#include "Framebuffer.h"

#include "glad/glad.h"

void Framebuffer::Generate()
{
	glGenFramebuffers(1, &handle);
}

void Framebuffer::Delete()
{
	glDeleteFramebuffers(1, &handle);
}

void Framebuffer::Bind()
{
	// All following read and write framebuffer operations will affect the currently bound framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::CheckStatus()
{
	GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE)
	{
		ELOG("Error generating a framebuffer\n");
		switch (framebufferStatus)
		{
		case GL_FRAMEBUFFER_UNDEFINED:						ELOG("GL_FRAMEBUFFER_UNDEFINED\n");						break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT\n");			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:	ELOG("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT\n"); break;
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER\n");		break;
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER\n");		break;
		case GL_FRAMEBUFFER_UNSUPPORTED:					ELOG("GL_FRAMEBUFFER_UNSUPPORTED\n");					break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:			ELOG("GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE\n");		break;
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:		ELOG("GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS\n");		break;
		default:											ELOG("Unknown framebuffer status error\n");				break;
		}
	}
}

void Framebuffer::AttachDepthTexture(const glm::ivec2& size)
{
	depthAttachment = CreateAttachment(GL_DEPTH_ATTACHMENT, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT, size, false);
}

void Framebuffer::AttachColorTexture(FBAttachmentType attachmentType, const glm::ivec2& size, bool clamp)
{
	u32 numAttachments = colorAttachmentHandles.size();

	u32 textureHandle = 0;
	switch (attachmentType)
	{
	case FBAttachmentType::COLOR_BYTE:
		textureHandle = CreateAttachment(GL_COLOR_ATTACHMENT0 + numAttachments, GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE, size, clamp);
		break;
	case FBAttachmentType::COLOR_FLOAT:
		textureHandle = CreateAttachment(GL_COLOR_ATTACHMENT0 + numAttachments, GL_RGBA16F, GL_RGBA, GL_FLOAT, size, clamp);
		break;
	case FBAttachmentType::COLOR_R:
		textureHandle = CreateAttachment(GL_COLOR_ATTACHMENT0 + numAttachments, GL_RED, GL_RED, GL_FLOAT, size, clamp);
		break;
	}
	colorAttachmentHandles.push_back(textureHandle);
}

u32 Framebuffer::CreateAttachment(GLenum target, GLint internalFormat, GLenum dataFormat, GLenum dataType, const glm::ivec2& size, bool clamp) const
{
	GLuint attachmentHandle;
	glGenTextures(1, &attachmentHandle);
	glBindTexture(GL_TEXTURE_2D, attachmentHandle);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, size.x, size.y, 0, dataFormat, dataType, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, target, attachmentHandle, 0);

	return attachmentHandle;
}

void Framebuffer::SetColorBuffers()
{
	std::vector<GLuint> buffers;
	buffers.reserve(colorAttachmentHandles.size());

	for (u32 i = 0; i < colorAttachmentHandles.size(); ++i)
		buffers.emplace_back(GL_COLOR_ATTACHMENT0 + i);

	glDrawBuffers(colorAttachmentHandles.size(), buffers.data());

	CheckStatus();
}