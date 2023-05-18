#include "GLDebugger.h"
#include "platform.h"

#include <iostream>

void GLClearError()
{
	while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line)
{
	while (GLenum errorCode = glGetError())
	{
		std::string error;
		switch (errorCode)
		{
		case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
		case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
		case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
		case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
		case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
		case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
		case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
		}
		ELOG("[ERROR] OpenGL (Code %u: %s): %s %s:%d\n", errorCode, error.c_str(), function, file, line);
		return false;
	}
	return true;
}

void APIENTRY OnGLError(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    if (severity == GL_DEBUG_SEVERITY_NOTIFICATION)
        return;

    ELOG("----------------------\n");
    ELOG("OpenGL Debug Message (%u): %s\n", id, message);

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:               ELOG("Source: API\n");                  break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     ELOG("Source: Window System\n");        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:   ELOG("Source: Shader Compiler\n");      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:       ELOG("Source: Third Party\n");          break;
    case GL_DEBUG_SOURCE_APPLICATION:       ELOG("Source: Application\n");          break;
    case GL_DEBUG_SOURCE_OTHER:             ELOG("Source: Other\n");                break;
    }

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:               ELOG("Type: Error\n");                  break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: ELOG("Type: Deprecated Behaviour\n");   break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  ELOG("Type: Undefined Behaviour\n");    break;
    case GL_DEBUG_TYPE_PORTABILITY:         ELOG("Type: Portability\n");            break;
    case GL_DEBUG_TYPE_PERFORMANCE:         ELOG("Type: Performance\n");            break;
    case GL_DEBUG_TYPE_MARKER:              ELOG("Type: Marker\n");                 break;
    case GL_DEBUG_TYPE_PUSH_GROUP:          ELOG("Type: Push Group\n");             break;
    case GL_DEBUG_TYPE_POP_GROUP:           ELOG("Type: Pop Group\n");              break;
    case GL_DEBUG_TYPE_OTHER:               ELOG("Type: Other\n");                  break;
    }

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:            ELOG("Severity: high\n");               break;
    case GL_DEBUG_SEVERITY_MEDIUM:          ELOG("Severity: medium\n");             break;
    case GL_DEBUG_SEVERITY_LOW:             ELOG("Severity: low\n");                break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:    ELOG("Severity: notification\n");       break;
    }
}