
#pragma once

#include "..\common.hpp"

#include <string>
#include <map>

#include <GLFW/glfw3.h>
#include <glext.h>

class gl_manager {
public:
	gl_manager();
	~gl_manager();

	void init();
	void kill();
	void clear_frame();

private:
	PFNGLBINDVERTEXARRAYPROC			glBindVertexArray;
	PFNGLDELETEBUFFERSPROC				glDeleteBuffers;
	PFNGLBINDBUFFERPROC					glBindBuffer;
	PFNGLBUFFERDATAPROC					glBufferData;
	PFNGLUSEPROGRAMPROC					glUseProgram;
	PFNGLENABLEVERTEXATTRIBARRAYPROC	glEnableVertexAttribArray;
	PFNGLVERTEXATTRIBPOINTERPROC		glVertexAttribPointer;
	PFNGLUNIFORMMATRIX4FVPROC			glUniformMatrix4fv;
	PFNGLGETUNIFORMLOCATIONPROC			glGetUniformLocation;
	PFNGLUNIFORM4FPROC					glUniform4f;
	PFNGLUNIFORM3FPROC					glUniform3f;
	PFNGLCREATESHADERPROC				glCreateShader;
	PFNGLSHADERSOURCEPROC				glShaderSource;
	PFNGLCOMPILESHADERPROC				glCompileShader;
	PFNGLCREATEPROGRAMPROC				glCreateProgram;
	PFNGLDELETESHADERPROC				glDeleteShader;
	PFNGLATTACHSHADERPROC				glAttachShader;
	PFNGLLINKPROGRAMPROC				glLinkProgram;
	PFNGLGENVERTEXARRAYSPROC			glGenVertexArrays;
	PFNGLGENBUFFERSPROC					glGenBuffers;
	PFNGLDELETEVERTEXARRAYSPROC			glDeleteVertexArrays;
	PFNGLDELETEPROGRAMPROC				glDeleteProgram;
	PFNGLACTIVETEXTUREPROC				glActiveTexture; 
	PFNGLUNIFORM1IPROC					glUniform1i;
	PFNGLDISABLEVERTEXATTRIBARRAYPROC	glDisableVertexAttribArray;
	PFNGLUNIFORM1FPROC					glUniform1f;
};