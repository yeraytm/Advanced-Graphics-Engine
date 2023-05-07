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

    bool isIndexed = true;
};

struct Material
{
    std::string name;

    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emissive;
    float shininess;

    u32 albedoTextureID;
    u32 emissiveTextureID;
    u32 specularTextureID;
    u32 normalsTextureID;
    u32 bumpTextureID;
};

enum class EntityType
{
    DEFAULT,
    PRIMITIVE,
    MODEL,
    LIGHT
};

struct Entity
{
public:
    Entity();
    Entity(EntityType type, u32 shaderID, glm::vec3 newPosition);
    Entity(EntityType type, u32 shaderID, glm::vec3 newPosition, Model* model, u32 modelID);
    ~Entity();

public:
    EntityType type;

    glm::vec3 position;
    glm::mat4 modelMatrix;

    u32 localParamOffset;
    u32 localParamSize;

    Model* model;
    u32 modelID;
    u32 shaderID;
};