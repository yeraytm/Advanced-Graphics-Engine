#include "Shader.h"

GLuint CreateShaderProgram(String programSource, const char* shaderName)
{
    GLchar infoLogBuffer[1024] = {};
    GLsizei infoLogBufferSize = sizeof(infoLogBuffer);
    GLsizei infoLogSize;
    GLint success;

    char versionString[] = "#version 430\n";
    char shaderNameDefine[128];
    sprintf(shaderNameDefine, "#define %s\n", shaderName);
    char vertexShaderDefine[] = "#define VERTEX\n";
    char fragmentShaderDefine[] = "#define FRAGMENT\n";

    const GLchar* vertexShaderSource[] = {
        versionString,
        shaderNameDefine,
        vertexShaderDefine,
        programSource.str
    };
    const GLint vertexShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(vertexShaderDefine),
        (GLint)programSource.len
    };
    const GLchar* fragmentShaderSource[] = {
        versionString,
        shaderNameDefine,
        fragmentShaderDefine,
        programSource.str
    };
    const GLint fragmentShaderLengths[] = {
        (GLint)strlen(versionString),
        (GLint)strlen(shaderNameDefine),
        (GLint)strlen(fragmentShaderDefine),
        (GLint)programSource.len
    };

    GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vshader, ARRAY_COUNT(vertexShaderSource), vertexShaderSource, vertexShaderLengths);
    glCompileShader(vshader);
    glGetShaderiv(vshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with vertex shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fshader, ARRAY_COUNT(fragmentShaderSource), fragmentShaderSource, fragmentShaderLengths);
    glCompileShader(fshader);
    glGetShaderiv(fshader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fshader, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glCompileShader() failed with fragment shader %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    GLuint programHandle = glCreateProgram();
    glAttachShader(programHandle, vshader);
    glAttachShader(programHandle, fshader);
    glLinkProgram(programHandle);
    glGetProgramiv(programHandle, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programHandle, infoLogBufferSize, &infoLogSize, infoLogBuffer);
        ELOG("glLinkProgram() failed with program %s\nReported message:\n%s\n", shaderName, infoLogBuffer);
    }

    glUseProgram(0);

    glDetachShader(programHandle, vshader);
    glDetachShader(programHandle, fshader);
    glDeleteShader(vshader);
    glDeleteShader(fshader);

    return programHandle;
}

u32 LoadShaderProgram(std::vector<ShaderProgram>& shaderPrograms, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    ShaderProgram program = {};
    program.handle = CreateShaderProgram(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    shaderPrograms.push_back(program);

    return shaderPrograms.size() - 1;
}

void InputShaderLayout(ShaderProgram& shaderProgram)
{
    shaderProgram.vertexLayout.attributes.clear();

    char* attributeName;
    GLint activeAttributes, attributeNameMaxLength;
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);

    attributeName = new char[attributeNameMaxLength++];

    for (u32 i = 0; i < activeAttributes; ++i)
    {
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(shaderProgram.handle, i, attributeNameMaxLength + 1, NULL, &attributeSize, &attributeType, attributeName);

        u8 attributeLocation = glGetAttribLocation(shaderProgram.handle, attributeName);

        u8 componentCount = 1;
        switch (attributeType)
        {
        case GL_FLOAT: componentCount = 1; break;
        case GL_FLOAT_VEC2: componentCount = 2; break;
        case GL_FLOAT_VEC3: componentCount = 3; break;
        case GL_FLOAT_VEC4: componentCount = 4; break;
        default:
            break;
        }

        shaderProgram.vertexLayout.attributes.push_back({ attributeLocation, componentCount });
    }
    delete[] attributeName;
}