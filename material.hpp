#pragma once
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "light.hpp"

struct Material {
	enum AlphaMode {
		AM_OPAQUE = 0,
		AM_MASK = 1,
		AM_BLEND = 2,
	};
	Material(AlphaMode alpha_mode = AM_OPAQUE) : alpha_mode(alpha_mode) { }
	virtual void apply(glm::mat4 model, const Camera& cam) = 0;
	AlphaMode alpha_mode = AM_OPAQUE;
};

struct SimpleColorMaterial : Material {
	SimpleColorMaterial(glm::vec3 color) : color(color) {
		if (shader_program == nullptr) {
			shader_program = load_shader();
		}
	}

	virtual void apply(glm::mat4 model, const Camera& cam) override {
		shader_program->use();

		GLuint location = glGetUniformLocation(shader_program->handle, "model");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(model));
		location = glGetUniformLocation(shader_program->handle, "view");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.view()));
		location = glGetUniformLocation(shader_program->handle, "project");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.project()));
		location = glGetUniformLocation(shader_program->handle, "color");
		glUniform3fv(location, 1, glm::value_ptr(color));
	}

	static std::shared_ptr<ShaderProgram> load_shader() {
		Shader vert("simple.vert", GL_VERTEX_SHADER);
		Shader frag("simple.frag", GL_FRAGMENT_SHADER);
		return std::make_shared<ShaderProgram>(vert, frag);
	}

	glm::vec3 color;
	static inline std::shared_ptr<ShaderProgram> shader_program = nullptr;
};

struct PhongMaterial : Material {
	float k_ambient, k_diffuse, k_specular;
	float phong_exponent;

	std::shared_ptr<ShaderProgram> shader_program;
	std::shared_ptr<Texture> texture;
	std::shared_ptr<Texture> shadow_map;
	std::shared_ptr<PointLight> light;
	static inline std::shared_ptr<ShaderProgram> default_shader_program = nullptr;

	PhongMaterial(
		std::shared_ptr<ShaderProgram> shader_program, 
		std::shared_ptr<Texture> texture, 
		std::shared_ptr<PointLight> light,
		std::shared_ptr<Texture> shadow_map = nullptr,
		AlphaMode alpha_mode = Material::AM_OPAQUE,
		float phong_exponent = 32.0f, float k_ambient = .2f, float k_diffuse = 1.0f, float k_specular = 0.5f)
	: shader_program(shader_program), texture(texture), light(light), shadow_map(shadow_map),
		phong_exponent(phong_exponent), k_ambient(k_ambient), k_diffuse(k_diffuse), k_specular(k_specular),
		Material(alpha_mode)
	{
		if (shader_program == nullptr && default_shader_program == nullptr) {
			this->shader_program = load_shader();
		}
	}

	virtual void apply(glm::mat4 model, const Camera &cam) override {
		texture->use(GL_TEXTURE0);
		if (shadow_map) {
			shadow_map->use(GL_TEXTURE1);
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
		glUniform1f(location, light->intensity);
		location = glGetUniformLocation(shader_program->handle, "material.k_ambient");
		glUniform1f(location, k_ambient);
		location = glGetUniformLocation(shader_program->handle, "material.k_diffuse");
		glUniform1f(location, k_diffuse);
		location = glGetUniformLocation(shader_program->handle, "material.k_specular");
		glUniform1f(location, k_specular);
		location = glGetUniformLocation(shader_program->handle, "material.phong_exponent");
		glUniform1f(location, phong_exponent);
		location = glGetUniformLocation(shader_program->handle, "eye");
		glUniform3fv(location, 1, glm::value_ptr(cam.pos));
		location = glGetUniformLocation(shader_program->handle, "vp_light");
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(light->light_cam.project() * light->light_cam.view()));
		location = glGetUniformLocation(shader_program->handle, "ourTexture");
		glUniform1i(location, 0);
		location = glGetUniformLocation(shader_program->handle, "shadowMap");
		glUniform1i(location, 1);
		location = glGetUniformLocation(shader_program->handle, "zNear");
		glUniform1f(location, light->light_cam.zNear);
		location = glGetUniformLocation(shader_program->handle, "zFar");
		glUniform1f(location, light->light_cam.zFar);
		location = glGetUniformLocation(shader_program->handle, "lightSize");
		glUniform1f(location, 1.0f);
		location = glGetUniformLocation(shader_program->handle, "recvShadow");
		glUniform1i(location, (shadow_map == nullptr ? 0 : 1));
	}

	static std::shared_ptr<ShaderProgram> load_shader() {
		Shader vert("shader.vert", GL_VERTEX_SHADER);
		Shader frag("shader.frag", GL_FRAGMENT_SHADER);
		return std::make_shared<ShaderProgram>(vert, frag);
	}
};
