
#pragma once

#include "../common.hpp"

#include <string>
#include <map>
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

	void load_shader(std::string name, file_path v, file_path f, std::function<void()> u = []()->void{});
	void reload_shader(std::string name);
	void remove_shader(std::string name);
	void remove_shaders();
	void use_shader(std::string name);
	GLuint get_uniform_loc(std::string shader, std::string name);

private:
	struct SHADER {
		file_path vertex_path;
		file_path fragment_path;
		std::function<void()> set_uniforms;
		GLuint program;
	};

private:
	std::string load_file_str(file_path file);
	std::map<std::string, SHADER> shaders;
	bool initialized;
};
