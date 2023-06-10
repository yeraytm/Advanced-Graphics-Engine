#include "Camera.h"

Camera::Camera(float speed) :
	position(glm::vec3(0.0f)), m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Up(glm::vec3(0.0f)), m_Right(glm::vec3(0.0f)), m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	speed(speed), defaultSpeed(speed), m_Sensitivity(0.1f), m_Yaw(-90.0f), m_Pitch(0.0f),
	FOV(45.0f), m_DefaultFOV(45.0f), m_NearPlane(0.1f), m_FarPlane(100.0f)
{
	UpdateVectors();
}

Camera::Camera(glm::vec3 position, float FOV, float nearPlane, float farPlane, float speed) :
	position(position), m_Front(glm::vec3(0.0f, 0.0f, -1.0f)), m_Up(glm::vec3(0.0f)), m_Right(glm::vec3(0.0f)), m_WorldUp(glm::vec3(0.0f, 1.0f, 0.0f)),
	speed(speed), defaultSpeed(speed), m_Sensitivity(0.1f), m_Yaw(-90.0f), m_Pitch(0.0f),
	FOV(FOV), m_DefaultFOV(FOV), m_NearPlane(nearPlane), m_FarPlane(farPlane)
{
	UpdateVectors();
}

void Camera::Update(const Input& input, const glm::ivec2& displaySize, float deltaTime)
{
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

	if (input.mouseButtons[MOUSE_LEFT] == BUTTON_PRESSED)
		ProcessMouse(input.mouseDelta);

	m_View = glm::lookAt(position, position + m_Front, m_Up);
	m_Projection = glm::perspective(glm::radians(FOV), float(displaySize.x) / float(displaySize.y), m_NearPlane, m_FarPlane);
}

void Camera::ProcessKeyboard(CameraDirection direction, float dt)
{
	// Camera Movement (Walk Around)
	float velocity = speed * dt;
	switch (direction)
	{
	case CameraDirection::CAMERA_FORWARD:
		position += m_Front * velocity;
		break;
	case CameraDirection::CAMERA_BACKWARD:
		position -= m_Front * velocity;
		break;
	case CameraDirection::CAMERA_LEFT:
		position -= m_Right * velocity;
		break;
	case CameraDirection::CAMERA_RIGHT:
		position += m_Right * velocity;
		break;
	case CameraDirection::CAMERA_UP:
		position += m_WorldUp * velocity;
		break;
	case CameraDirection::CAMERA_DOWN:
		position -= m_WorldUp * velocity;
		break;
	}
}

void Camera::ProcessMouse(const glm::vec2& mouseDelta)
{
	// Camera Rotation (Look Around)
	float mDx = mouseDelta.x * m_Sensitivity;
	float mDy = mouseDelta.y * m_Sensitivity;

	// Constrain yaw to only use values between 0-360 as float precision could be lost
	m_Yaw = glm::mod(m_Yaw + mDx, 360.0f);

	// Negative to invert vertical movement
	m_Pitch -= mDy;

	// Avoid locking the camera completely up or down (pitch would be put of bounds and the screen would get flipped)
	if (m_Pitch > 89.0f)
		m_Pitch = 89.0f;
	if (m_Pitch < -89.0f)
		m_Pitch = -89.0f;

	UpdateVectors();
}

void Camera::UpdateVectors()
{
	m_Front = glm::vec3(0.0f);
	m_Front.x = glm::cos(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
	m_Front.y = glm::sin(glm::radians(m_Pitch));
	m_Front.z = glm::sin(glm::radians(m_Yaw)) * glm::cos(glm::radians(m_Pitch));
	m_Front = glm::normalize(m_Front);

	m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
	m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}

void Camera::Zoom(float scrollY)
{
	FOV -= scrollY;
	FOV < 5.0f ? FOV = 5.0f : FOV;
	FOV > 45.0f ? FOV = m_DefaultFOV : FOV;
}

// Rotate Camera around target
//const float radius = 10.0f;
//position.x = sin(app->currentTime) * radius;
//position.z = cos(app->currentTime) * radius;
//view = glm::lookAt(position, target, glm::vec3(0.0f, 1.0f, 0.0f));