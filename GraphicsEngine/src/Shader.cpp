#include "Shader.h"

void Shader::Bind()
{
    glUseProgram(handle);
}

void Shader::Unbind()
{
    glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    glUniform1i(GetUniformLocation(name), value);
}

void Shader::SetUniform1ui(const std::string& name, u32 value)
{
    glUniform1ui(GetUniformLocation(name), value);
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}

void Shader::SetUniform2f(const std::string& name, const glm::vec2& value)
{
    glUniform2fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1)
{
    glUniform2f(GetUniformLocation(name), v0, v1);
}

void Shader::SetUniform3f(const std::string& name, const glm::vec3& value)
{
    glUniform3fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, const glm::vec4& value)
{
    glUniform4fv(GetUniformLocation(name), 1, &value[0]);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniformMat4(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}

int Shader::GetUniformLocation(const std::string& name) const
{
    auto locationSearch = m_UniformLocationCache.find(name);
    if (locationSearch != m_UniformLocationCache.end())
        return locationSearch->second;

    int location = glGetUniformLocation(handle, name.c_str());
    if (location == -1)
        ELOG("[WARNING] Shader Uniform doesn't exist: %s", name.c_str());

    m_UniformLocationCache[name] = location;

    return location;
}

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

void InputShaderLayout(Shader& shaderProgram)
{
    char* attributeName;
    int activeAttributes, attributeNameMaxLength;
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTES, &activeAttributes);
    glGetProgramiv(shaderProgram.handle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attributeNameMaxLength);

    attributeName = new char[++attributeNameMaxLength];

    for (int i = 0; i < activeAttributes; ++i)
    {
        GLint attributeSize;
        GLenum attributeType;
        glGetActiveAttrib(shaderProgram.handle, i, attributeNameMaxLength, NULL, &attributeSize, &attributeType, attributeName);

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

u32 LoadShaderProgram(std::vector<Shader>& shaderPrograms, ShaderType type, const char* filepath, const char* programName)
{
    String programSource = ReadTextFile(filepath);

    Shader program = {};
    program.handle = CreateShaderProgram(programSource, programName);
    program.filepath = filepath;
    program.programName = programName;
    program.lastWriteTimestamp = GetFileLastWriteTimestamp(filepath);
    program.type = type;

    InputShaderLayout(program);

    shaderPrograms.push_back(program);

    return shaderPrograms.size() - 1;
}