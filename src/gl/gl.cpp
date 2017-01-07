
#include "gl.hpp"

gl_manager::gl_manager() {
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
}

gl_manager::~gl_manager() {

}

void gl_manager::clear_frame() {
	glClearColor(0.0f, 1.0f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
