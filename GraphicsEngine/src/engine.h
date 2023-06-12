//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "platform.h"
#include "Texture.h"
#include "Entity.h"
#include "Camera.h"
#include "BufferManagement.h"

#include "Renderer.h"

#include <memory>

class Shader;

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

struct RendererOptions
{
    bool open;

    std::vector<const char*> renderTargets;

    // Environment Mapping Options
    bool activeSkybox;
    bool activeIrradiance;
    bool activeReflection;
    bool activeRefraction;

    // SSAO Options
    bool activeSSAO;
    bool activeRangeCheck;
    bool activeSSAOBlur;
    float ssaoRadius;
    float ssaoBias;
    float ssaoPower;
    int ssaoKernelSize;
};

struct Light
{
    // 3 components for position/direction and last for the type of the light
    // 0.0 is Directional light and 1.0 is Point light
    glm::vec4 lightVector;

    glm::vec3 color;
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
    RendererOptions rendererOptions;
    bool sceneGui;
    bool performanceGui;
    bool forwardRendering;

    // CAMERA //
    Camera camera;

    // Renderer
    Renderer renderer;

    // UNIFORM BUFFER (CONSTANT BUFFER) //
    int uniformBufferOffsetAlignment;
    Buffer UBO;
    u32 globalParamOffset;
    u32 globalParamSize;

    // ENTITIES //
    std::vector<Entity> entities;
    u32 numEntities;

    // LIGHTS //
    u32 firstLightEntityID;
    std::vector<Light> lights;
    u32 numLights;
    
    // RESOURCES //
    std::vector<std::unique_ptr<Model>> models;
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
void UpdateUniformBuffer(App* app);

Entity* CreateEntity(App* app, u32 shaderID, glm::vec3 position, Model* model);

void CreatePointLight(App* app, glm::vec3 position, glm::vec3 color, Model* model, float constant = 1.0f, float scale = 1.0f);
void CreateDirectionalLight(App* app, glm::vec3 entityPosition, glm::vec3 direction, glm::vec3 color, Model* model, float scale = 1.0f);
float Lerp(float a, float b, float f);