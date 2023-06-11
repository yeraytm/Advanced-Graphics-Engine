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

struct Material
{
    std::string name;

    glm::vec3 albedo;
    glm::vec3 specular = glm::vec3(0.5f);
    glm::vec3 reflective = glm::vec3(0.0f);
    glm::vec3 emissive = glm::vec3(0.0f);
    float shininess = 32.0f / 256.0f;

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
    Entity(u32 shaderID, const glm::vec3& newPosition);
    Entity(u32 shaderID, const glm::vec3& newPosition, Model* model);
    ~Entity();

    void Translate(const glm::vec3& newPosition);
    void Rotate(float newRotation, const glm::vec3& axis);
    void Scale(float newScale);

    inline const glm::mat4& GetModelMatrix() const { return modelMatrix; }

public:
    glm::vec3 position;

    u32 localParamOffset;
    u32 localParamSize;

    Model* model;
    u32 shaderID;

private:
    glm::mat4 modelMatrix;
};