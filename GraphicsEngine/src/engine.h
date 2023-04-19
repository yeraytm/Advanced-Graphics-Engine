//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"

#include "Layouts.h"

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

struct Image
{
    void* pixels;
    glm::ivec2 size;
    int nchannels;
    int stride;
};

struct Texture
{
    u32 handle;
    std::string filepath;
};

struct ShaderProgram
{
    u32 handle;
    std::string filepath;
    std::string programName;
    u64 lastWriteTimestamp;

    VertexShaderLayout vertexLayout;
};

struct Material
{
    std::string name;

    glm::vec3 albedo;
    glm::vec3 emissive;
    float smoothness;

    u32 albedoTextureID;
    u32 emissiveTextureID;
    u32 specularTextureID;
    u32 normalsTextureID;
    u32 bumpTextureID;
};

struct VAO
{
    u32 handle;
    u32 shaderProgramHandle;
};

struct Mesh
{
    VertexBufferLayout VBLayout;
    std::vector<float> vertices;
    std::vector<u32> indices;
    u32 vertexOffset;
    u32 indexOffset;

    std::vector<VAO> VAOs;
};

struct Model
{
    std::vector<Mesh> meshes;
    std::vector<u32> materialIDs;

    u32 VBHandle;
    u32 EBHandle;
};

class Entity
{
public:
    Entity();
    Entity(const std::string& name);
    ~Entity();

public:
    Model model;
    u32 modelID;

private:
    std::string m_Name;
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

u32 LoadTexture2D(App* app, const char* filepath);

void Render(Entity& entity, const ShaderProgram& shaderProgram, App* app);