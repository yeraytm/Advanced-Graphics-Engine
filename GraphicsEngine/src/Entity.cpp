#include "Entity.h"

Entity::Entity(glm::vec3 newPosition) : modelID(0), position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0)
{
	modelMatrix = glm::translate(modelMatrix, newPosition);
}

Entity::~Entity()
{
}

void Entity::Translate()
{
	modelMatrix = glm::translate(modelMatrix, position);
}