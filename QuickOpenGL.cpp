#include <iostream>
#include <stdexcept>
#include <array>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <memory>
#include <cassert>
#include <chrono>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <glm/glm.hpp>

#include "config.h"
#include "shader.hpp"
#include "shape.hpp"
#include "camera.hpp"
#include "light.hpp"

class QuickGLApplication {
public: 
	QuickGLApplication() { 
		_init_glfw();
	}

	~QuickGLApplication() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void main_loop() {
		camera = Camera{
			.fovy = glm::radians(45.0f),
			.aspect = (float)viewport_width / viewport_height,
			.pos = glm::vec3(-3.0f, .5f, 3.0f),
			.up = glm::vec3(.0f, 1.0f, .0f),
		};

		auto light = std::make_shared<PointLight>(PointLight{
			.position = glm::vec3(-5.0f, .0f, .0f),
			.color = glm::vec3(1.0f),
			.intensity = 20.0f,
		});

		Camera light_camera{
			.fovy = glm::radians(60.0f),
			.aspect = (float)shadow_width / shadow_height,
			.pos = light->position,
			.up = glm::vec3(.0f, 1.0f, .0f),
			.look_at = glm::vec3(.0f),
		};

		light->light_cam = light_camera;

		auto tex_earth = std::make_shared<Texture>("resource/earth2048.bmp");
		auto tex_moon = std::make_shared<Texture>("resource/moon1024.bmp");

		auto shader = create_shader_program("shader.vert", "shader.frag");

		auto mat_earth = std::make_shared<PhongMaterial>(shader, tex_earth, light);
		auto mat_moon = std::make_shared<PhongMaterial>(shader, tex_moon, light);
		auto mat_simple = std::make_shared<SimpleColorMaterial>(glm::vec3(1.0, 1.0, 1.0));

		Sphere sph = Sphere(1.0f);
		Sphere sph2 = Sphere(0.1f);
		Sphere light_sph = Sphere(0.1f);

		auto start_time = std::chrono::high_resolution_clock::now();

		GLuint depth_map_fbo;
		glGenFramebuffers(1, &depth_map_fbo);
		auto shadow_map = std::make_shared<Texture>(depth_map_fbo, shadow_width, shadow_height);
		mat_earth->shadow_map = shadow_map;
		mat_moon->shadow_map = shadow_map;

		// A framebuffer object however is not complete without a color buffer so we need to explicitly tell OpenGL we're not going to render any color data
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		glEnable(GL_DEPTH_TEST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

		glm::vec4 temp(light->position[0], light->position[1], light->position[2], 1.0f);
		glm::vec4 cam_pos_ori(camera.pos, 1.0f);

		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);

		while (!glfwWindowShouldClose(window))
		{
			auto now_time = std::chrono::high_resolution_clock::now();
			float t = std::chrono::duration_cast<std::chrono::duration<float>>(now_time - start_time).count();

			glm::vec4 temp2 = temp;
			temp2 = glm::rotate(glm::mat4(1.0f), glm::sin(t) / 4, glm::vec3(0, 0, -1.0f)) * temp;
			light->position[0] = temp2[0] / temp2.w;
			light->position[1] = temp2[1] / temp2.w;
			light->position[2] = temp2[2] / temp2.w;
			light->light_cam.pos = light->position;

			/*glm::vec4 cam_pos_temp = cam_pos_ori;
			cam_pos_temp = glm::rotate(glm::mat4(1.0f), 0.4f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f)) * cam_pos_temp;
			camera.pos[0] = cam_pos_temp[0] / cam_pos_temp.w;
			camera.pos[1] = cam_pos_temp[1] / cam_pos_temp.w;
			camera.pos[2] = cam_pos_temp[2] / cam_pos_temp.w;*/

			sph.model = glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			sph2.model = glm::rotate(glm::mat4(1.0f), 2.0f * 15.0f / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f))
				* glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 2.0f))
				* glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			light_sph.model = glm::translate(glm::mat4(1.0f), light->position);

			glViewport(0, 0, shadow_width, shadow_height);
			glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
			glClear(GL_DEPTH_BUFFER_BIT);
			// draw
			sph.draw(light->light_cam, mat_simple);
			sph2.draw(light->light_cam, mat_simple);
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "fb error: " << status << std::endl;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

			glViewport(0, 0, viewport_width, viewport_height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			
			sph.draw(camera, mat_earth);
			sph2.draw(camera, mat_moon);
			light_sph.draw(camera, mat_simple);
			glfwSwapBuffers(window);
			glfwPollEvents(); 
		}
	}

	std::shared_ptr<ShaderProgram> create_shader_program(const char* vert_shader, const char* frag_shader) {
		Shader vert(vert_shader, GL_VERTEX_SHADER);
		Shader frag(frag_shader, GL_FRAGMENT_SHADER);

		auto shader = std::make_shared<ShaderProgram>(vert, frag);
		return shader;
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
		app->camera.aspect = static_cast<float>(width) / height;
		app->viewport_width = width;
		app->viewport_height = height;
		std::cout << "resize: " << width << ", " << height << std::endl;
	}
private:
	GLFWwindow* window;
	Camera camera;
	GLuint viewport_width = 1920, viewport_height = 1080;
	GLuint shadow_width = 3072, shadow_height = 3072;

	void _init_glfw() {
		if (!glfwInit()) {
			throw std::runtime_error("failed to initialize GLFW");
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_SAMPLES, 8);
		window = glfwCreateWindow(viewport_width, viewport_height, "QuickOpenGL", NULL, NULL);
		glfwSetWindowUserPointer(window, this);
		if (!window) {
			throw std::runtime_error("failed to create window"); 
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			throw std::runtime_error("failed to initialize GLAD");
		}

		glfwSetFramebufferSizeCallback(window, QuickGLApplication::framebuffer_size_callback);
		glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
		glClearColor(0.025f, 0.05f, 0.1f, 1.0f);
		glEnable(GL_MULTISAMPLE);
	}
};

int main()
{
	QuickGLApplication app;
	app.main_loop();
	return 0;
}
