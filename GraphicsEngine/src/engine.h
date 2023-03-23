//
// engine.h: This file contains the types and functions relative to the engine.
//

#pragma once

#include "Platform.h"
#include "Layouts.h"

#include "glad/glad.h"

enum Mode
{
    Mode_TexturedQuad,
    Mode_TexturedMesh,
    Mode_Count
};

struct GLInfo
{
    bool openGLStatus;
    const char* version;
    const char* vendor;
    const char* renderer;
    const char* glslVersion;
    std::string extensions;
};

struct Vertex
{
    glm::vec3 position;
    glm::vec2 texCoords;
    //vec3 normal;
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
    u64 lastWriteTimestamp; // What is this for?

    VertexShaderLayout vertexLayout;
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

struct App
{
    // Loop
    float deltaTime;
    bool isRunning;

    // Input
    Input input;

    GLInfo glInfo;

    glm::ivec2 displaySize;

    u32 modelID;
    std::vector<Model> models;
    std::vector<Mesh> meshes;
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
    Mode mode;

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