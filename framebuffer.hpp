#pragma once

#include <glad/glad.h>

struct Framebuffer {
    GLuint handle; // fbo

    Framebuffer(bool enable_depth_test = true, GLenum draw_buffer = GL_NONE, GLenum read_buffer = GL_NONE) {
        glGenFramebuffers(1, &handle);
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
        glDrawBuffer(draw_buffer);
        glReadBuffer(read_buffer);
        if (enable_depth_test) {
            glEnable(GL_DEPTH_TEST);
        } else {
            glDisable(GL_DEPTH_TEST);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
    
    void bind() {
        glBindFramebuffer(GL_FRAMEBUFFER, handle);
    }

    void unbind() {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};