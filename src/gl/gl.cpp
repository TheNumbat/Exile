
#include "gl.hpp"

#include <glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <fstream>

gl_manager::gl_manager() {
	initialized = false;
}

void gl_manager::kill() {
	if(initialized) {
		remove_shaders();
		remove_textures();
		initialized = false;
	}
}	

void gl_manager::init() {

	LOG_PUSH_CONTEXT(GL_MANAGER);
	
	LOG_INFO("Initializing gl_manager");
	LOG_PUSH_SEC();

    glewExperimental = true;
	GLenum error = glewInit();
	assert(error == GLEW_OK);
	LOG_INFO("Initialized GLEW");

	glEnable(GL_DEPTH_TEST);
	// ...

	LOG_POP_SEC();
	LOG_POP_CONTEXT();

	initialized = true;
}

gl_manager::~gl_manager() {
	kill();
}

void gl_manager::clear_frame() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_manager::load_shader(std::string name, std_file_path v, std_file_path f, std::function<void()> u) {
	LOG_PUSH_CONTEXT(GL_MANAGER);
	LOG_INFO("Loading shader " + name + " from " + v.string() + " and " + f.string());
	SHADER s;
	s.vertex_path = v;
	s.fragment_path = f;
	s.set_uniforms = u;

	std::string vertex_text = load_file_str(v);
	std::string fragment_text = load_file_str(f);
	const char* vertex_c_str = vertex_text.c_str();
	const char* fragment_c_str = fragment_text.c_str();

	s.program = glCreateProgram();
	GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
	GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v_shader, 1, &vertex_c_str, nullptr);
	glShaderSource(f_shader, 1, &fragment_c_str, nullptr);
	glCompileShader(v_shader);
	glCompileShader(f_shader);
	glAttachShader(s.program, v_shader);
	glAttachShader(s.program, f_shader);
	glLinkProgram(s.program);
	glDeleteShader(v_shader);
	glDeleteShader(f_shader);

	assert(shaders.find(name) == shaders.end());

	shaders.insert({name, s});

	LOG_POP_CONTEXT();
}

void gl_manager::reload_shader(std::string name) {
	LOG_PUSH_CONTEXT(GL_MANAGER);
	LOG_INFO("Reloading shader " + name);
	auto shader_entry = shaders.find(name);
	assert(shader_entry != shaders.end());
	std_file_path v = shader_entry->second.vertex_path;
	std_file_path f = shader_entry->second.fragment_path;
	remove_shader(name);
	load_shader(name, v, f);
	LOG_POP_CONTEXT();
}

void gl_manager::remove_shader(std::string name) {
	auto shader_entry = shaders.find(name);
	assert(shader_entry != shaders.end());
	glDeleteProgram(shader_entry->second.program);
	shaders.erase(shader_entry);
}

void gl_manager::remove_shaders() {
	while(shaders.size()) {
		remove_shader(shaders.begin()->first);
	}
}

void gl_manager::use_shader(std::string name) {
	auto shader_entry = shaders.find(name);
	assert(shader_entry != shaders.end());
	glUseProgram(shader_entry->second.program);
	shader_entry->second.set_uniforms();
}

GLuint gl_manager::get_uniform_loc(std::string shader, std::string name) {
	auto shader_entry = shaders.find(shader);
	assert(shader_entry != shaders.end());
	return glGetUniformLocation(shader_entry->second.program, name.c_str());
}

std::string gl_manager::load_file_str(std_file_path file) {
	std::ifstream fin(file.string());
	std::string file_str;
	while(fin.good()) {
		std::string line;
		std::getline(fin, line);
		file_str.append(line + "\n");
	}
	return file_str;
}

void gl_manager::load_texture(std::string name, std_file_path path) {
	LOG_PUSH_CONTEXT(GL_MANAGER);
	LOG_INFO("Loading texture " + name + " from " + path.string());
	TEXTURE tex;
	tex.path = path;
	glGenTextures(1, &tex.texture);

	u8* image = stbi_load(path.string().c_str(), &tex.w, &tex.h, nullptr, 0);

	assert(image);
	if(!image) {
		LOG_ERROR("Failed to load texture " + name + " from " + path.string());
	}

	glBindTexture(GL_TEXTURE_2D, tex.texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex.w, tex.h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	stbi_image_free(image);

	assert(textures.find(name) == textures.end());

	textures.insert({name, tex});

	LOG_POP_CONTEXT();
}

void gl_manager::reload_texture(std::string name) {
	LOG_PUSH_CONTEXT(GL_MANAGER);
	LOG_INFO("Reloading texture " + name);
	auto texture_entry = textures.find(name);
	assert(texture_entry != textures.end());
	std_file_path path = texture_entry->second.path;
	remove_texture(name);
	load_texture(name, path);
	LOG_POP_CONTEXT();
}

void gl_manager::remove_texture(std::string name) {
	auto texture_entry = textures.find(name);
	assert(texture_entry != textures.end());
	glDeleteTextures(1, &texture_entry->second.texture);
	textures.erase(texture_entry);
}

void gl_manager::remove_textures() {
	while(textures.size()) {
		remove_texture(textures.begin()->first);
	}
}

void gl_manager::use_texture(std::string name, int index) {
	auto texture_entry = textures.find(name);
	assert(texture_entry != textures.end());
	glActiveTexture(GL_TEXTURE0 + index);
	glBindTexture(GL_TEXTURE_2D, texture_entry->second.texture);
}
