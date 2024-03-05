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
#include "utils.hpp"


class ShapeObject { 
public:
	std::shared_ptr<ShaderProgram> shader_program = nullptr;
	glm::mat4 model = glm::mat4(1.0f); 
	std::shared_ptr<Texture> texture = nullptr;

	ShapeObject(std::shared_ptr<ShaderProgram> shader = nullptr, std::shared_ptr<Texture> texture = nullptr) : shader_program(shader), texture(texture) {

	}
	virtual void draw(const Camera &cam) {

	} 
};

class Triangle : public ShapeObject {
public:
	std::array<float, 9> vertices; 
	GLuint vbo, vao;

	Triangle(const std::array<Point3f, 3>& points, std::shared_ptr<ShaderProgram> shader) : ShapeObject(shader) {
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

	void draw(const Camera& cam) override {
		// location, size, type, normalize, stride, offset
		assert(shader_program != nullptr);
		shader_program->use();
		GLuint location = glGetUniformLocation(shader_program->handle, "model");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
		location = glGetUniformLocation(shader_program->handle, "view");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.view()));
		location = glGetUniformLocation(shader_program->handle, "project");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.project()));
		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
};

class Sphere : public ShapeObject {
public:
	std::vector<float> vertices;
	std::vector<GLuint> indices;
	GLuint vbo, vao, ebo;
	std::shared_ptr<PointLight> light;
	PhongMaterial material;

	Sphere(std::shared_ptr<ShaderProgram> shader, float r = 1.0f, std::shared_ptr<Texture> texture = nullptr) : ShapeObject(shader, texture) {
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
				vertices.push_back((step * i) / pi);
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

	void draw(const Camera &cam) override {
		if (texture != nullptr) {
			texture->use();
		}

		assert(shader_program != nullptr);
		shader_program->use();

		GLuint location = glGetUniformLocation(shader_program->handle, "model");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
		location = glGetUniformLocation(shader_program->handle, "view");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.view()));
		location = glGetUniformLocation(shader_program->handle, "project");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.project()));
		location = glGetUniformLocation(shader_program->handle, "light.position");
		glUniform3fv(location, 1, glm::value_ptr(light->position));
		location = glGetUniformLocation(shader_program->handle, "light.color");
		glUniform3fv(location, 1, glm::value_ptr(light->color));
		location = glGetUniformLocation(shader_program->handle, "light.intensity");
		glUniform1fv(location, 1, &light->intensity);
		location = glGetUniformLocation(shader_program->handle, "material.k_ambient");
		glUniform1fv(location, 1, &material.k_ambient);
		location = glGetUniformLocation(shader_program->handle, "material.k_diffuse");
		glUniform1fv(location, 1, &material.k_diffuse);
		location = glGetUniformLocation(shader_program->handle, "material.k_specular");
		glUniform1fv(location, 1, &material.k_specular);
		location = glGetUniformLocation(shader_program->handle, "material.phong_exponent");
		glUniform1fv(location, 1, &material.phong_exponent);
		location = glGetUniformLocation(shader_program->handle, "eye");
		glUniform3fv(location, 1, glm::value_ptr(cam.pos));

		glBindVertexArray(vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	}
};