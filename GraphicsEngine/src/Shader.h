#pragma once

#include "platform.h"
#include "Layouts.h"

class Shader
{
public:
    //Shader();
    //~Shader();

    void Bind();
    void Unbind();

    u32 handle;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp;

    VertexShaderLayout vertexLayout;
};

GLuint CreateShaderProgram(String programSource, const char* shaderName);

u32 LoadShaderProgram(std::vector<Shader>& shaderPrograms, const char* filepath, const char* programName);