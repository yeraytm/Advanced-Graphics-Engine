#include "Entity.h"

Entity::Entity() : type(EntityType::MODEL), position(glm::vec3(0.0f)), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(0)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::Entity(EntityType type, u32 shaderID, glm::vec3 newPosition) : type(type), position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(nullptr), shaderID(shaderID)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::Entity(EntityType type, u32 shaderID, glm::vec3 newPosition, Model* model) : type(type), position(newPosition), modelMatrix(glm::mat4(1.0f)), localParamOffset(0), localParamSize(0), model(model), shaderID(shaderID)
{
	modelMatrix = glm::translate(modelMatrix, position);
}

Entity::~Entity()
{
}