
#include "gl.hpp"
#include <glm.hpp>

GLfloat vertices[] = {
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
     0.5f, -0.5f, -0.5f,
     0.5f, -0.5f,  0.5f,
     0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f,  0.5f,
    -0.5f, -0.5f, -0.5f,
    -0.5f,  0.5f, -0.5f,
     0.5f,  0.5f, -0.5f,
     0.5f,  0.5f,  0.5f,
     0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f,  0.5f,
    -0.5f,  0.5f, -0.5f
};

gl_manager::gl_manager() {
	initialized = false;
	cam.reset();
}

void gl_manager::kill() {
	if(initialized) {
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteProgram(program);
		initialized = false;
	}
}	

void gl_manager::init() {
	glBindVertexArray				= (PFNGLBINDVERTEXARRAYPROC)			glfwGetProcAddress("glBindVertexArray");
	glDeleteBuffers					= (PFNGLDELETEBUFFERSPROC)				glfwGetProcAddress("glDeleteBuffers");
	glBindBuffer					= (PFNGLBINDBUFFERPROC)					glfwGetProcAddress("glBindBuffer");
	glBufferData					= (PFNGLBUFFERDATAPROC)					glfwGetProcAddress("glBufferData");
	glUseProgram					= (PFNGLUSEPROGRAMPROC)					glfwGetProcAddress("glUseProgram");
	glEnableVertexAttribArray		= (PFNGLENABLEVERTEXATTRIBARRAYPROC)	glfwGetProcAddress("glEnableVertexAttribArray");
	glVertexAttribPointer			= (PFNGLVERTEXATTRIBPOINTERPROC)		glfwGetProcAddress("glVertexAttribPointer");
	glUniformMatrix4fv				= (PFNGLUNIFORMMATRIX4FVPROC)			glfwGetProcAddress("glUniformMatrix4fv");
	glGetUniformLocation			= (PFNGLGETUNIFORMLOCATIONPROC)			glfwGetProcAddress("glGetUniformLocation");
	glUniform4f						= (PFNGLUNIFORM4FPROC)					glfwGetProcAddress("glUniform4f");
	glUniform3f						= (PFNGLUNIFORM3FPROC)					glfwGetProcAddress("glUniform3f");
	glCreateShader					= (PFNGLCREATESHADERPROC)				glfwGetProcAddress("glCreateShader");
	glShaderSource					= (PFNGLSHADERSOURCEPROC)				glfwGetProcAddress("glShaderSource");
	glCompileShader					= (PFNGLCOMPILESHADERPROC)				glfwGetProcAddress("glCompileShader");
	glCreateProgram					= (PFNGLCREATEPROGRAMPROC)				glfwGetProcAddress("glCreateProgram");
	glDeleteShader					= (PFNGLDELETESHADERPROC)				glfwGetProcAddress("glDeleteShader");
	glAttachShader					= (PFNGLATTACHSHADERPROC)				glfwGetProcAddress("glAttachShader");
	glLinkProgram					= (PFNGLLINKPROGRAMPROC)				glfwGetProcAddress("glLinkProgram");
	glGenVertexArrays				= (PFNGLGENVERTEXARRAYSPROC)			glfwGetProcAddress("glGenVertexArrays");
	glGenBuffers					= (PFNGLGENBUFFERSPROC)					glfwGetProcAddress("glGenBuffers");
	glDeleteVertexArrays			= (PFNGLDELETEVERTEXARRAYSPROC)			glfwGetProcAddress("glDeleteVertexArrays");
	glDeleteProgram					= (PFNGLDELETEPROGRAMPROC)				glfwGetProcAddress("glDeleteProgram");
	glActiveTexture					= (PFNGLACTIVETEXTUREPROC)				glfwGetProcAddress("glActiveTexture");
	glUniform1i						= (PFNGLUNIFORM1IPROC)					glfwGetProcAddress("glUniform1i");
	glDisableVertexAttribArray		= (PFNGLDISABLEVERTEXATTRIBARRAYPROC)	glfwGetProcAddress("glDisableVertexAttribArray");
	glUniform1f						= (PFNGLUNIFORM1FPROC)					glfwGetProcAddress("glUniform1f");

	// test
	const GLchar* vertex = {
		"#version 330 core\n"

		"layout (location = 0) in vec3 position;\n"

		"uniform mat4 modelviewproj;\n"

		"void main() {\n"
		"	gl_Position = modelviewproj * vec4(position, 1.0f);\n"
		"}\n"
	};
	const GLchar* fragment = {
		"#version 330 core\n"

		"uniform vec4 vcolor;\n"
		"out vec4 color;\n"

		"void main() {\n"
		"	color = vcolor;\n"
		"}\n"
	};

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, 1280, 720);

	GLuint v, f;
	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(v, 1, &vertex, NULL);
	glShaderSource(f, 1, &fragment, NULL);
	glCompileShader(v);
	glCompileShader(f);
	program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glLinkProgram(program);
	glDeleteShader(v);
	glDeleteShader(f);

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0); // Unbind VAO

	initialized = true;
}

gl_manager::~gl_manager() {
	kill();
}

void gl_manager::clear_frame() {
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void gl_manager::render_box() {
    glm::mat4 model, view, proj;
    model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(1, 1, 1));
    view = cam.getView();
    proj = glm::perspective(glm::radians(60.0f), (GLfloat)1280 / (GLfloat)720, 0.1f, 100.0f);
    glm::mat4 modelviewproj = proj * view * model;
    
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program,"modelviewproj"), 1, GL_FALSE, glm::value_ptr(modelviewproj));
    glUniform4f(glGetUniformLocation(program, "vcolor"), 0.5f, 0.2f, 0.3f, 1.0f);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}