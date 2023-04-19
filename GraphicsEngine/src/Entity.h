//#pragma once
//
//#include "Platform.h"
//#include "Layouts.h"
//
//struct VAO
//{
//    u32 handle;
//    u32 shaderProgramHandle;
//};
//
//struct Mesh
//{
//    VertexBufferLayout VBLayout;
//    std::vector<float> vertices;
//    std::vector<u32> indices;
//    u32 vertexOffset;
//    u32 indexOffset;
//
//    std::vector<VAO> VAOs;
//};
//
//struct Model
//{
//    std::vector<Mesh> meshes;
//    std::vector<u32> materialIDs;
//
//    u32 VBHandle;
//    u32 EBHandle;
//};
//
//class Entity
//{
//public:
//	Entity();
//	Entity(const std::string& name);
//	~Entity();
//
//	//void Render(const ShaderProgram& shaderProgram, App* app);
//
//public:
//	Model model;
//    u32 modelID;
//
//private:
//	std::string m_Name;
//};