#pragma once

typedef unsigned int u32;

struct App;

struct aiScene;
struct aiMesh;
struct aiMaterial;
struct aiNode;

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory);

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Model* myModel, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Model* myModel, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

u32 LoadModel(App* app, const char* filename);