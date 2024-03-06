#version 430 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
uniform mat4 model;
uniform mat4 view;
uniform mat4 project;
uniform mat4 vp_light;
out vec3 pos;
out vec4 pos_light_space;
out vec2 texCoord;
out vec3 normal;

void main() {
	vec4 temp_pos = model * vec4(aPos, 1.0f);
	vec3 center = (model * vec4(.0f, .0f, .0f, 1.0f)).xyz;
	gl_Position = project * view * temp_pos;
	pos = temp_pos.xyz / temp_pos.w;
	texCoord = aTexCoord;
	normal = normalize((model * vec4(aNormal, 1.0f)).xyz - center);
	
	pos_light_space = vp_light * model * vec4(aPos, 1.0f);
}