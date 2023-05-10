#include "Framebuffer.h"

#include "glad/glad.h"

Framebuffer::Framebuffer() : handle(0)
{
}

Framebuffer::~Framebuffer()
{
}

void Framebuffer::Bind()
{
	// All following read and write framebuffer operations will affect the currently bound framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, handle);
}

void Framebuffer::Unbind()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::Generate()
{
	glGenFramebuffers(1, &handle);

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

void Framebuffer::Delete()
{
	glDeleteFramebuffers(1, &handle);
}

void Framebuffer::AttachTexture(GLenum attachmentType, GLenum internalFormat, GLenum dataFormat, GLenum dataType, int sizeX, int sizeY)
{
	GLuint attachmentHandle;
	glGenTextures(1, &attachmentHandle);
	glBindTexture(GL_TEXTURE_2D, attachmentHandle);

	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, sizeX, sizeY, 0, dataFormat, dataType, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, sizeX, sizeY, 0, GL_RGBA8, GL_UNSIGNED_BYTE, NULL);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, sizeX, sizeY, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST); //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR)

	/*Shouldnt be necessary to set wrapping methods*/
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, attachmentHandle, 0);
}