//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "Layouts.h"
#include "Shader.h"
#include "Texture.h"
#include "Entity.h"
#include "Camera.h"
#include "BufferManagement.h"
#include "Framebuffer.h"

struct OpenGLGUI
{
    bool open;
    std::string version;
    std::string vendor;
    std::string renderer;
    std::string glslVersion;
    std::vector<std::string> extensions;
    int numExtensions;
};

struct RendererGUI
{
    bool open;
    std::vector<const char*> renderTargets;
};

struct Quad
{
    Framebuffer FBO;
    u32 VAO;
    u32 shaderID;
    u32 currentRenderTarget;
};

struct Light
{
    // 3 components for position/direction and last for the type of the light
    // 0.0 is Directional light and 1.0 is Point light
    glm::vec4 lightVector;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
};

struct App
{
    // ENGINE PARAMETERS //
    float deltaTime;
    f64 currentTime;
    bool isRunning;
    glm::ivec2 displaySize;

    // INPUT //
    Input input;

    // IMGUI //
    OpenGLGUI openGLGui;
    RendererGUI rendererGui;
    bool sceneGui;
    bool performanceGui;

    // CAMERA //
    Camera camera;

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int uniformBufferOffsetAlignment;
    Buffer UBO;
    u32 globalParamOffset;
    u32 globalParamSize;

    // DEFERRED SHADING //
    Framebuffer GBuffer;
    u32 lightingPassShaderID;
    Quad screenQuad;

    // SHADERS //
    u32 defaultShaderID;
    u32 lightCasterShaderID;
    
    // ENTITIES //
    std::vector<Entity> entities;
    u32 numEntities;

    // LIGHTS //
    u32 firstLightEntityID;
    std::vector<Light> lights;
    u32 numLights;
    
    // RESOURCES //
    std::vector<Texture> textures;
    std::vector<Material> materials;
    std::vector<Shader> shaderPrograms;
};

void Init(App* app);

void ImGuiRender(App* app);

void Update(App* app);

void Render(App* app);

//void CleanUp(App* app);

// Engine Additional Functions
inline void BindDefaultFramebuffer() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

u32 FindVAO(Model* model, u32 meshIndex, const Shader& shaderProgram);

void UpdateUniformBuffer(App* app);

Entity* CreateEntity(App* app, u32 shaderID, glm::vec3 position, Model* model);

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, Model* model, float constant = 1.0f, float scale = 1.0f);
void CreateDirectionalLight(App* app, glm::vec3 entityPosition, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, Model* model, float scale = 1.0f);