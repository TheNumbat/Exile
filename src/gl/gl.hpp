
#pragma once

#include "../common.hpp"

#include <string>
#include <map>

#include <gl.hpp>

#include "cam/cam.h"

class gl_manager {
public:
	gl_manager();
	~gl_manager();

	void init();
	void kill();
	void clear_frame();

	void render_box();
	GLuint VAO, VBO, program;

	cam_3d cam;

private:
	bool initialized;
};
