
#include "state.hpp"

// testing testing testing
GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
};

game_state::game_state() {
	running = false;
	initialized = false;
}

game_state::~game_state() {
	kill();
}

void game_state::init() {

	cam.reset();
	glfw.init();
	gl.init();

	gl.load_texture("test", "textures/test.png");
	gl.load_shader("tex", "shaders/tex_vertex.v", "shaders/tex_fragment.f",
		[this]() -> void {
		    glm::mat4 model, view, proj;
		    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 1, 1));
		    view = cam.getView();
		    proj = glm::perspective(glm::radians(60.0f), (GLfloat)1280 / (GLfloat)720, 0.1f, 100.0f);
		    glm::mat4 modelviewproj = proj * view * model;
			glUniformMatrix4fv(gl.get_uniform_loc("tex", "modelviewproj"), 1, GL_FALSE, glm::value_ptr(modelviewproj));
			glUniform1i(gl.get_uniform_loc("tex", "tex"), 0);
	});

	input_state state("default");
	state.key_button = [this](int key, int, int, int) {
		if(key == GLFW_KEY_ESCAPE) {
			glfw.close_window();
		}
	};
	state.cursor_pos = [this](double x, double y) -> void {
		static double mx = x, my = y;
		cam.move(x - mx, y - my);
		mx = x; my = y;
	};
	state.every_frame = [this]() -> void {
		if(glfw.keydown(GLFW_KEY_W)) {
			cam.pos += cam.front * cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_S)) {
			cam.pos -= cam.front * cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_D)) {
			cam.pos += cam.right * cam.speed;
		}
		else if(glfw.keydown(GLFW_KEY_A)) {
			cam.pos -= cam.right * cam.speed;
		}
	};
	state.close_window = [this]() -> void {running = false;};
	glfw.input_add_state(state);

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

	running = true;
	initialized = true;
}

void game_state::kill() {
	if(initialized) {
		gl.kill();
	    glfw.kill();

		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);

		initialized = false;
	}
}

void game_state::run() {
	while (running) {
		glfw.events();

		gl.clear_frame();

		gl.use_shader("tex");

		gl.use_texture("test", 0);
    	glBindVertexArray(VAO);
    	glDrawArrays(GL_TRIANGLES, 0, 36);
    	glBindVertexArray(0);

		glfw.swap_window();
	}
}
