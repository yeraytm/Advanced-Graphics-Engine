#pragma once

#include "Engine.h"

u32 CreateQuad(App* app, Model* model)
{
    app->models.push_back(model);
    u32 modelID = (u32)app->models.size() - 1u;

    Material material = {};
    material.name = "Quad";
    material.albedoTextureID = app->whiteTexIdx;
    model->materialIDs.push_back((u32)app->materials.size());
    app->materials.push_back(material);

    Mesh mesh = {};
    mesh.vertices.insert(mesh.vertices.end(),
        {
            -0.5f, -0.5f,  0.0f,
            //0.0f,  0.0f,  1.0f,
            0.0f,  0.0f,

            0.5f, -0.5f,  0.0f,
            //0.0f,  1.0f,  1.0f,
            1.0f,  0.0f,

            0.5f,  0.5f,  0.0f,
            //0.0f,  1.0f,  1.0f,
            1.0f,  1.0f,

           -0.5f,  0.5f,  0.0f,
           //0.0f,  0.0f,  1.0f,
           0.0f,  1.0f
        });

    mesh.indices.insert(mesh.indices.end(),
        {
            0, 1, 2,
            0, 2, 3
        });

    // Create the vertex format
    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    //mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    //mesh.VBLayout.stride = 6 * sizeof(float);
    mesh.VBLayout.stride = 3 * sizeof(float);

    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 1, 2, mesh.VBLayout.stride });
    mesh.VBLayout.stride += 2 * sizeof(float);

    mesh.vertexOffset = 0;
    mesh.indexOffset = 0;

    glGenBuffers(1, &model->VBHandle);
    glBindBuffer(GL_ARRAY_BUFFER, model->VBHandle);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &model->EBHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model->EBHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(u32), mesh.indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    model->meshes.push_back(mesh);

    return modelID;
}

u32 CreateCube(App* app, Model* model)
{
    app->models.push_back(model);
    u32 modelID = (u32)app->models.size() - 1u;

    Material material = {};
    material.name = "Cube";
    material.albedoTextureID = app->whiteTexIdx;
    model->materialIDs.push_back((u32)app->materials.size());
    app->materials.push_back(material);

    Mesh mesh = {};
    mesh.vertices.insert(mesh.vertices.end(),
        {
            -0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             0.0f,  0.0f,

             0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             1.0f,  1.0f,

             0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             1.0f,  0.0f,

             0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             1.0f,  1.0f,

            -0.5f, -0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             0.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,
             0.0f,  0.0f, -1.0f,
             0.0f,  1.0f,
             //
            -0.5f, -0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             0.0f,  0.0f,

             0.5f, -0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             1.0f,  0.0f,

             0.5f,  0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             1.0f,  1.0f,

             0.5f,  0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             1.0f,  1.0f,

            -0.5f,  0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             0.0f,  1.0f,

            -0.5f, -0.5f,  0.5f,
             0.0f,  0.0f,  1.0f,
             0.0f,  0.0f,
             //
            -0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  1.0f,

            -0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             0.0f,  1.0f,

            -0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             0.0f,  1.0f,

            -0.5f, -0.5f,  0.5f,
             0.0f,  0.0f,  0.0f,
             0.0f,  0.0f,

            -0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  0.0f,
             //
             0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  0.0f,

             0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             0.0f,  1.0f,

             0.5f,  0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  1.0f,

             0.5f, -0.5f, -0.5f,
             1.0f,  0.0f,  0.0f,
             0.0f,  1.0f,

             0.5f,  0.5f,  0.5f,
             1.0f,  0.0f,  0.0f,
             1.0f,  0.0f,

             0.5f, -0.5f,  0.5f,
             1.0f,  0.0f,  0.0f,
             0.0f,  0.0f,
             //
            -0.5f, -0.5f, -0.5f,
             0.0f, -1.0f,  0.0f,
             0.0f,  1.0f,

             0.5f, -0.5f, -0.5f,
             0.0f, -1.0f,  0.0f,
             1.0f,  1.0f,

             0.5f, -0.5f,  0.5f,
             0.0f, -1.0f,  0.0f,
             1.0f,  0.0f,

             0.5f, -0.5f,  0.5f,
             0.0f, -1.0f,  0.0f,
             1.0f,  0.0f,

            -0.5f, -0.5f,  0.5f,
             0.0f, -1.0f,  0.0f,
             0.0f,  0.0f,

            -0.5f, -0.5f, -0.5f,
             0.0f, -1.0f,  0.0f,
             0.0f,  1.0f,
             //
            -0.5f,  0.5f, -0.5f,
             0.0f,  1.0f,  0.0f,
             0.0f,  1.0f,

             0.5f,  0.5f,  0.5f,
             0.0f,  1.0f,  0.0f,
             1.0f,  0.0f,

             0.5f,  0.5f, -0.5f,
             0.0f,  1.0f,  0.0f,
             1.0f,  1.0f,

             0.5f,  0.5f,  0.5f,
             0.0f,  1.0f,  0.0f,
             1.0f,  0.0f,

            -0.5f,  0.5f, -0.5f,
             0.0f,  1.0f,  0.0f,
             0.0f,  1.0f,

            -0.5f,  0.5f,  0.5f,
             0.0f,  1.0f,  0.0f,
             0.0f,  0.0f
        });

    // Create the vertex format
    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    mesh.VBLayout.stride = 6 * sizeof(float);

    mesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, mesh.VBLayout.stride });
    mesh.VBLayout.stride += 2 * sizeof(float);

    mesh.vertexOffset = 0;
    mesh.indexOffset = 0;

    glGenBuffers(1, &model->VBHandle);
    glBindBuffer(GL_ARRAY_BUFFER, model->VBHandle);
    glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size() * sizeof(float), mesh.vertices.data(), GL_STATIC_DRAW);

    //glGenBuffers(1, &model.EBHandle);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBHandle);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.indices.size() * sizeof(u32), mesh.indices.data(), GL_STATIC_DRAW);

    ///glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    model->meshes.push_back(mesh);

    return modelID;
}