#include "AssimpLoading.h"

#include "Layouts.h"
#include "Texture.h"

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory)
{
    aiString name;

    aiColor3D diffuseColor;
    aiColor3D emissiveColor;
    aiColor3D specularColor;
    ai_real shininess;

    material->Get(AI_MATKEY_NAME, name);
    material->Get(AI_MATKEY_COLOR_DIFFUSE, diffuseColor);
    material->Get(AI_MATKEY_COLOR_EMISSIVE, emissiveColor);
    material->Get(AI_MATKEY_COLOR_SPECULAR, specularColor);
    material->Get(AI_MATKEY_SHININESS, shininess);

    myMaterial.name = name.C_Str();
    myMaterial.albedo = glm::vec3(diffuseColor.r, diffuseColor.g, diffuseColor.b);
    myMaterial.emissive = glm::vec3(emissiveColor.r, emissiveColor.g, emissiveColor.b);
    myMaterial.smoothness = shininess / 256.0f;

    aiString aiFilename;
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
    {
        material->GetTexture(aiTextureType_DIFFUSE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.albedoTextureID = LoadTexture2D(app->textures, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
    {
        material->GetTexture(aiTextureType_EMISSIVE, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.emissiveTextureID = LoadTexture2D(app->textures, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0)
    {
        material->GetTexture(aiTextureType_SPECULAR, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.specularTextureID = LoadTexture2D(app->textures, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0)
    {
        material->GetTexture(aiTextureType_NORMALS, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.normalsTextureID = LoadTexture2D(app->textures, filepath.str);
    }
    if (material->GetTextureCount(aiTextureType_HEIGHT) > 0)
    {
        material->GetTexture(aiTextureType_HEIGHT, 0, &aiFilename);
        String filename = MakeString(aiFilename.C_Str());
        String filepath = MakePath(directory, filename);
        myMaterial.bumpTextureID = LoadTexture2D(app->textures, filepath.str);
    }

    //myMaterial.createNormalFromBump();
}

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Model* myModel, u32 baseMeshMaterialIndex, std::vector<u32>& modelMaterialIndices)
{
    Mesh myMesh = {};

    bool hasTexCoords = false;
    bool hasTangentSpace = false;

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        myMesh.vertices.push_back(mesh->mVertices[i].x);
        myMesh.vertices.push_back(mesh->mVertices[i].y);
        myMesh.vertices.push_back(mesh->mVertices[i].z);
        myMesh.vertices.push_back(mesh->mNormals[i].x);
        myMesh.vertices.push_back(mesh->mNormals[i].y);
        myMesh.vertices.push_back(mesh->mNormals[i].z);

        if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
        {
            hasTexCoords = true;
            myMesh.vertices.push_back(mesh->mTextureCoords[0][i].x);
            myMesh.vertices.push_back(mesh->mTextureCoords[0][i].y);
        }

        if (mesh->mTangents != nullptr && mesh->mBitangents)
        {
            hasTangentSpace = true;
            myMesh.vertices.push_back(mesh->mTangents[i].x);
            myMesh.vertices.push_back(mesh->mTangents[i].y);
            myMesh.vertices.push_back(mesh->mTangents[i].z);

            // For some reason ASSIMP gives me the bitangents flipped.
            // Maybe it's my fault, but when I generate my own geometry
            // in other files (see the generation of standard assets)
            // and all the bitangents have the orientation I expect,
            // everything works ok.
            // I think that (even if the documentation says the opposite)
            // it returns a left-handed tangent space matrix.
            // SOLUTION: I invert the components of the bitangent here.
            myMesh.vertices.push_back(-mesh->mBitangents[i].x);
            myMesh.vertices.push_back(-mesh->mBitangents[i].y);
            myMesh.vertices.push_back(-mesh->mBitangents[i].z);
        }
    }

    // Process indices
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            myMesh.indices.push_back(face.mIndices[j]);
        }
    }

    // Store the proper (previously proceessed) material for this mesh
    modelMaterialIndices.push_back(baseMeshMaterialIndex + mesh->mMaterialIndex);

    // Create the vertex format
    myMesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 0, 3, 0 });
    myMesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 1, 3, 3 * sizeof(float) });
    myMesh.VBLayout.stride = 6 * sizeof(float);

    if (hasTexCoords)
    {
        myMesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 2, 2, myMesh.VBLayout.stride });
        myMesh.VBLayout.stride += 2 * sizeof(float);
    }
    if (hasTangentSpace)
    {
        myMesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 3, 3, myMesh.VBLayout.stride });
        myMesh.VBLayout.stride += 3 * sizeof(float);

        myMesh.VBLayout.attributes.push_back(VertexBufferAttribute{ 4, 3, myMesh.VBLayout.stride });
        myMesh.VBLayout.stride += 3 * sizeof(float);
    }

    // add the mesh into the model
    myModel->meshes.push_back(myMesh);
}

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Model* myModel, u32 baseMeshMaterialIndex, std::vector<u32>& modelMaterialIndices)
{
    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        ProcessAssimpMesh(scene, mesh, myModel, baseMeshMaterialIndex, modelMaterialIndices);
    }

    // then do the same for each of its children
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessAssimpNode(scene, node->mChildren[i], myModel, baseMeshMaterialIndex, modelMaterialIndices);
    }
}

