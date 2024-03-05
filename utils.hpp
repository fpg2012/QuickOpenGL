#pragma once
#include <glm/glm.hpp>

struct Point3f {
	float x, y, z;
};

struct PointLight {
	glm::vec3 position;
	glm::vec3 color;
	float intensity;
};

struct PhongMaterial {
	float k_ambient, k_diffuse, k_specular;
	float phong_exponent;
};