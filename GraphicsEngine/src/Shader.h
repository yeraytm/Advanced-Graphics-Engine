#pragma once

#include "Platform.h"
#include "Layouts.h"

struct ShaderProgram
{
    u32 handle;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp;

    VertexShaderLayout vertexLayout;
};

GLuint CreateShaderProgram(String programSource, const char* shaderName);

u32 LoadShaderProgram(std::vector<ShaderProgram>& shaderPrograms, const char* filepath, const char* programName);

void InputShaderLayout(ShaderProgram& shaderProgram);