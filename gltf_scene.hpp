#pragma once
#include <iostream>
#include <algorithm>
#include <glad/glad.h>
#include <deque>
#include <memory>

#include "glm/ext/matrix_transform.hpp"
#include "utils.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "material.hpp"

struct GLTFScene;
struct GLTFBufferView;
struct GLTFPrimitive;
struct GLTFMesh;
struct GLTFRenderQueue;

struct GLTFRenderRequest {
	const Camera* cam;
	glm::mat4 transform;
	std::shared_ptr<Material> material;
	std::shared_ptr<GLTFPrimitive> primitive;
	float z = .0f;

	bool operator<(const GLTFRenderRequest& req) {
		return z > req.z;
	}
};

struct GLTFRenderQueue {
public:
	static GLTFRenderQueue* getInstance();
	void render();
	void push(GLTFRenderRequest&& request);
private:
	explicit GLTFRenderQueue() = default;
	static inline GLTFRenderQueue *instance = nullptr;
	std::deque<GLTFRenderRequest> opaque_queue;
	std::deque<GLTFRenderRequest> blend_queue;
};

struct GLTFBufferView {
	GLuint handle;
	tinygltf::BufferView &bufv;

	GLTFBufferView(tinygltf::BufferView& view, tinygltf::Buffer& buffer);
	void bind();
};

struct GLTFPrimitive {
	std::shared_ptr<Material> default_material = nullptr;
	GLuint mode;
	std::shared_ptr<GLTFBufferView> index_bufv, pos_bufv, normal_bufv, texcoord_bufv;
	tinygltf::Accessor* pos_acc;
	tinygltf::Accessor* normal_acc;
	tinygltf::Accessor* texcoord_acc;
	tinygltf::Accessor* index_acc;
	GLuint vao;

	GLTFPrimitive(tinygltf::Model& model, tinygltf::Primitive& primitive, GLTFScene* scene);
	void draw(const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material = nullptr);
};

struct GLTFMesh {
	std::vector<std::shared_ptr<GLTFPrimitive>> primitives;

	GLTFMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, GLTFScene* scene);
	void draw(const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material = nullptr);
};

struct GLTFScene {

	GLTFScene(const std::string& filename);

	void init(std::shared_ptr<ShaderProgram> shader_program, std::shared_ptr<PointLight> light, std::shared_ptr<Texture> shadow_map = nullptr);
	void render(const Camera& cam, std::shared_ptr<Material> material = nullptr);
	void draw_node(tinygltf::Node& node, const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material = nullptr);
	void update_matrix(glm::mat4&& mat);

	tinygltf::Model model;
	std::string err;
	std::string warn;
	std::vector<std::shared_ptr<Texture>> textures;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<GLTFBufferView>> bufferViews;
	std::vector <std::shared_ptr<GLTFMesh>> meshes;
	glm::mat4 matrix = glm::mat4(1.0);
};

//// GLTFScene
GLTFScene::GLTFScene(const std::string& filename) {
	tinygltf::TinyGLTF loader;
	stbi_set_flip_vertically_on_load(false);
	// bool result = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	bool result = false;
	if (filename.ends_with(".gltf")) {
		result = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
	} else if (filename.ends_with(".glb")) {
		result = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
	}

	if (!warn.empty()) {
		std::cerr << warn << std::endl;
	}
	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	if (!result) {
		std::cerr << "failed to load model " << filename << std::endl;
	}
}

void GLTFScene::init(std::shared_ptr<ShaderProgram> shader_program, std::shared_ptr<PointLight> light, std::shared_ptr<Texture> shadow_map) {
	tinygltf::Scene scene = model.scenes[model.defaultScene];
	
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
		Material::AlphaMode alpha_mode = Material::OPAQUE;
		if (mat.alphaMode == "BLEND") {
			alpha_mode = Material::BLEND;
		} else {
			std::cout << "alpha mode not support: " << mat.alphaMode << std::endl;
		}
		std::cout << "alpha mode: " << mat.alphaMode << ", " << alpha_mode << std::endl;
		auto texture = textures[texture_index];

		std::cout << "load material: " << mat.name << std::endl;

		auto my_material = std::make_shared<PhongMaterial>(shader_program, texture, light, shadow_map, alpha_mode);
		materials.push_back(my_material);
	}

	for (tinygltf::BufferView& view : model.bufferViews) {
		auto buf = std::make_shared<GLTFBufferView>(view, model.buffers[view.buffer]);
		std::cout << "load bufferview: " << model.buffers[view.buffer].data.size() << ", uri: " << model.buffers[view.buffer].uri << ", offset: " << view.byteOffset << std::endl;
		bufferViews.push_back(buf);
	}

	for (tinygltf::Mesh& mesh : model.meshes) {
		auto my_mesh = std::make_shared<GLTFMesh>(model, mesh, this);
		std::cout << "load mesh: " << mesh.name << std::endl;
		meshes.push_back(my_mesh);
	}
}

