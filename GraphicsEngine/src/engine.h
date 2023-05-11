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

struct Quad
{
    u32 VAO;
    u32 textureHandle;
    u32 shaderHandle;
    u32 textureUniformLocation;
};

enum class LightType
{
    DIRECTIONAL,
    POINT
};

struct Light
{
    Light(LightType type, glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant) 
        : type(type), position(position), direction(direction), ambient(ambient), diffuse(diffuse), specular(specular), constant(constant)
    {}

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

    // OPENGL STATE & IMGUI //
    OpenGLState glState;
    bool openGLStatus;
    bool debugInfo;
    bool sceneInfo;

    // CAMERA //
    Camera camera;
    glm::mat4 projection;

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int uniformBufferOffsetAlignment;
    Buffer UBO;
    u32 globalParamOffset;
    u32 globalParamSize;
    
    // LIGHTS //
    u32 numLights;
    std::vector<Light> lights;

    // ENTITIES //
    u32 numEntities;
    std::vector<Entity> entities;
    
    // RESOURCES //
    std::vector<Model*> models;
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<ShaderProgram> shaderPrograms;

    // COMMON SHADERS IDs //
    u32 defaultProgramID;
    u32 lightProgramID;
    // Location of the texture's uniform in the textured mesh shader
    u32 meshTextureAlbedoLocation;

    // SCREEN-FILLING QUAD //
    Quad quad;

    // MODE //
    RenderMode mode;

    // TEMPORAL //
    u32 cubeTextureAlbedoLocation;
    u32 cubeTextureSpecularLocation;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);

// Engine Additional Functions
u32 FindVAO(Model* model, u32 meshIndex, const ShaderProgram& shaderProgram);

void UpdateUniformBuffer(App* app);

Entity* CreateEntity(App* app, EntityType type, u32 shaderID, glm::vec3 position, Model* model);

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, Model* model, float scale);
void CreateDirectionalLight(App* app, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);