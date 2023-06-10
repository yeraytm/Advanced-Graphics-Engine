#include "Entity.h"

Entity::Entity() : position(glm::vec3(0.0f)), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(0)
{
	Translate(position);
}

Entity::Entity(u32 shaderID, const glm::vec3& newPosition) : position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(shaderID)
{
	Translate(position);
}

Entity::Entity(u32 shaderID, const glm::vec3& newPosition, Model* model) : position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(model), shaderID(shaderID)
{
	Translate(position);
}

Entity::~Entity()
{
}

void Entity::Translate(const glm::vec3& newPosition)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::Rotate(float newRotation, const glm::vec3& axis)
{
	modelMatrix = glm::rotate(modelMatrix, glm::radians(newRotation), axis);
}

void Entity::Scale(float newScale)
{
	modelMatrix = glm::scale(modelMatrix, glm::vec3(newScale));
}