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
	Camera(glm::vec3 position, float FOV, float nearPlane, float farPlane, float speed = CAMERA_DEFAULT_SPEED, bool freeCam = true);

    void Update(const Input& input, const glm::ivec2& displaySize, float deltaTime, float currentTime);

    void Zoom(float scrollY);

    inline const glm::mat4& GetViewMatrix(const glm::ivec2& displaySize) const { return m_View; }
    inline const glm::mat4& GetProjectionMatrix(const glm::ivec2& displaySize) const { return m_Projection; }

public:
    glm::vec3 position;

    // Movement Options
    float speed;
    float defaultSpeed;

    // Projection Options
    float FOV;

    //Pivot Camera Options
    bool freeCamera;
    float m_Radius;
    bool autoRotate;
    float m_RotationSpeed;

private:
    void ProcessKeyboard(CameraDirection direction, float dt);
    void ProcessMouse(const glm::vec2& mouseDelta);
    void UpdateVectors();
    void UpdateVectorsPivotCamera();

private:
    // Matrices
    glm::mat4 m_View;
    glm::mat4 m_Projection;

    // Direction Vectors
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