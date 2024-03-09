#pragma once
#include <iostream>
#include <algorithm>
#include <glad/glad.h>

#include "light.hpp"
#include "utils.hpp"
#include "shader.hpp"
#include "shape.hpp"
#include "camera.hpp"
#include "material.hpp"

struct GLTFSceneTest {

    tinygltf::Model model;
    std::shared_ptr<ShaderProgram> shader_program;
    std::shared_ptr<PointLight> light;
    std::shared_ptr<Texture> shadow_map;
    std::vector<std::shared_ptr<Texture>> textures;
	std::vector<std::shared_ptr<Material>> materials;
    std::vector<GLuint> buffer_handles;
    GLuint vao;

    GLTFSceneTest(std::string& filename, std::shared_ptr<ShaderProgram> shader_program, std::shared_ptr<PointLight> light,  std::shared_ptr<Texture> shadow_map)
     : shader_program(shader_program), light(light), shadow_map(shadow_map)
    {
        std::string err, warn;
        tinygltf::TinyGLTF loader;
        bool result = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

	    if (!warn.empty()) {
		    std::cerr << warn << std::endl;
	    }
	    if (!err.empty()) {
		    std::cerr << err << std::endl;
	    }
	    if (!result) {
		    std::cerr << "failed to load model " << filename << std::endl;
	    }

        for (tinygltf::Texture& texture : model.textures) {
		    std::cout << "loading texture: " << texture.name << std::endl;
		    tinygltf::Image& image = model.images[texture.source];
		    tinygltf::Sampler& sampler = model.samplers[texture.sampler];

		    std::cout << "image: " << image.name << ", " << image.width << ", " << image.height << ", " << image.image.size() << std::endl;

		    auto my_texture = std::make_shared<Texture>(image.image, image.width, image.height, sampler.wrapS, sampler.wrapT, sampler.minFilter, sampler.magFilter, image.pixel_type);

		    textures.push_back(my_texture);
	    }

	    for (tinygltf::Material& mat : model.materials) {
		    int texture_index = mat.pbrMetallicRoughness.baseColorTexture.index;
		    auto texture = textures[texture_index];

		    std::cout << "load material: " << mat.name << std::endl;

		    auto my_material = std::make_shared<PhongMaterial>(shader_program, texture, light, shadow_map);
		    materials.push_back(my_material);
	    }
        
        buffer_handles.resize(model.bufferViews.size());
        std::fill(buffer_handles.begin(), buffer_handles.end(), -1);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        // bind mesh
        tinygltf::Mesh& mesh = model.meshes[4];
        std::cout << "binding mesh: " << mesh.name << std::endl;
        tinygltf::Primitive& prim = mesh.primitives[0];
        tinygltf::Accessor index_acc = model.accessors[prim.indices];
        tinygltf::BufferView& index_bufv = model.bufferViews[index_acc.bufferView];
        tinygltf::Buffer& index_buf = model.buffers[index_bufv.buffer];

        GLuint ebo;
        glGenBuffers(1, &ebo);
        buffer_handles[index_acc.bufferView] = ebo;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_bufv.byteLength, index_buf.data.data() + index_bufv.byteOffset, GL_STATIC_DRAW);
        
        for (const auto& attr : prim.attributes) {
            std::string kind = attr.first;
            int acc_id = attr.second;
            tinygltf::Accessor &acc = model.accessors[acc_id];
            tinygltf::BufferView &bufv = model.bufferViews[acc.bufferView];
            tinygltf::Buffer &buf = model.buffers[bufv.buffer];

            GLuint vaa = -1;
            if (kind == "POSITION") {
                vaa = 0;
            } else if (kind == "NORMAL") {
                vaa = 2;
            } else if (kind == "TEXCOORD_0") {
                vaa = 1;
            }

            if (vaa == -1) {
                std::cout << "invalid vaa" << std::endl;
                exit(-1);
            }
            
            GLuint vbo;
            if (buffer_handles[acc.bufferView] == -1) {
                glGenBuffers(1, &vbo);
                buffer_handles[acc.bufferView] = vbo;
                glBindBuffer(GL_ARRAY_BUFFER, vbo);
                glBufferData(GL_ARRAY_BUFFER, bufv.byteLength, buf.data.data() + bufv.byteOffset, GL_STATIC_DRAW);
            } else {
                vbo = buffer_handles[acc.bufferView];
            }
            glBindBuffer(GL_ARRAY_BUFFER, vbo);
            glEnableVertexAttribArray(vaa);
            glVertexAttribPointer(vaa, tinygltf::GetNumComponentsInType(acc.type), GL_FLOAT, (acc.normalized ? GL_TRUE : GL_FALSE), acc.ByteStride(bufv), (void*)(0 + acc.byteOffset));\
        }
    }
};