#include <iostream>
#include <stdexcept>
#include <algorithm>
#include <string>
#include <memory>
#include <chrono>

#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include "glm/matrix.hpp"

#include "gltf_scene.hpp"
#include "config.h"
#include "shader.hpp"
#include "shape.hpp"
#include "camera.hpp"
#include "light.hpp"
#include "framebuffer.hpp"
#include "skybox.hpp"

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
		if (gltf_scene == nullptr) {
			std::cerr << "no gltf scene" << std::endl;
			exit(-1);
		}

		camera = Camera{
			.fovy = glm::radians(45.0f),
			.aspect = (float)viewport_width / viewport_height,
			.pos = glm::vec3(-1.5f, 0.2f, -1.0f),
			.up = glm::vec3(.0f, 1.0f, .0f),
		};

		auto light = std::make_shared<PointLight>(PointLight{
			.position = glm::vec3(.0f, 5.0f, 3.0f),
			.color = glm::vec3(1.0f),
			.intensity = 30.0f,
		});

		Camera light_camera{
			.fovy = glm::radians(20.0f),
			.aspect = (float)shadow_width / shadow_height,
			.pos = light->position,
			.up = glm::vec3(.0f, 1.0f, .0f),
			.look_at = glm::vec3(.0f),
		};

		light->light_cam = light_camera;

		auto tex_earth = std::make_shared<Texture>("resource/earth2048.bmp");
		auto tex_moon = std::make_shared<Texture>("resource/moon1024.bmp");

		auto shader = create_shader_program("shader.vert", "shader.frag");

		Framebuffer depth_frame_buf;
		depth_frame_buf.bind();
		auto shadow_map = std::make_shared<Texture>(depth_frame_buf.handle, shadow_width, shadow_height);
		// A framebuffer object however is not complete without a color buffer so we need to explicitly tell OpenGL we're not going to render any color data
		depth_frame_buf.unbind();

		auto mat_earth = std::make_shared<PhongMaterial>(shader, tex_earth, light, shadow_map);
		auto mat_moon = std::make_shared<PhongMaterial>(shader, tex_moon, light, shadow_map);
		auto mat_simple = std::make_shared<SimpleColorMaterial>(glm::vec3(1.0, 1.0, 1.0));

		gltf_scene->init(shader, light, shadow_map);

		Sphere sph = Sphere(1.0f);
		Sphere sph2 = Sphere(0.1f);
		Sphere light_sph = Sphere(0.1f);

		SkyBox skybox({
			"resource/cloudy_0/bluecloud_ft.jpg",
			"resource/cloudy_0/bluecloud_bk.jpg",
			"resource/cloudy_0/bluecloud_dn.jpg",
			"resource/cloudy_0/bluecloud_up.jpg",
			"resource/cloudy_0/bluecloud_rt.jpg",
			"resource/cloudy_0/bluecloud_lf.jpg",
		});

		auto start_time = std::chrono::high_resolution_clock::now();

		glm::vec4 temp(light->position[0], light->position[1], light->position[2], 1.0f);
		glm::vec4 cam_pos_ori(camera.pos, 1.0f);

		gltf_scene->update_matrix(
			glm::scale(glm::mat4(1.0f), glm::vec3(10.0f, 10.0f, 10.0f))
		);

		glEnable(GL_DEPTH_TEST);

		while (!glfwWindowShouldClose(window))
		{
			auto now_time = std::chrono::high_resolution_clock::now();
			float t = std::chrono::duration_cast<std::chrono::duration<float>>(now_time - start_time).count();

			// glm::vec4 temp2 = temp;
			// temp2 = glm::rotate(glm::mat4(1.0f), glm::sin(t) / 4, glm::vec3(0, 0, -1.0f)) * temp;
			// light->position[0] = temp2[0] / temp2.w;
			// light->position[1] = temp2[1] / temp2.w;
			// light->position[2] = temp2[2] / temp2.w;
			// light->light_cam.pos = light->position;

			/*glm::vec4 cam_pos_temp = cam_pos_ori;
			cam_pos_temp = glm::rotate(glm::mat4(1.0f), 0.4f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f)) * cam_pos_temp;
			camera.pos[0] = cam_pos_temp[0] / cam_pos_temp.w;
			camera.pos[1] = cam_pos_temp[1] / cam_pos_temp.w;
			camera.pos[2] = cam_pos_temp[2] / cam_pos_temp.w;*/

			// sph.model = glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			// sph2.model = glm::rotate(glm::mat4(1.0f), 6.0f * t / (2.0f * 3.14f), glm::vec3(.0f, .0f, 1.0f))
			// 	* glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 2.0, .0f))
			// 	* glm::rotate(glm::mat4(1.0f), 3.0f * t / (2.0f * 3.14f), glm::vec3(.0f, 1.0f, .0f));
			// light_sph.model = glm::translate(glm::mat4(1.0f), light->position);

			glDepthMask(GL_TRUE);
			glViewport(0, 0, shadow_width, shadow_height);
			depth_frame_buf.bind();
			glClear(GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_FRONT);
			// draw
			// sph.draw(light->light_cam, mat_simple);
			// sph2.draw(light->light_cam, mat_simple);
			gltf_scene->render(light->light_cam, mat_simple);
			GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
			if (status != GL_FRAMEBUFFER_COMPLETE) {
				std::cerr << "fb error: " << status << std::endl;
			}
			glBindFramebuffer(GL_FRAMEBUFFER, 0); // unbind

			glViewport(0, 0, viewport_width, viewport_height);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			glCullFace(GL_BACK);
			// sph.draw(camera, mat_earth);
			// sph2.draw(camera, mat_moon);
			// light_sph.draw(light->light_cam, mat_simple);
			
			skybox.draw(camera);
			gltf_scene->render(camera);

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

	void load_scene(const std::string &filename) {
		gltf_scene = std::make_shared<GLTFScene>(filename);
	}

	static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
		app->camera.aspect = static_cast<float>(width) / height;
		app->viewport_width = width;
		app->viewport_height = height;
		std::cout << "resize: " << width << ", " << height << std::endl;
	}

	static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
		if (yoffset < 0) {
			app->camera.fovy = std::min(glm::radians(87.0f), app->camera.fovy + 0.1f);
		}
		else if (yoffset > 0) {
			app->camera.fovy = std::max(glm::radians(3.0f), app->camera.fovy - 0.1f);
		}
	}

	static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
    	if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			glm::vec3 delta = glm::vec3(.0f, .0f, -0.1f);
			app->camera.pos += glm::transpose(glm::mat3(app->camera.view())) * delta;
		} else if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			glm::vec3 delta = glm::vec3(.0f, .0f, 0.1f);
			app->camera.pos += glm::transpose(glm::mat3(app->camera.view())) * delta;
		} else if (key == GLFW_KEY_ESCAPE) {
			exit(0);
		}
	}

	static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
	{
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
		if (app->left_pressed) {
			float delta_xpos = xpos - app->xpos;
			glm::mat3 rotate = glm::mat3(glm::rotate(glm::mat4(1.0f), -delta_xpos / app->viewport_width * 2, glm::vec3(.0f, 1.0f, .0f)));
			app->camera.pos = rotate * app->old_cam.pos;
		} else if (app->middle_pressed) {
			// screen to camera
			glm::vec3 move_camera_space((xpos - app->xpos) / app->viewport_width * 2, -(ypos - app->ypos) / app->viewport_height * 2, .0f);
			glm::vec3 move_ndc_space = glm::transpose(glm::mat3(app->old_cam.view())) * move_camera_space;

			app->camera.look_at = app->old_cam.look_at - move_ndc_space;
			app->camera.pos = app->old_cam.pos - move_ndc_space;
		} else {
			app->xpos = xpos;
			app->ypos = ypos;
		}
	}

	static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
	{
		auto app = reinterpret_cast<QuickGLApplication*>(glfwGetWindowUserPointer(window));
    	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
			app->left_pressed = true;
			app->old_cam = app->camera;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
			app->middle_pressed = true;
			app->old_cam = app->camera;
		}

		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
			app->left_pressed = false;
		}
		if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
			if (app->middle_pressed) {
				app->middle_pressed = false;
			}
		}
	}

