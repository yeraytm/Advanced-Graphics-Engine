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

class Entity
{
public:
    Entity();
    Entity(const std::string& name);
    ~Entity();

    void Render(const ShaderProgram& shaderProgram, std::vector<Material>& materials, std::vector<Texture>& textures, u32 programUniformTexture);

public:
    Model model;
    u32 modelID;

private:
    std::string m_Name;
};