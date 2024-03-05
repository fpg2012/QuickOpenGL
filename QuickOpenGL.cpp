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
#include "utils.hpp"
#include "shader.hpp"
#include "shape.hpp"
#include "camera.hpp"

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
		Sphere sph = create_sphere(1.0f, "resource/earth2048.bmp");
		Sphere sph2 = create_sphere(0.1f, "resource/moon1024.bmp");
		auto start_time = std::chrono::high_resolution_clock::now();

		auto light = std::make_shared<PointLight>(PointLight{
			.position = glm::vec3(-50.0f, .0f, .0f),
			.color = glm::vec3(1.0f),
			.intensity = 5000.0f,
		});

		glm::vec4 temp(light->position[0], light->position[1], light->position[2], 1.0f);
		temp = camera.view() * temp;
		light->position[0] = temp[0];
		light->position[1] = temp[1];
		light->position[2] = temp[2];
		
		PhongMaterial material{
			.k_ambient = 0.25f,
			.k_diffuse = 1.0f,
			.k_specular = 0.5f,
			.phong_exponent = 32.0f,
		};

		sph.light = light;
		sph.material = material;
		sph2.light = light;
		sph2.material = material;

		glCullFace(GL_BACK);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);

		while (!glfwWindowShouldClose(window))
		{
			auto now_time = std::chrono::high_resolution_clock::now();
			float t = std::chrono::duration_cast<std::chrono::duration<float>>(now_time - start_time).count();

			glm::vec4 temp2 = temp;
			temp2 = glm::rotate(glm::mat4(1.0f), glm::sin(t) / 2, glm::vec3(0, 0, -1.0f)) * temp;
			light->position[0] = temp2[0];
			light->position[1] = temp2[1];
			light->position[2] = temp2[2];

			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			sph.model = glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			sph2.model = glm::rotate(glm::mat4(1.0f), 6.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f))
				         * glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 2.0))
				         * glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			
			sph.draw(camera);
			sph2.draw(camera);
			glfwSwapBuffers(window);
			glfwPollEvents(); 
		}
	}

	Triangle create_triangle() {
		Point3f a{ .x = .0f, .y = .0f, .z = .0f };
		Point3f b{ .x = .0f, .y = .5f, .z = .0f };
		Point3f c{ .x = .5f, .y = .0f, .z = .0f };

		Shader vert("shader.vert", GL_VERTEX_SHADER);
		Shader frag("shader.frag", GL_FRAGMENT_SHADER); 

		auto shader = std::make_shared<ShaderProgram>(vert, frag);

		return Triangle({ a, b, c }, shader);
	}

	Sphere create_sphere(float radius = 1.0f, const char *texture_file = nullptr) {
		Shader vert("shader.vert", GL_VERTEX_SHADER);
		Shader frag("shader.frag", GL_FRAGMENT_SHADER);

		auto shader = std::make_shared<ShaderProgram>(vert, frag);

		std::shared_ptr<Texture> texture = nullptr;
		if (texture_file) {
			texture = std::make_shared<Texture>(texture_file);
		}

		return Sphere(shader, radius, texture);
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
		app->camera.aspect = static_cast<float>(width) / height;
		std::cout << "resize: " << width << ", " << height << std::endl;
	}
private:
	GLFWwindow* window;
	Camera camera;

	void _init_glfw() {
		if (!glfwInit()) {
			throw std::runtime_error("failed to initialize GLFW");
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		window = glfwCreateWindow(VIEWPORT_WIDTH, VIEWPORT_HEIGHT, "QuickOpenGL", NULL, NULL);
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
	}
};

int main()
{
	QuickGLApplication app;
	app.main_loop();
	return 0;
}
