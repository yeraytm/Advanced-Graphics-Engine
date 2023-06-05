#pragma once

#include "platform.h"

#define CAMERA_DEFAULT_SPEED 3.0f

enum class CameraDirection
{
    CAMERA_FORWARD,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
};

class Camera
{
public:
    Camera(float speed = CAMERA_DEFAULT_SPEED);
	Camera(glm::vec3 position, float FOV, float nearPlane, float farPlane, float speed = CAMERA_DEFAULT_SPEED);

    void ProcessInput(const Input& input, const glm::ivec2& displaySize, float deltaTime);

    void Zoom(float scrollY);

    glm::mat4 GetViewProjectionMatrix(const glm::ivec2& displaySize) const;
    glm::mat4 GetViewMatrix(const glm::ivec2& displaySize) const;
    glm::mat4 GetProjectionMatrix(const glm::ivec2& displaySize) const;

public:
    glm::vec3 position;

    // Movement Options
    float speed;
    float defaultSpeed;

    // Projection Options
    float FOV;

private:
    void ProcessKeyboard(CameraDirection direction, float dt);
    void ProcessMouse(const glm::vec2& mouseDelta);
    void UpdateVectors();

private:
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;

    // Camera Options
    float m_Sensitivity;

    // Euler Angles
    float m_Yaw, m_Pitch;

    // Projection Options
    float m_DefaultFOV;
    float m_NearPlane;
    float m_FarPlane;
};