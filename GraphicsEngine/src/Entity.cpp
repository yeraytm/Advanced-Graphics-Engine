#include "Entity.h"

#include "Shader.h"
#include "Texture.h"

#include "glad/glad.h"

u32 FindVAO(Model& model, u32 meshIndex, const ShaderProgram& shaderProgram)
{
    Mesh& mesh = model.meshes[meshIndex];

    // Try Finding a VAO for this mesh/program
    for (u32 i = 0; i < (u32)mesh.VAOs.size(); ++i)
    {
        if (mesh.VAOs[i].shaderProgramHandle == shaderProgram.handle)
            return mesh.VAOs[i].handle;
    }

    u32 vaoHandle = 0;

    // Create new VAO for this mesh/program
    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glBindBuffer(GL_ARRAY_BUFFER, model.VBHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBHandle);

    // We have to link all vertex inputs attributes to attributes in the vertex buffer
    for (u32 i = 0; i < shaderProgram.vertexLayout.attributes.size(); ++i)
    {
        bool attributeWasLinked = false;

        const std::vector<VertexBufferAttribute>& attributes = mesh.VBLayout.attributes;
        for (u32 j = 0; j < attributes.size(); ++j)
        {
            if (shaderProgram.vertexLayout.attributes[i].location == attributes[j].location)
            {
                const u32 index = attributes[j].location;
                const u32 nComp = attributes[j].componentCount;
                const u32 offset = attributes[j].offset + mesh.vertexOffset;
                const u32 stride = mesh.VBLayout.stride;

                glVertexAttribPointer(index, nComp, GL_FLOAT, GL_FALSE, stride, (void*)(u64)offset);
                glEnableVertexAttribArray(index);

                attributeWasLinked = true;
                break;
            }
        }
        assert(attributeWasLinked); // The mesh should provide an attribute for each vertex inputs
    }

    glBindVertexArray(0);

    // Store the VAO handle in the list of VAOs for this mesh
    VAO vao = { vaoHandle, shaderProgram.handle };
    mesh.VAOs.push_back(vao);

    return vaoHandle;
}

Entity::Entity() : modelID(0), m_Name("DefaultEntityName")
{
}

Entity::Entity(const std::string& name) : modelID(0), m_Name(name)
{
}

Entity::~Entity()
{
}

void Entity::Render(const ShaderProgram& shaderProgram, std::vector<Material>& materials, std::vector<Texture>& textures, u32 programUniformTexture)
{
    u32 numMeshes = model.meshes.size();
    for (u32 meshIndex = 0; meshIndex < numMeshes; ++meshIndex)
    {
        u32 vao = FindVAO(model, meshIndex, shaderProgram);
        glBindVertexArray(vao);

        u32 meshMaterialID = model.materialIDs[meshIndex];
        Material& meshMaterial = materials[meshMaterialID];

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[meshMaterial.albedoTextureID].handle);
        glUniform1i(programUniformTexture, 0);

        Mesh& mesh = model.meshes[meshIndex];
        glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

        glBindVertexArray(0);
    }
    glUseProgram(0);
}