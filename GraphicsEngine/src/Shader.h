#pragma once

#include "platform.h"
#include "Layouts.h"

#include <unordered_map>

enum class ShaderType
{
    DEFAULT,
    TEXTURED_ALBEDO,
    TEXTURED_ALB_SPEC,
    SCREEN_QUAD,
    LIGHTING_PASS,
    LIGHT_CASTER,
    OTHER
};

class Shader
{
public:
    void Bind();
    void Unbind();

    void SetUniform1i(const std::string& name, int value);
    void SetUniform1ui(const std::string& name, u32 value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform2f(const std::string& name, const glm::vec2& value);
    void SetUniform2f(const std::string& name, float v0, float v1);
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

    ShaderType type;

private:
    int GetUniformLocation(const std::string& name) const;

private:
    // Caching for uniforms
    mutable std::unordered_map<std::string, GLint> m_UniformLocationCache;
};

GLuint CreateShaderProgram(String programSource, const char* shaderName);

u32 LoadShaderProgram(std::vector<Shader>& shaderPrograms, ShaderType type, const char* filepath, const char* programName);