#pragma once

#include <glm/glm.hpp>
#include "camera.hpp"

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
	float intensity;

	Camera light_cam;
};