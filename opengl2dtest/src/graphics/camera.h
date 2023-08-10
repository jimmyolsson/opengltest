#ifndef CAMERA_H
#define CAMERA_H

// I couldn't be less interested in camera stuff so i stole this file from:
// https://learnopengl.com/Getting-started/Camera

#include "../util/common_graphics.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

enum Camera_Movement
{
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
//const float YAW = 115.0f;
const float PITCH = 0.0f;
//const float PITCH = -32.0f;
const float SPEED = 400.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 60.0f;

class Camera
{
	public:
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	float Yaw;
	float Pitch;
	float MovementSpeed;
	float MouseSensitivity;
	float Zoom;

	// constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, 1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();

		// Moves the player a very tiny bit forward. This somehow fixes a bug where the mouse is a bit off, probably something thats not initialized properly.
		// TODO: Fix the bug for real
		ProcessKeyboard(Camera_Movement::FORWARD, 0.0001f);
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}
	glm::vec3 TargetVelocity = glm::vec3(0.0f);
	glm::vec3 Velocity = glm::vec3(0.0f);
	float DampingFactor = 0.9f; // A value between 0 and 1 to control the damping

	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		float velocityValue = MovementSpeed * deltaTime;
		glm::vec3 movementDirection = glm::vec3(0.0f);

		if (direction == FORWARD) movementDirection += Front;
		if (direction == BACKWARD) movementDirection -= Front;
		if (direction == LEFT) movementDirection -= Right;
		if (direction == RIGHT) movementDirection += Right;
		if (direction == UP) movementDirection.y += 0.6;
		if (direction == DOWN) movementDirection.y -= 0.6f;

		if(direction != UP && direction != DOWN)
			movementDirection.y = 0;
		Velocity += movementDirection * velocityValue;
	}

	void Update(float deltaTime)
	{
		Velocity *= DampingFactor;

		float epsilon = 0.001f;

		if (glm::length(Velocity) < epsilon)
		{
			Velocity = glm::vec3(0.0f); // Fully stop if the velocity is very low
		}

		Position += Velocity * deltaTime;
	}

	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		updateCameraVectors();
	}

	void ProcessMouseScroll(float yoffset)
	{
		Zoom -= (float)yoffset;
		if (Zoom < 1.0f)
			Zoom = 1.0f;
		if (Zoom > ZOOM)
			Zoom = ZOOM;
	}

	private:
	void updateCameraVectors()
	{
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		Right = glm::normalize(glm::cross(Front, WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif

