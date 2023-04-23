#include "Entity.h"

Entity::Entity() : modelID(0), position(glm::vec3(0.0f))
{
}

Entity::Entity(glm::vec3 position) : modelID(0), position(position)
{
}

Entity::~Entity()
{
}