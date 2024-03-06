#pragma once
#include <array>
#include <cassert>
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"
#include "material.hpp"
#include "utils.hpp"


class ShapeObject { 
public:
	glm::mat4 model = glm::mat4(1.0f);

	ShapeObject() {

	}
	virtual void draw(const Camera &cam, std::shared_ptr<Material> material) {

	} 
};

class Triangle : public ShapeObject {
public:
	std::array<float, 9> vertices; 
	GLuint vbo, vao;

	Triangle(const std::array<Point3f, 3>& points) {
		for (int i = 0; i < 3; ++i) {
			vertices[i * 3] = points[i].x;
			vertices[i * 3 + 1] = points[i].y;
			vertices[i * 3 + 2] = points[i].z;
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
	}

	void draw(const Camera& cam, std::shared_ptr<Material> material) override {
		material->apply(model, cam);
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
};

class Sphere : public ShapeObject {
public:
	std::vector<float> vertices;
	std::vector<GLuint> indices;
	GLuint vbo, vao, ebo;

	Sphere(float r = 1.0f) {
		float pi = glm::pi<float>();
		int m_step = 50;
		float step = 2 * pi / m_step;
		for (int i = 0; i <= m_step / 2; ++i) {
			float y = r * glm::cos(step * i);
			if (i == 0) {
				y = r;
			}
			else if (i == m_step / 2) {
				y = -r;
			}
			float rho = r * glm::sin(step * i);
			for (int j = 0; j <= m_step; ++j) {
				float theta = step * j;
				float x = rho * glm::cos(theta);
				float z = -rho * glm::sin(theta);
				if (i == 0 || i == m_step / 2) {
					x = .0f;
					z = .0f; 
				}
				vertices.push_back(x);
				vertices.push_back(y);
				vertices.push_back(z);
				vertices.push_back((float)j / m_step);
				vertices.push_back(1.0f - (step * i) / pi);
				vertices.push_back(x/r);
				vertices.push_back(y/r);
				vertices.push_back(z/r);
				if (j == m_step) {
					continue;
				}
				if (1 <= i && i < m_step/2) {
					indices.push_back((i) * (m_step + 1) + j);
					indices.push_back((i - 1) * (m_step + 1) + j);
					indices.push_back((i) * (m_step + 1) + (j + 1));
					
					indices.push_back((i - 1) * (m_step + 1) + j);
					indices.push_back((i - 1) * (m_step + 1) + (j + 1));
					indices.push_back((i) * (m_step + 1) + (j + 1));
				}
			}
		}

		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &ebo);

		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices[0]) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices[0]) * indices.size(), indices.data(), GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		glEnableVertexAttribArray(2);
	}

	void draw(const Camera &cam, std::shared_ptr<Material> material) override {
		material->apply(model, cam);

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
};