#pragma once

#include "Platform.h"

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
	Camera(glm::vec3 position, const glm::ivec2& displaySize, float FOV, float nearPlane, float farPlane, float speed = CAMERA_DEFAULT_SPEED);
	~Camera();

    void ProcessInput(const Input& input, float deltaTime);

    void SetProjectionMatrix(const glm::ivec2& displaySize);
    inline glm::mat4 GetProjectionMatrix() const { return projection; }
    glm::mat4 GetViewMatrix() const;

public:
    glm::vec3 position;

    // Options
    float speed;
    float defaultSpeed;

private:
    void ProcessMouse(glm::vec2 mouseDelta);
    void ProcessKeyboard(CameraDirection direction, float dt);
    void UpdateVectors();

private:
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw, pitch;

    // Projection Matrix
    glm::mat4 projection;

    // Projection Options
    float FOV;
    float nearPlane;
    float farPlane;

    // Camera Options
    float sensitivity;
};