u32 LoadModel(App* app, const char* filename, Model& model)
{
    const aiScene* scene = aiImportFile(filename,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_CalcTangentSpace |
        aiProcess_JoinIdenticalVertices |
        aiProcess_PreTransformVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_OptimizeMeshes |
        aiProcess_SortByPType);

    if (!scene)
    {
        ELOG("Error loading mesh %s: %s\n", filename, aiGetErrorString());
        return UINT32_MAX;
    }

    app->models.push_back(Model{});
    model = app->models.back();
    u32 modelID = (u32)app->models.size() - 1u;

    String directory = GetDirectoryPart(MakeString(filename));

    // Create a list of materials
    u32 baseMeshMaterialIndex = (u32)app->materials.size();
    for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
    {
        app->materials.push_back(Material{});
        Material& material = app->materials.back();
        ProcessAssimpMaterial(app, scene->mMaterials[i], material, directory);
    }

    ProcessAssimpNode(scene, scene->mRootNode, &model, baseMeshMaterialIndex, model.materialIDs);

    aiReleaseImport(scene);

    u32 vertexBufferSize = 0;
    u32 indexBufferSize = 0;

    for (u32 i = 0; i < model.meshes.size(); ++i)
    {
        vertexBufferSize += model.meshes[i].vertices.size() * sizeof(float);
        indexBufferSize += model.meshes[i].indices.size() * sizeof(u32);
    }

    glGenBuffers(1, &model.VBHandle);
    glBindBuffer(GL_ARRAY_BUFFER, model.VBHandle);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, NULL, GL_STATIC_DRAW);

    glGenBuffers(1, &model.EBHandle);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.EBHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, NULL, GL_STATIC_DRAW);

    u32 indicesOffset = 0;
    u32 verticesOffset = 0;

    for (u32 i = 0; i < model.meshes.size(); ++i)
    {
        const void* verticesData = model.meshes[i].vertices.data();
        const u32 verticesSize = model.meshes[i].vertices.size() * sizeof(float);
        glBufferSubData(GL_ARRAY_BUFFER, verticesOffset, verticesSize, verticesData);
        model.meshes[i].vertexOffset = verticesOffset;
        verticesOffset += verticesSize;

        const void* indicesData = model.meshes[i].indices.data();
        const u32 indicesSize = model.meshes[i].indices.size() * sizeof(u32);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, indicesOffset, indicesSize, indicesData);
        model.meshes[i].indexOffset = indicesOffset;
        indicesOffset += indicesSize;
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return modelID;
}