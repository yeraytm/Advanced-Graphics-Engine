#include "Camera.h"

Camera::Camera(float speed)
	: position(glm::vec3(0.0f)), speed(speed), defaultSpeed(speed),
	front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f)), right(glm::vec3(0.0f)), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(-90.0f), pitch(0.0f), FOV(45.0f), nearPlane(0.1f), farPlane(100.0f), sensitivity(0.1f)
{
	UpdateVectors();

	projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(FOV), 1280.0f / 720.0f, nearPlane, farPlane);
}

Camera::Camera(glm::vec3 position, const glm::ivec2& displaySize, float FOV, float nearPlane, float farPlane, float speed)
	: position(position), speed(speed), defaultSpeed(speed),
	front(glm::vec3(0.0f, 0.0f, -1.0f)), up(glm::vec3(0.0f)), right(glm::vec3(0.0f)), worldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	yaw(-90.0f), pitch(0.0f), FOV(FOV), nearPlane(nearPlane), farPlane(farPlane), sensitivity(0.1f)
{
	UpdateVectors();

	projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(FOV), float(displaySize.x) / float(displaySize.y), nearPlane, farPlane);
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

void Camera::ProcessInput(const Input& input, float deltaTime)
{
	//ProcessMouse(input.mouseDelta);

	if (input.keys[K_LSHIFT] == BUTTON_PRESS)
		speed *= 3.0f;
	if (input.keys[K_LSHIFT] == BUTTON_RELEASE)
		speed = defaultSpeed;

	if (input.keys[K_W] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_FORWARD, deltaTime);
	if (input.keys[K_S] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_BACKWARD, deltaTime);
	if (input.keys[K_A] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_LEFT, deltaTime);
	if (input.keys[K_D] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_RIGHT, deltaTime);
	if (input.keys[K_Q] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_UP, deltaTime);
	if (input.keys[K_E] == BUTTON_PRESSED)
		ProcessKeyboard(CameraDirection::CAMERA_DOWN, deltaTime);

	if (input.mouseButtons[MOUSE_LEFT] == BUTTON_PRESS)
	{
		
	}
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
		position += worldUp * velocity;
		break;
	case CameraDirection::CAMERA_DOWN:
		position -= worldUp * velocity;
		break;
	}
}

void Camera::SetProjectionMatrix(const glm::ivec2& displaySize)
{
	projection = glm::perspective(glm::radians(FOV), float(displaySize.x) / float(displaySize.y), nearPlane, farPlane);
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