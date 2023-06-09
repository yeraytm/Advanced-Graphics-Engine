#pragma once

#include "engine.h"

#include "assimp/cimport.h"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

void ProcessAssimpMaterial(App* app, aiMaterial* material, Material& myMaterial, String directory, bool flipTextures);

void ProcessAssimpMesh(const aiScene* scene, aiMesh* mesh, Model& myModel, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

void ProcessAssimpNode(const aiScene* scene, aiNode* node, Model& myModel, u32 baseMeshMaterialIndex, std::vector<u32>& submeshMaterialIndices);

Model* LoadModel(App* app, const char* filename, bool flipTextures = true);