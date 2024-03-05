#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <glad/glad.h>

class Shader {
public:
	GLuint handle;
	std::string source;
	//char *source = nullptr;
	GLint success;
	size_t len = 0;
	char info_log[1024];

	Shader(const char* filename, GLenum type) {
		std::ifstream ifs(filename, std::ios::ate);
		if (!ifs.is_open()) {
			throw std::runtime_error("failed to open file");
		}
		size_t len = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		//ifs.read(source, len);
		std::stringstream ss;
		ss << ifs.rdbuf();
		source = ss.str();
		ifs.close();

		std::cerr << std::endl << filename << ", length: " << len << std::endl << source << std::endl;

		const char* source_c = source.c_str();
		init(source_c, type);
	}

	~Shader() {
		if (success == GL_TRUE) {
			glDeleteShader(handle);
		}
	}
private:
	void init(const char*& source, GLenum type) {
		handle = glCreateShader(type);

		glShaderSource(handle, 1, &source, NULL);
		glCompileShader(handle);

		glGetShaderiv(handle, GL_COMPILE_STATUS, &success);

		if (success != GL_TRUE) {
			glGetShaderInfoLog(handle, 1024, NULL, info_log);
			std::cerr << "failed to compile shader: " << std::endl;
			std::cerr << info_log << std::endl;
		}
	}
};

class ShaderProgram {
public:
	GLuint handle;
	GLint success;

	ShaderProgram(const Shader& vert, const Shader& frag) {
		handle = glCreateProgram();
		glAttachShader(handle, vert.handle);
		glAttachShader(handle, frag.handle);
		glLinkProgram(handle);
		glGetProgramiv(handle, GL_LINK_STATUS, &success);
		if (!success) {
			char info_log[1024];
			glGetProgramInfoLog(handle, 1024, NULL, info_log);
			std::cerr << info_log << std::endl;
		}
	}

	void use() {
		glUseProgram(handle);
	}
};