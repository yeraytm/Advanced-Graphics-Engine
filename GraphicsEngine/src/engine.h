//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"
#include "Layouts.h"
#include "Shader.h"
#include "Texture.h"
#include "Entity.h"

enum class RenderMode
{
    TexturedQuad,
    TexturedMesh,
    Count
};

struct OpenGLState
{
    std::string version;
    std::string vendor;
    std::string renderer;
    std::string glslVersion;
    std::vector<std::string> extensions;
    int numExtensions;
};

struct App
{
    // Loop
    float deltaTime;
    bool isRunning;

    // Input
    Input input;

    bool debugInfo;
    OpenGLState glState;
    bool openGLStatus;

    glm::ivec2 displaySize;

    u32 numEntities;
    std::vector<Entity*> entities;

    std::vector<Model> models;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<ShaderProgram> shaderPrograms;

    // Program indices
    u32 texturedQuadProgramID;
    u32 texturedMeshProgramID;

    // Texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;

    // Mode
    RenderMode mode;

    // Embedded geometry (in-editor simple meshes such as
    // a screen filling quad, a cube, a sphere...)
    u32 VBO;
    u32 EBO;

    // Location of the texture uniform in the textured quad shader
    u32 programUniformTexture;

    // VAO object to link our screen filling quad with our textured quad shader
    VAO vao;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);