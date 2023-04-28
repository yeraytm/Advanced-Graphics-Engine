#pragma once

#include "Platform.h"
#include "Layouts.h"

struct ShaderProgram;
struct Texture;

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

    u32 VBHandle = 0;
    u32 EBHandle = 0;
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

struct Entity
{
public:
    Entity();
    Entity(glm::vec3 newPosition, bool hasIndices = true);
    ~Entity();

public:
    glm::vec3 position;
    glm::mat4 modelMatrix;

    u32 localParamOffset;
    u32 localParamSize;

    Model* model;
    u32 modelID;
    bool hasIndices;
};