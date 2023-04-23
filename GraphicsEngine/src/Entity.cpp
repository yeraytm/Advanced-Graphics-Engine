#include "Entity.h"

Entity::Entity(glm::vec3 position) : modelID(0), position(position), modelMatrix(glm::mat4(1.0f))
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}