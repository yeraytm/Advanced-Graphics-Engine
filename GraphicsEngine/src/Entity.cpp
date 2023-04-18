#include "Entity.h"

Entity::Entity() : modelID(0)
{
}

Entity::~Entity()
{
}

void Entity::Render()
{
    //Model& model = app->models[app->patrickEntity.modelID];
    //u32 numMeshes = model.meshes.size();
    //for (u32 meshIndex = 0; meshIndex < model.meshes.size(); ++meshIndex)
    //{
    //    u32 vao = FindVAO(model, meshIndex, texturedMeshProgram);
    //    glBindVertexArray(vao);

    //    u32 meshMaterialID = model.materialIDs[meshIndex];
    //    Material& meshMaterial = app->materials[meshMaterialID];

    //    glActiveTexture(GL_TEXTURE0);
    //    glBindTexture(GL_TEXTURE_2D, app->textures[meshMaterial.albedoTextureID].handle);
    //    glUniform1i(app->programUniformTexture, 0);

    //    Mesh& mesh = model.meshes[meshIndex];
    //    glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, (void*)(u64)mesh.indexOffset);

    //    glBindVertexArray(0);
    //}
    //glUseProgram(0);
}