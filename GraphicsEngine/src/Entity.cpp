#include "Entity.h"

Entity::Entity() : position(glm::vec3(0.0f)), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), modelID(0), hasIndices(true)
{
}

Entity::Entity(glm::vec3 newPosition, bool hasIndices) : position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), modelID(0), hasIndices(hasIndices)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}