//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"
#include "Layouts.h"
#include "Shader.h"
#include "Texture.h"
#include "Entity.h"
#include "Camera.h"

enum class RenderMode
{
    QUAD,
    TEXTURE_MESH
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
    f64 currentTime;
    bool isRunning;

    glm::ivec2 displaySize;

    // Input
    Input input;

    OpenGLState glState;
    bool openGLStatus;
    bool debugInfo;
    bool sceneInfo;

    u32 numEntities;
    std::vector<Entity> entities;

    std::vector<Model> models;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<ShaderProgram> shaderPrograms;

    // Program indices
    u32 quadProgramID;
    u32 meshProgramID;

    // Location of the texture uniform in the textured quad shader
    u32 quadTextureLocation;
    // Location of the texture uniform in the textured mesh shader
    u32 meshTextureLocation;

    Camera camera;

    glm::mat4 projection;

    int uniformBufferOffsetAlignment;
    u32 UBO;

    Entity quad;

    // Mode
    RenderMode mode;

    // Texture indices
    u32 diceTexIdx;
    u32 whiteTexIdx;
    u32 blackTexIdx;
    u32 normalTexIdx;
    u32 magentaTexIdx;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);