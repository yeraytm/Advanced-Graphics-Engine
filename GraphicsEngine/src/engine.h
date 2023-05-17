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
#include "Framebuffer.h"

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

struct Quad
{
    Framebuffer FBO;
    u32 VAO;
    u32 shaderHandle;
    u32 renderTarget;
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

    float constant;
};

struct App
{
    // LOOP //
    float deltaTime;
    f64 currentTime;
    bool isRunning;

    glm::ivec2 displaySize;

    // INPUT //
    Input input;

    // OPENGL DEBUG & IMGUI //
    OpenGLState glState;
    bool openGLStatus;
    bool debugInfo;
    bool sceneInfo;

    // CAMERA & PROJECTION //
    Camera camera;

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int uniformBufferOffsetAlignment;
    Buffer UBO;
    u32 globalParamOffset;
    u32 globalParamSize;

    // DEFERRED SHADING //
    Framebuffer gBuffer;        // G-Buffer
    u32 lightingPassProgram;    // Lighting Pass Handle
    Quad screenQuad;            // Screen-Filling Quad

    // SHADERS & UNIFORM TEXTURES //
    u32 defaultProgramID;
    u32 lightCasterProgramID;
    
    // ENTITIES //
    std::vector<Entity> entities;
    u32 numEntities;

    // LIGHTS //
    u32 firstLightEntityID;
    std::vector<Light> lights;
    u32 numLights;
    
    // RESOURCES //
    std::vector<Model*> models;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<ShaderProgram> shaderPrograms;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);

void CleanUp(App* app);

// Engine Additional Functions
u32 FindVAO(Model* model, u32 meshIndex, const ShaderProgram& shaderProgram);

void UpdateUniformBuffer(App* app);

Entity* CreateEntity(App* app, u32 shaderID, glm::vec3 position, Model* model);

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, Model* model, float scale);
void CreateDirectionalLight(App* app, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);