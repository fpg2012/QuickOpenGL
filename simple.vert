#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
out vec2 texCoord;
uniform mat4 model;
uniform mat4 view;
uniform mat4 project;

void main() {
	gl_Position = project * view * model * vec4(aPos, 1.0f);
	texCoord = aTexCoord;
}