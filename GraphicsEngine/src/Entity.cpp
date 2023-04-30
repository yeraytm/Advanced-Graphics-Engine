#include "Entity.h"

Entity::Entity() : type(MODEL), position(glm::vec3(0.0f)), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), modelID(0), hasIndices(true), shaderID(0)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::Entity(Type type, u32 shaderID, glm::vec3 newPosition, bool hasIndices) : type(type), position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), modelID(0), hasIndices(hasIndices), shaderID(shaderID)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}