
#include "gl.h"

glClipControl_t glClipControl;

glIs_t glIsTexture;
glIs_t glIsBuffer;
glIs_t glIsFramebuffer;
glIs_t glIsRenderbuffer;
glIs_t glIsVertexArray;
glIs_t glIsShader;
glIs_t glIsProgram;
glIs_t glIsProgramPipeline;
glIs_t glIsQuery;

glGetStringi_t      glGetStringi;
glGetInteger64v_t   glGetInteger64v;
glGetBooleani_v_t   glGetBooleani_v;
glGetDoublei_v_t    glGetDoublei_v;
glGetFloati_v_t     glGetFloati_v;
glGetIntegeri_v_t   glGetIntegeri_v;
glGetInteger64i_v_t glGetInteger64i_v;

glDrawArraysInstanced_t 			glDrawArraysInstanced;
glDrawArraysInstancedBaseInstance_t	glDrawArraysInstancedBaseInstance;
glDrawElementsInstanced_t 			glDrawElementsInstanced;
glDrawElementsInstancedBaseVertex_t glDrawElementsInstancedBaseVertex;
glVertexAttribDivisor_t				glVertexAttribDivisor;

glDebugMessageCallback_t 	glDebugMessageCallback;
glDebugMessageInsert_t		glDebugMessageInsert;
glDebugMessageControl_t		glDebugMessageControl;

glAttachShader_t  		glAttachShader;
glCompileShader_t 		glCompileShader;
glCreateProgram_t 		glCreateProgram;
glCreateShader_t  		glCreateShader;
glDeleteProgram_t 		glDeleteProgram;
glDeleteShader_t  		glDeleteShader;
glLinkProgram_t   		glLinkProgram;
glShaderSource_t  		glShaderSource;
glUseProgram_t    		glUseProgram;
glGetShaderiv_t 		glGetShaderiv;
glGetShaderInfoLog_t	glGetShaderInfoLog;
glGetShaderSource_t 	glGetShaderSource;
glMinSampleShading_t	glMinSampleShading;

glGetUniformLocation_t 		glGetUniformLocation;
glGetAttribLocation_t		glGetAttribLocation;
glUniformMatrix4fv_t   		glUniformMatrix4fv;
glUniform1f_t				glUniform1f;
glUniform2f_t				glUniform2f;
glUniform1i_t				glUniform1i;
glUniform2i_t				glUniform2i;
glUniform4fv_t 				glUniform4fv;
glUniform3fv_t				glUniform3fv;
glGenerateMipmap_t			glGenerateMipmap;
glActiveTexture_t			glActiveTexture;
glCreateTextures_t			glCreateTextures;
glBindTextureUnit_t			glBindTextureUnit;
glTexParameterIiv_t 		glTexParameterIiv;
glTexStorage3D_t 			glTexStorage3D;
glTexSubImage3D_t			glTexSubImage3D;
glBindSampler_t				glBindSampler;

glBindVertexArray_t    		glBindVertexArray; 		
glDeleteVertexArrays_t 		glDeleteVertexArrays;
glGenVertexArrays_t    		glGenVertexArrays;
glBlendEquation_t			glBlendEquation;
glBindBuffer_t				glBindBuffer;
glDeleteBuffers_t			glDeleteBuffers;
glGenBuffers_t				glGenBuffers;
glBufferData_t				glBufferData;
glNamedBufferData_t			glNamedBufferData;
glVertexAttribPointer_t		glVertexAttribPointer;
glVertexAttribIPointer_t	glVertexAttribIPointer;
glEnableVertexAttribArray_t glEnableVertexAttribArray;
glDrawElementsBaseVertex_t	glDrawElementsBaseVertex;

glGenRenderbuffers_t				glGenRenderbuffers;
glBindRenderbuffer_t				glBindRenderbuffer;
glRenderbufferStorage_t				glRenderbufferStorage;
glRenderbufferStorageMultisample_t	glRenderbufferStorageMultisample;
glDeleteRenderbuffers_t				glDeleteRenderbuffers;
glTexImage2DMultisample_t 			glTexImage2DMultisample;
glClearTexImage_t					glClearTexImage;

glGenFramebuffers_t 		glGenFramebuffers;
glDeleteFramebuffers_t 		glDeleteFramebuffers;
glBindFramebuffer_t 		glBindFramebuffer;
glFramebufferTexture2D_t 	glFramebufferTexture2D;
glFramebufferRenderbuffer_t glFramebufferRenderbuffer;
glDrawBuffers_t 			glDrawBuffers;
glBlitNamedFramebuffer_t	glBlitNamedFramebuffer;
glBlitFramebuffer_t 		glBlitFramebuffer;

glNamedFramebufferDrawBuffers_t  		glNamedFramebufferDrawBuffers;
glNamedFramebufferTexture_t	 	 		glNamedFramebufferTexture;
glNamedFramebufferRenderbuffer_t 		glNamedFramebufferRenderbuffer;
glNamedRenderbufferStorage_t			glNamedRenderbufferStorage;
glNamedRenderbufferStorageMultisample_t	glNamedRenderbufferStorageMultisample;
glNamedFramebufferReadBuffer_t			glNamedFramebufferReadBuffer;
glClearNamedFramebufferiv_t				glClearNamedFramebufferiv;
glClearNamedFramebufferuiv_t			glClearNamedFramebufferuiv;
glClearNamedFramebufferfv_t				glClearNamedFramebufferfv;

glNamedStringARB_t			glNamedStringARB;
glDeleteNamedStringARB_t	glDeleteNamedStringARB;
glCompileShaderIncludeARB_t	glCompileShaderIncludeARB;
