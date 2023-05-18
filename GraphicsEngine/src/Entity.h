#pragma once

#include "platform.h"
#include "Layouts.h"

class Shader;
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

enum class MaterialType
{
    DEFAULT,
    TEXTURED_ALBEDO,
    TEXTURED_ALB_SPEC
};

struct Material
{
    MaterialType type;
    std::string name;

    glm::vec3 albedo;
    glm::vec3 specular;
    glm::vec3 emissive;
    float shininess;

    u32 albedoTextureID;
    u32 emissiveTextureID;
    u32 specularTextureID;
    u32 normalsTextureID;
    u32 bumpTextureID;
};

class Entity
{
public:
    Entity();
    Entity(u32 shaderID, glm::vec3 newPosition);
    Entity(u32 shaderID, glm::vec3 newPosition, Model* model);
    ~Entity();

public:
    glm::vec3 position;
    glm::mat4 modelMatrix;

    u32 localParamOffset;
    u32 localParamSize;

    Model* model;
    u32 shaderID;
};