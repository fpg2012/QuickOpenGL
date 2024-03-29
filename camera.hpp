#pragma once
#include <glm/glm.hpp>

#include "config.h"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

struct Camera {
	float fovy = glm::radians(45.0f);
	float aspect = (float)VIEWPORT_WIDTH / VIEWPORT_HEIGHT;
	glm::vec3 pos = glm::vec3(.0f, 3.0f, 3.0f);
	glm::vec3 up = glm::vec3(.0f, 1.0f, .0f);
	glm::vec3 look_at = glm::vec3(.0f, .0f, .0f);
	float zNear = 1.0f;
	float zFar = 100.0f;

	glm::mat4 project() const {
		return glm::perspective(fovy, aspect, zNear, zFar);
	}

	glm::mat4 view() const {
		return glm::lookAt(pos, look_at, up);
	}
};