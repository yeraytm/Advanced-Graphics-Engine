#include "Entity.h"

Entity::Entity(std::string newName, glm::vec3 newPosition, bool primitive) : name(newName), isPrimitive(primitive), position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), modelID(0)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}