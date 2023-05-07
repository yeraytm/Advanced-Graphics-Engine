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
#include "BufferManagement.h"

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

enum class LightType
{
    DIRECTIONAL,
    POINT
};

struct Light
{
    LightType type;

    glm::vec3 position;
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
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

    Camera camera;
    glm::mat4 projection;

    int uniformBufferOffsetAlignment;
    Buffer UBO;

    u32 numLights;
    std::vector<Light> lights;

    u32 numEntities;
    std::vector<Entity> entities;

    std::vector<Model*> models;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<ShaderProgram> shaderPrograms;

    // Location of the texture uniform in the textured quad shader
    //u32 quadTextureLocation;

    // Location of the textures uniform in the textured mesh shader
    u32 meshTextureAlbedoLocation;

    //u32 quadProgramID;
    //Entity quad;

    // Mode
    RenderMode mode;

    // TEMPORAL
    u32 cubeTextureAlbedoLocation;
    u32 cubeTextureSpecularLocation;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);