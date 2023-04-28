#include "Camera.h"

Camera::Camera(glm::vec3 newPosition)
	: position(newPosition), speed(2.0f),
	front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f)), right(glm::vec3(0.0f)), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(-90.0f), pitch(0.0f), sensitivity(0.1f)
{
	UpdateVectors();
}

Camera::~Camera()
{
}

void Camera::UpdateVectors()
{
	front = glm::vec3(0.0f);
	front.x = glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	front.y = glm::sin(glm::radians(pitch));
	front.z = glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}

void Camera::ProcessMouse(glm::vec2 mouseDelta)
{
	// Camera Rotation (Look Around)
	mouseDelta.x *= sensitivity;
	mouseDelta.y *= sensitivity;

	// Constrain yaw to only use values between 0-360 as float precision could be lost
	yaw = glm::mod(yaw + mouseDelta.x, 360.0f); 

	// Negative to invert vertical movement
	pitch -= mouseDelta.y;

	// Avoid locking the camera completely up or down (pitch would be put of bounds and the screen would get flipped)
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	UpdateVectors();
}

void Camera::ProcessKeyboard(CameraDirection direction, float dt)
{
	// Camera Movement (Walk Around)
	float velocity = speed * dt;
	switch (direction)
	{
	case CameraDirection::CAMERA_FORWARD:
		position += front * velocity;
		break;
	case CameraDirection::CAMERA_BACKWARD:
		position -= front * velocity;
		break;
	case CameraDirection::CAMERA_LEFT:
		position -= right * velocity;
		break;
	case CameraDirection::CAMERA_RIGHT:
		position += right * velocity;
		break;
	case CameraDirection::CAMERA_UP:
		position += up * velocity;
		break;
	case CameraDirection::CAMERA_DOWN:
		position -= up * velocity;
		break;
	}
}

glm::mat4 Camera::GetViewMatrix() const
{
	return glm::lookAt(position, position + front, up);
}

// Rotate Camera around target
//const float radius = 10.0f;
//position.x = sin(app->currentTime) * radius;
//position.z = cos(app->currentTime) * radius;
//view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));