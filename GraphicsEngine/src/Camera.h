#pragma once

#include "Platform.h"

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
	Camera(glm::vec3 cameraPosition = glm::vec3(0.0f));
	~Camera();

    void ProcessMouse(glm::vec2 mouseDelta);
    void ProcessKeyboard(CameraDirection direction, float dt);
    glm::mat4 GetViewMatrix() const;

public:
    glm::vec3 position;

    // Options
    float speed;

private:
    void UpdateVectors();

private:
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;

    // Euler Angles
    float yaw, pitch;

    // Options
    float sensitivity;
};