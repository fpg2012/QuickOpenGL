#pragma once

#include <glad/glad.h>
#include <initializer_list>
#include <memory>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "texture.hpp"
#include "camera.hpp"

struct SkyBox {

    // right, left, up, down, back, front
    SkyBox(std::array<const char*, 6> &&filenames) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);

        Shader vert(vertex_shader, GL_VERTEX_SHADER, 0);
        Shader frag(fragment_shader, GL_FRAGMENT_SHADER, 0);
        shader_program = std::make_shared<ShaderProgram>(vert, frag);
        texture = std::make_shared<CubeMapTexture>(std::move(filenames));
    }

    void draw(const Camera &cam) {
        shader_program->use();
        GLuint location = glGetUniformLocation(shader_program->handle, "rotation");
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(cam.project() * cam.view()));
        location = glGetUniformLocation(shader_program->handle, "tex");
        glUniform1i(location, 0);
        texture->use();

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    GLuint vao, vbo;
    std::shared_ptr<ShaderProgram> shader_program = nullptr;
    std::shared_ptr<CubeMapTexture> texture = nullptr;

    const char *vertex_shader = R"(
        #version 430 core
        layout (location = 0) in vec3 aPos;
        out vec3 texCoord;

        uniform mat4 rotation;

        void main() {
            texCoord = aPos;
            float scale = 10.0;
            gl_Position = rotation * mat4(scale, 0.0, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 0.0, scale, 0.0, 0.0, 0.0, 0.0, 1.0) * vec4(aPos, 1.0);
        }
    )";

    const char *fragment_shader = R"(
        #version 430 core
        in vec3 texCoord;
        out vec4 color;

        uniform samplerCube tex;

        void main() {
            color = texture(tex, texCoord);
        }
    )";

    static constexpr float skyboxVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};
};