private:
	GLFWwindow* window;
	Camera camera;
	GLuint viewport_width = 1080, viewport_height = 720;
	GLuint shadow_width = 2048, shadow_height = 2048;
	std::shared_ptr<GLTFScene> gltf_scene = nullptr;
	bool left_pressed = false, middle_pressed = false;
	Camera old_cam;
	double xpos, ypos;

	void _init_glfw() {
		if (!glfwInit()) {
			throw std::runtime_error("failed to initialize GLFW");
		}
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		// glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
		glfwWindowHint(GLFW_SAMPLES, 4);
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
		glfwSetScrollCallback(window, QuickGLApplication::scroll_callback);
		glfwSetCursorPosCallback(window, QuickGLApplication::cursor_position_callback);
		glfwSetMouseButtonCallback(window, QuickGLApplication::mouse_button_callback);
		glfwSetKeyCallback(window, QuickGLApplication::key_callback);

		glViewport(0, 0, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
		// glClearColor(0.025f, 0.05f, 0.1f, 1.0f);
		glClearColor(0.27f, 0.27f, 0.3f, 1.0f);
		// glEnable(GL_MULTISAMPLE);
		glfwSwapInterval(1);

		// GLint flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
		// if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
		// {
    	// 	// 初始化调试输出 
		// 	std::cout << "init debug context" << std::endl;
		// }

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  
        // glEnable(GL_FRAMEBUFFER_SRGB);
	}
};

int main(int argc, char **argv)
{
	std::string filename = "resource/forest_house/scene.gltf";
	if (argc >= 2) {
		filename = argv[1];
	}
	QuickGLApplication app;
	app.load_scene(filename);
	app.main_loop();
	return 0;
}
