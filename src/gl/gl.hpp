
#pragma once

#include "../common.hpp"

#include <string>
#include <unordered_map>
#include <experimental/filesystem>
#include <functional>

#include <gl.hpp>

class gl_manager {
public:
	gl_manager();
	~gl_manager();

	void init();
	void kill();
	void clear_frame();

	void load_shader(std::string name, std_file_path v, std_file_path f, std::function<void()> u = []()->void{});
	void reload_shader(std::string name);
	void remove_shader(std::string name);
	void remove_shaders();
	void use_shader(std::string name);
	GLuint get_uniform_loc(std::string shader, std::string name);

	void load_texture(std::string name, std_file_path path);
	void reload_texture(std::string name);
	void remove_texture(std::string name);
	void remove_textures();
	void use_texture(std::string name, int index);

private:
	struct SHADER {
		std_file_path vertex_path;
		std_file_path fragment_path;
		std::function<void()> set_uniforms;
		GLuint program;
	};
	struct TEXTURE {
		std_file_path path;
		int w, h;
		GLuint texture;
	};

private:
	std::string load_file_str(std_file_path file);
	std::unordered_map<std::string, SHADER> shaders;
	std::unordered_map<std::string, TEXTURE> textures;
	bool initialized;
};
