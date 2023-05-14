#include "Entity.h"

Entity::Entity() : position(glm::vec3(0.0f)), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(0)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::Entity(u32 shaderID, glm::vec3 newPosition) : position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(shaderID)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::Entity(u32 shaderID, glm::vec3 newPosition, Model* model) : position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(model), shaderID(shaderID)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}