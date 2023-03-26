#pragma once

#include "glad/glad.h"

#define ASSERT_DEBUG(x) if (!(x)) __debugbreak();

#ifdef _DEBUG
#define GLCall(x) GLClearError();\
	x;\
	ASSERT_DEBUG(GLLogCall(#x, __FILE__, __LINE__))
#else
#define GLCall(x) x
#endif

void GLClearError();

bool GLLogCall(const char* function, const char* file, int line);

void OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar * message, const void* userParam);