
#pragma once

#include <glm.hpp>
#include "..\..\common.hpp"

struct cam_3d {
	glm::vec3 pos, front, up, right, globalUp;
	float pitch, yaw, speed;
	float lastUpdate;

	glm::mat4 getView() {
		glm::mat4 ret = lookAt(pos, pos + front, up);
		return ret;
	}

	void updateFront() {
		front.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		front = normalize(front);
		right = normalize(cross(front, globalUp));
		up = normalize(cross(right, front));
	}

	void reset() {
		pitch = -45.0f;
		yaw = 225.0f;
		speed = 0.5f;
		pos = glm::vec3(5, 5, 5);
		globalUp = glm::vec3(0, 1, 0);
		lastUpdate = (float)glfwGetTime();
		updateFront();
	}

	void move(double dx, double dy) {
		const float sens = 0.1f;
		yaw += (float)dx * sens;
		pitch -= (float)dy * sens;
		if (yaw > 360.0f) yaw = 0.0f;
		else if (yaw < 0.0f) yaw = 360.0f;
		if (pitch > 89.0f) pitch = 89.0f;
		else if (pitch < -89.0f) pitch = -89.0f;
		updateFront();
	}
};

