#pragma once

#include "platform.h"
#include "Layouts.h"

#include <unordered_map>

class Shader
{
public:
    //Shader();
    //~Shader();

    void Bind();
    void Unbind();

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1ui(const std::string& name, u32 value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform3f(const std::string& name, const glm::vec3& value);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, const glm::vec4& value);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4(const std::string& name, const glm::mat4& matrix);

public:
    u32 handle;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp;

    VertexShaderLayout vertexLayout;

private:
    int GetUniformLocation(const std::string& name) const;

private:
    // Caching for uniforms
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

GLuint CreateShaderProgram(String programSource, const char* shaderName);

u32 LoadShaderProgram(std::vector<Shader>& shaderPrograms, const char* filepath, const char* programName);