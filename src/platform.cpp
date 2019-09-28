
#include <glad/glad.h>

#include "lib/log.h"
#include "lib/alloc.h"
#include "platform.h"

#ifdef WIN32
extern "C" {
	__declspec(dllexport) bool NvOptimusEnablement = true;
	__declspec(dllexport) bool AmdPowerXpressRequestHighPerformance = true;
}
#endif

void gl_debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up);

string glString(GLenum name) {
	return string::literal((const char*)glGetString(name));
}

void Platform::init() {

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	window = SDL_CreateWindow("Exile", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if(!window) {
		die("Failed to create SDL window!");
	}
	info("Created SDL window.");
	
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);

	gl_context = SDL_GL_CreateContext(window);
	if(!gl_context) {
		die("Failed to create OpenGL 4.5 context! Does your GPU support GL 4.5?");
	}
	info("Created OpenGL 4.5 Context.");
	
	SDL_GL_MakeCurrent(window, gl_context);
	SDL_GL_SetSwapInterval(1);

	if(!gladLoadGL()) {
		die("Failed to load OpenGL functions!");
	}
	info("Loaded OpenGL functions.");

	info("GL version: %", glString(GL_VERSION));
	info("GL renderer: %", glString(GL_RENDERER));
	info("GL vendor: %", glString(GL_VENDOR));
	info("GL shading: %", glString(GL_SHADING_LANGUAGE_VERSION));

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(gl_debug_proc, null);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, null, GL_TRUE);
}

void Platform::destroy() {
	gl_validate();
	SDL_GL_DeleteContext(gl_context);
	SDL_DestroyWindow(window);
	window = null;
	gl_context = null;
	SDL_Quit();	
}

void Platform::gl_validate() {
	
	#define GL_CHECK(type) if(glIs##type && glIs##type(i)) { warn("Leaked OpenGL handle % of type %", i, string::literal(#type)); leaked = true;}

	bool leaked = false;
	for(GLuint i = 0; i < 100000; i++) {
		GL_CHECK(Texture);
		GL_CHECK(Buffer);
		GL_CHECK(Framebuffer);
		GL_CHECK(Renderbuffer);
		GL_CHECK(VertexArray);
		GL_CHECK(Program);
		GL_CHECK(ProgramPipeline);
		GL_CHECK(Query);

		if(glIsShader(i)) {

			leaked = true;
			GLint shader_len = 0;
			glGetShaderiv(i, GL_SHADER_SOURCE_LENGTH, &shader_len);

			GLchar* shader = (GLchar*)base_alloc(shader_len);
			glGetShaderSource(i, shader_len, null, shader);

			warn("Leaked OpenGL shader %, source %", i, string::literal((const char*)shader)); 

			base_free(shader);
		}
	}

	if(!leaked) {
		info("No OpenGL objects leaked.");
	}

	#undef GL_CHECK
}

void gl_debug_proc(GLenum glsource, GLenum gltype, GLuint id, GLenum severity, GLsizei length, const GLchar* glmessage, const void* up) {

	string message = glmessage;
	string source, type;

	switch(glsource) {
	case GL_DEBUG_SOURCE_API:
		source = "OpenGL API";
		break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
		source = "Window System";
		break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER:
		source = "Shader Compiler";
		break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:
		source = "Third Party";
		break;
	case GL_DEBUG_SOURCE_APPLICATION:
		source = "Application";
		break;
	case GL_DEBUG_SOURCE_OTHER:
		source = "Other";
		break;
	}

	switch(gltype) {
	case GL_DEBUG_TYPE_ERROR:
		type = "Error";
		break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		type = "Deprecated";
		break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		type = "Undefined Behavior";
		break;
	case GL_DEBUG_TYPE_PORTABILITY:
		type = "Portability";
		break;
	case GL_DEBUG_TYPE_PERFORMANCE:
		type = "Performance";
		break;
	case GL_DEBUG_TYPE_MARKER:
		type = "Marker";
		break;
	case GL_DEBUG_TYPE_PUSH_GROUP:
		type = "Push Group";
		break;
	case GL_DEBUG_TYPE_POP_GROUP:
		type = "Pop Group";
		break;
	case GL_DEBUG_TYPE_OTHER:
		type = "Other";
		break;
	}

	switch(severity) {
	case GL_DEBUG_SEVERITY_HIGH:
	case GL_DEBUG_SEVERITY_MEDIUM:
	case GL_DEBUG_SEVERITY_LOW:
		warn("OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		info("OpenGL: % SOURCE: % TYPE: %", message, source, type);
		break;
	}
}