void GLTFScene::render(const Camera& cam, std::shared_ptr<Material> material) {
	tinygltf::Scene scene = model.scenes[model.defaultScene];
	for (int node : scene.nodes) {
		tinygltf::Node& n = model.nodes[node];
		draw_node(n, cam, matrix, material);
	}
	auto request_queue = GLTFRenderQueue::getInstance();
	request_queue->render();
}

void GLTFScene::draw_node(tinygltf::Node& node, const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material) {
	// std::vector<float> temp;
	glm::mat4 matrix(1.0f);

	if (node.matrix.size() == 16) {
		matrix = glm::make_mat4(node.matrix.data());
	} else {
		if (node.scale.size() == 3) {
			glm::mat4 scale = glm::scale(glm::mat4(1.0), glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
			matrix = scale * matrix;
		}
		if (node.rotation.size() == 4) {
			glm::quat qua(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
			glm::mat4 rotate = glm::mat4_cast(qua);
			matrix = rotate * matrix;
		}
		if (node.translation.size() == 3) {
			glm::mat4 translate = glm::translate(glm::mat4(1.0), glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
			matrix = translate * matrix;
		}
	}
	glm::mat4 real_transform = transform * matrix;

	if (node.mesh != -1) {
		auto mesh_ref = meshes[node.mesh];
		mesh_ref->draw(cam, real_transform, material);
		return;
	}
	
	for (int child : node.children) {
		tinygltf::Node& next_node = model.nodes[child];
		draw_node(next_node, cam, real_transform, material);
	}
}

void GLTFScene::update_matrix(glm::mat4&& transform) {
	matrix = transform;
}

//// GLTFBufferView
GLTFBufferView::GLTFBufferView(tinygltf::BufferView& view, tinygltf::Buffer& buffer) : bufv(view) {
	glGenBuffers(1, &handle);
	glBindBuffer(view.target, handle);

	glBufferData(view.target, view.byteLength, buffer.data.data() + view.byteOffset, GL_STATIC_DRAW);
}

void GLTFBufferView::bind() {
	glBindBuffer(bufv.target, handle);
}

//// GLTFPrimitive
GLTFPrimitive::GLTFPrimitive(tinygltf::Model& model, tinygltf::Primitive& primitive, GLTFScene* scene) {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// std::cout << "vao: " << vao << std::endl;

	pos_acc = &model.accessors[primitive.attributes["POSITION"]];
	normal_acc = &model.accessors[primitive.attributes["NORMAL"]];
	texcoord_acc = &model.accessors[primitive.attributes["TEXCOORD_0"]];
	index_acc = &model.accessors[primitive.indices];
	mode = primitive.mode;

	index_bufv = scene->bufferViews[index_acc->bufferView];
	pos_bufv = scene->bufferViews[pos_acc->bufferView];
	normal_bufv = scene->bufferViews[normal_acc->bufferView];
	texcoord_bufv = scene->bufferViews[texcoord_acc->bufferView];
	
	index_bufv->bind();

	pos_bufv->bind();
	glVertexAttribPointer(
		0, 
		tinygltf::GetNumComponentsInType(pos_acc->type), 
		pos_acc->componentType, 
		pos_acc->normalized ? GL_TRUE : GL_FALSE,
		pos_acc->ByteStride(pos_bufv->bufv),
		(void *)(0 + pos_acc->byteOffset)
	);
	// std::cout << "pos" << std::endl;
	// std::cout << "vertex attrib " << 0 << ": " << std::endl << "vbo: " << pos_bufv->handle << ", size: " << tinygltf::GetNumComponentsInType(pos_acc->type)
	// 		<< ", component type: " << pos_acc->componentType << std::endl 
	// 		<< "stride: " << pos_acc->ByteStride(pos_bufv->bufv) << ", offset: " << pos_acc->byteOffset << std::endl;
	glEnableVertexAttribArray(0);

	normal_bufv->bind();
	glVertexAttribPointer(
		2, 
		tinygltf::GetNumComponentsInType(normal_acc->type), 
		normal_acc->componentType, 
		normal_acc->normalized ? GL_TRUE : GL_FALSE,
		normal_acc->ByteStride(normal_bufv->bufv),
		(void *)(0 + normal_acc->byteOffset)
	);
	glEnableVertexAttribArray(2);

	// std::cout << "normal" << std::endl;
	// std::cout << "vertex attrib " << 2 << ": " << std::endl << "vbo: " << normal_bufv->handle << ", size: " << tinygltf::GetNumComponentsInType(normal_acc->type)
	// 		<< ", component type: " << normal_acc->componentType << std::endl 
	// 		<< "stride: " << normal_acc->ByteStride(normal_bufv->bufv) << ", offset: " << normal_acc->byteOffset << std::endl;

	texcoord_bufv->bind();
	glVertexAttribPointer(
		1, 
		tinygltf::GetNumComponentsInType(texcoord_acc->type), 
		texcoord_acc->componentType, 
		texcoord_acc->normalized ? GL_TRUE : GL_FALSE,
		texcoord_acc->ByteStride(texcoord_bufv->bufv),
		(void *)(0 + texcoord_acc->byteOffset)
	);
	glEnableVertexAttribArray(1);

	// std::cout << "texcoord" << std::endl;
	// std::cout << "vertex attrib " << 1 << ": " << std::endl << "vbo: " << texcoord_bufv->handle << ", size: " << tinygltf::GetNumComponentsInType(texcoord_acc->type)
	// 		<< ", component type: " << texcoord_acc->componentType << std::endl 
	// 		<< "stride: " << texcoord_acc->ByteStride(texcoord_bufv->bufv) << ", offset: " << texcoord_acc->byteOffset << std::endl;

	default_material = scene->materials[primitive.material];
}

void GLTFPrimitive::draw(const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material) {
	if (material == nullptr) {
		material = default_material;
	}
	assert(material != nullptr);
	material->apply(transform, cam);

	glBindVertexArray(vao);
	index_bufv->bind();

	glDrawElements(mode, index_acc->count, index_acc->componentType, (void*)index_acc->byteOffset);
}

//// GLTFMesh
GLTFMesh::GLTFMesh(tinygltf::Model& model, tinygltf::Mesh& mesh, GLTFScene* scene) {
	for (tinygltf::Primitive& primitive : mesh.primitives) {
		auto prim = std::make_shared<GLTFPrimitive>(model, primitive, scene);
		primitives.push_back(prim);
	}
	std::cout << "create " << mesh.primitives.size() << " primitives" << std::endl;
}

void GLTFMesh::draw(const Camera& cam, glm::mat4& transform, std::shared_ptr<Material> material) {
	auto* render_queue = GLTFRenderQueue::getInstance();
	for (const auto& pr : primitives) {
		// pr->draw(cam, transform, material);
		glm::vec4 pos = cam.project() * cam.view() * transform * glm::vec4(.0f, .0f, .0f, 1.0f);
		float z = pos.z / pos.w;
		if (material == nullptr) {
			material = pr->default_material;
		}
		render_queue->push(GLTFRenderRequest{
			.cam = &cam,
			.transform = transform,
			.material = material,
			.primitive = pr,
			.z = z,
		});
	}
}

GLTFRenderQueue* GLTFRenderQueue::getInstance() {
	if (instance == nullptr) {
		instance = new GLTFRenderQueue();
	}
	return instance;
}

void GLTFRenderQueue::push(GLTFRenderRequest&& request) {
	switch (request.material->alpha_mode) {
	case Material::BLEND:
		blend_queue.push_back(request);
		break;
	default:
		opaque_queue.push_back(request);
	}
}

void GLTFRenderQueue::render() {
	glDepthMask(GL_TRUE);
	// glDepthFunc(GL_LESS);
	while (!opaque_queue.empty()) {
		GLTFRenderRequest& request = opaque_queue.front();
		request.primitive->draw(*request.cam, request.transform, request.material);
		opaque_queue.pop_front();
	}
	opaque_queue.clear();

	glDepthMask(GL_FALSE);
	// glDepthFunc(GL_ALWAYS);
	std::sort(blend_queue.begin(), blend_queue.end());
	while (!blend_queue.empty()) {
		GLTFRenderRequest& request = blend_queue.front();
		request.primitive->draw(*request.cam, request.transform, request.material);
		blend_queue.pop_front();
	}
	blend_queue.clear();
	glDepthMask(GL_TRUE);
}