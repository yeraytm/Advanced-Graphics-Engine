#include "Entity.h"

Entity::Entity(glm::vec3 newPosition, bool hasIndices) : position(newPosition), hasIndices(hasIndices), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), modelID(0)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}