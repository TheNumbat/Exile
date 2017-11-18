// OpenGL 

typedef unsigned int GLenum;
typedef unsigned char GLboolean;
typedef unsigned int GLbitfield;
typedef signed char GLbyte;
typedef short GLshort;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLubyte;
typedef unsigned short GLushort;
typedef unsigned int GLuint;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef double GLclampd;
typedef void GLvoid;
typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

enum class gl_bool : GLboolean {
	_false						= 0,
	_true 						= 1
};

enum class gl_info : GLenum {
	vendor                  	= 0x1F00,
	renderer                	= 0x1F01,
	version                 	= 0x1F02,
	extensions              	= 0x1F03,
	shading_language_version	= 0x8B8C
};

enum class gl_tex_target : GLenum {
	_1D							= 0x0DE0,
	_1D_array               	= 0x8C18,
	_2D                     	= 0x0DE1,
	_2D_array               	= 0x8C1A,
	_2D_multisample         	= 0x9100,
	_2D_multisample_array   	= 0x9102,
	_3D                     	= 0x806F,
	cube_map					= 0x8513,
	cube_map_array         		= 0x9009,
	rectangle              		= 0x84F5
};

enum class gl_tex_param : GLenum {
	depth_stencil_texture_mode	= 0x90EA,
	base_level             		= 0x813C,
	compare_func           		= 0x884D,
	compare_mode           		= 0x884C,
	lod_bias               		= 0x8501,
	min_filter             		= 0x2801,
	mag_filter             		= 0x2800,
	min_lod                		= 0x813A,
	max_lod                		= 0x813B,
	max_level              		= 0x813D,
	swizzle_r              		= 0x8E42,
	swizzle_g              		= 0x8E43,
	swizzle_b              		= 0x8E44,
	swizzle_a              		= 0x8E45,
	wrap_s                 		= 0x2802,
	wrap_t                 		= 0x2803,
	wrap_r                 		= 0x8072,
	border_color				= 0x1004,
	swizzle_rgba				= 0x8E46
};

enum class gl_tex_format : GLint {
	depth_component				= 0x1902,
	depth_stencil        		= 0x84F9,
	red							= 0x1903,
	rg                   		= 0x8227,
	rgb                  		= 0x1907,
	rgba                 		= 0x1908,
	r8                   		= 0x8229,
	r8_snorm             		= 0x8F94,
	r16                  		= 0x822A,
	r16_snorm            		= 0x8F98,
	rg8                  		= 0x822B,
	rg8_snorm            		= 0x8F95,
	rg16                 		= 0x822C,
	rg16_snorm                 	= 0x8F99,
	r3_g3_b2					= 0x2A10,
	rgb4						= 0x804F,
	rgb5						= 0x8050,
	rgb8						= 0x8051,
	rgb8_snorm          		= 0x8F96,
	rgb10						= 0x8052,
	rgb12						= 0x8053,
	rgb16_snorm         		= 0x8F9A,
	rgba2						= 0x8055,
	rgba4						= 0x8056,
	rgb5_a1						= 0x8057,
	rgba8						= 0x8058,
	rgba8_snorm         		= 0x8F97,
	rgb10_a2					= 0x8059,
	rgb10_a2ui          		= 0x906F,
	rgba12						= 0x805A,
	rgba16						= 0x805B,
	srgb8                      	= 0x8C41,
	srgb8_alpha8               	= 0x8C43,
	r16f                       	= 0x822D,
	rg16f                      	= 0x822F,
	rgb16f                     	= 0x881B,
	rgba16f                    	= 0x881A,
	r32f                       	= 0x822E,
	rg32f                      	= 0x8230,
	rgb32f                     	= 0x8815,
	rgba32f                    	= 0x8814,
	r11f_g11f_b10f             	= 0x8C3A,
	rgb9_e5                    	= 0x8C3D,
	r8i                        	= 0x8231,
	r8ui                       	= 0x8232,
	r16i                       	= 0x8233,
	r16ui                      	= 0x8234,
	r32i                       	= 0x8235,
	r32ui                      	= 0x8236,
	rg8i                       	= 0x8237,
	rg8ui                      	= 0x8238,
	rg16i                      	= 0x8239,
	rg16ui                     	= 0x823A,
	rg32i                      	= 0x823B,
	rg32ui                     	= 0x823C,
	rgb8i                      	= 0x8D8F,
	rgb8ui                     	= 0x8D7D,
	rgb16i                     	= 0x8D89,
	rgb16ui                    	= 0x8D77,
	rgb32i                     	= 0x8D83,
	rgb32ui                    	= 0x8D71,
	rgba8i                     	= 0x8D8E,
	rgba8ui                     = 0x8D7C,
	rgba16i                    	= 0x8D88,
	rgba16ui                   	= 0x8D76,
	rgba32i                    	= 0x8D82,
	rgba32ui                   	= 0x8D70,
	compressed_red             	= 0x8225,
	compressed_rg              	= 0x8226,
	compressed_rgb             	= 0x84ED,
	compressed_rgba            	= 0x84EE,
	compressed_srgb            	= 0x8C48,
	compressed_srgb_alpha      	= 0x8C49,
	compressed_red_rgtc1       	= 0x8DBB,
	compressed_signed_red_rgtc1	= 0x8DBC,
	compressed_rg_rgtc2        	= 0x8DBD,
	compressed_signed_rg_rgtc2 	= 0x8DBE,
	compressed_rgba_bptc_unorm 	= 0x8E8C,
	compressed_srgb_alpha_bptc_unorm	= 0x8E8D,
	compressed_rgb_bptc_signed_float 	= 0x8E8E,
	compressed_rgb_bptc_unsigned_float 	= 0x8E8F
};

enum class gl_pixel_data_format : GLenum {
	red							= 0x1903,
	rg                 			= 0x8227,
	rgb                			= 0x1907,
	bgr                			= 0x80E0,
	rgba               			= 0x1908,
	bgra               			= 0x80E1,
	red_integer        			= 0x8D94,
	rg_integer         			= 0x8228,
	rgb_integer        			= 0x8D98,
	bgr_integer        			= 0x8D9A,
	rgba_integer       			= 0x8D99,
	bgra_integer       			= 0x8D9B,
	stencil_index				= 0x1901,
	depth_component				= 0x1902,
	depth_stencil       		= 0x84F9	
};

enum class gl_pixel_data_type : GLenum {
	unsigned_byte            	= 0x1401,
	byte                     	= 0x1400,
	unsigned_short           	= 0x1403,
	_short                    	= 0x1402,
	unsigned_int				= 0x1405,
	_int						= 0x1404,
	_float						= 0x1406,
	unsigned_byte_3_3_2        	= 0x8032,
	unsigned_byte_2_3_3_rev    	= 0x8362,
	unsigned_short_5_6_5       	= 0x8363,
	unsigned_short_5_6_5_rev   	= 0x8364,
	unsigned_short_4_4_4_4     	= 0x8033,
	unsigned_short_4_4_4_4_rev 	= 0x8365,
	unsigned_short_5_5_5_1     	= 0x8034,
	unsigned_short_1_5_5_5_rev 	= 0x8366,
	unsigned_int_8_8_8_8       	= 0x8035,
	unsigned_int_8_8_8_8_rev   	= 0x8367,
	unsigned_int_10_10_10_2    	= 0x8036,
	unsigned_int_2_10_10_10_rev	= 0x8368
};

enum class gl_blend_factor : GLenum {
	zero                      	= 0,
	one                       	= 1,
	src_color                 	= 0x0300,
	one_minus_src_color       	= 0x0301,
	dst_color                 	= 0x0306,
	one_minus_dst_color       	= 0x0307,
	src_alpha                 	= 0x0302,
	one_minus_src_alpha       	= 0x0303,
	dst_alpha                 	= 0x0304,
	one_minus_dst_alpha       	= 0x0305,
	constant_color            	= 0x8001,
	one_minus_constant_color  	= 0x8002,
	constant_alpha            	= 0x8003,
	one_minus_constant_alpha  	= 0x8004,
	src_alpha_saturate        	= 0x0308,
	src1_color                	= 0x88F9,
	one_minus_src1_color      	= 0x88FA,
	src1_alpha                	= 0x8589,
	one_minus_src1_alpha      	= 0x88FB
};

enum class gl_capability : GLenum {
	blend						= 0x0BE2,
	color_logic_op             	= 0x0BF2,
	cull_face                  	= 0x0B44,
	debug_output               	= 0x92E0,
	debug_output_synchronous   	= 0x8242,
	depth_clamp                	= 0x864F,
	depth_test                 	= 0x0B71,
	dither                     	= 0x0BD0,
	framebuffer_srgb           	= 0x8DB9,
	line_smooth                	= 0x0B20,
	multisample                	= 0x809D,
	polygon_offset_fill        	= 0x8037,
	polygon_offset_line			= 0x2A02,
	polygon_offset_point		= 0x2A01,
	polygon_smooth				= 0x0B41,
	primitive_restart         	= 0x8F9D,
	primitive_restart_fixed_index=0x8D69,
	rasterizer_discard        	= 0x8C89,
	sample_alpha_to_coverage  	= 0x809E,
	sample_alpha_to_one       	= 0x809F,
	sample_coverage           	= 0x80A0,
	sample_shading            	= 0x8C36,
	sample_mask               	= 0x8E51,
	scissor_test              	= 0x0C11,
	stencil_test              	= 0x0B90,
	texture_cube_map_seamless 	= 0x884F,
	program_point_size        	= 0x8642
};

enum class gl_clear : GLbitfield {
	depth_buffer_bit      		= 0x00000100,
	stencil_buffer_bit    		= 0x00000400,
	color_buffer_bit      		= 0x00004000
};

enum class gl_draw_mode : GLenum {
	points                   	= 0x0000,
	line_strip               	= 0x0003,
	line_loop                	= 0x0002,
	lines                    	= 0x0001,
	line_strip_adjacency     	= 0x000B,
	lines_adjacency          	= 0x000A,
	triangle_strip           	= 0x0005,
	triangle_fan             	= 0x0006,
	triangles                	= 0x0004,
	triangle_strip_adjacency 	= 0x000D,
	triangles_adjacency      	= 0x000C,
	patches                  	= 0x000E
};

enum class gl_index_type : GLenum {
	unsigned_byte            	= 0x1401,
	unsigned_short           	= 0x1403,
	unsigned_int				= 0x1405
};

enum class gl_debug_source : GLenum {
	api               			= 0x8246,
	window_system     			= 0x8247,
	shader_compiler   			= 0x8248,
	third_party       			= 0x8249,
	application       			= 0x824A,
	other             			= 0x824B,
	dont_care					= 0x1100
};

enum class gl_debug_type : GLenum {
	error               		= 0x824C,
	deprecated_behavior 		= 0x824D,
	undefined_behavior  		= 0x824E,
	portability         		= 0x824F,
	performance         		= 0x8250,
	other               		= 0x8251,
	push_group          		= 0x8269,
	pop_group           		= 0x826A,
	marker		              	= 0x8268,
	dont_care					= 0x1100
};

enum class gl_debug_severity : GLenum {
	high            			= 0x9146,
	medium          			= 0x9147,
	low             			= 0x9148,
	notification    			= 0x826B,
	dont_care					= 0x1100
};

enum class gl_shader_type : GLenum {
	compute                 	= 0x91B9,
	vertex                  	= 0x8B31,
	tess_control            	= 0x8E88,
	tess_evaluation         	= 0x8E87,
	geometry                	= 0x8DD9,
	fragment                	= 0x8B30
};

enum class gl_buf_target : GLenum {
	array                   	= 0x8892,
	atomic_counter          	= 0x92C0,
	copy_read               	= 0x8F36,
	copy_write              	= 0x8F37,
	dispatch_indirect       	= 0x90EE,
	draw_indirect           	= 0x8F3F,
	element_array           	= 0x8893,
	pixel_pack              	= 0x88EB,
	pixel_unpack            	= 0x88EC,
	query                   	= 0x9192,
	shader_storage          	= 0x90D2,
	texture                 	= 0x8C2A,
	transform_feedback      	= 0x8C8E,
	uniform                 	= 0x8A11
};

enum class gl_buf_usage : GLenum {
	stream_draw               	= 0x88E0,
	stream_read               	= 0x88E1,
	stream_copy               	= 0x88E2,
	static_draw               	= 0x88E4,
	static_read               	= 0x88E5,
	static_copy               	= 0x88E6,
	dynamic_draw              	= 0x88E8,
	dynamic_read              	= 0x88E9,
	dynamic_copy              	= 0x88EA
};

enum class gl_vert_attrib_type : GLenum {
	byte                     	= 0x1400,	
	unsigned_byte            	= 0x1401,
	_short                    	= 0x1402,	
	unsigned_short           	= 0x1403,
	_int						= 0x1404,	
	unsigned_int				= 0x1405,
	half_float                	= 0x140B,
	_float						= 0x1406,
	_double						= 0x140A,	
	fixed                     	= 0x140C,
	int_2_10_10_10_rev        	= 0x8D9F,
	unsigned_int_2_10_10_10_rev	= 0x8368,
	unsigned_int_10f_11f_11f_rev= 0x8C3B	
};

enum class gl_tex_filter : GLint {
	nearest                   	= 0x2600,
	linear                    	= 0x2601,
	nearest_mipmap_nearest    	= 0x2700,
	linear_mipmap_nearest     	= 0x2701,
	nearest_mipmap_linear     	= 0x2702,
	linear_mipmap_linear      	= 0x2703
};

enum class gl_tex_swizzle : GLint {
	red							= 0x1903,
	green						= 0x1904,
	blue						= 0x1905,
	alpha						= 0x1906,
	zero                      	= 0,
	one                       	= 1
};

enum class gl_tex_wrap : GLint {
	clamp_to_edge				= 0x812F,
	clamp_to_border				= 0x812D,
	mirrored_repeat           	= 0x8370,
	repeat                    	= 0x2901,
	mirror_clamp_to_edge      	= 0x8743
};

enum class gl_shader_param : GLenum {
	delete_status           	= 0x8B80,
	compile_status          	= 0x8B81,
	info_log_length         	= 0x8B84,
	shader_source_length    	= 0x8B88,
	shader_type             	= 0x8B4F
};

enum class wgl_context : int {
	major_version_arb     		= 0x2091,
	minor_version_arb     		= 0x2092,
	flags_arb             		= 0x2094,
	forward_compatible_bit_arb 	= 0x00000002,
	profile_mask_arb      		= 0x9126,
	core_profile_bit_arb  		= 0x00000001
};

DLL_IMPORT const GLubyte* glGetString(gl_info name);

DLL_IMPORT void glDeleteTextures(GLsizei n, const GLuint *textures);
DLL_IMPORT void glTexParameteri(gl_tex_target target, gl_tex_param pname, GLint param);
DLL_IMPORT void glTexParameterfv(gl_tex_target target, gl_tex_param pname, const GLfloat *params);
DLL_IMPORT void glTexImage2D(gl_tex_target target, GLint level, gl_tex_format internalformat, GLsizei width, GLsizei height, GLint border, gl_pixel_data_format format, gl_pixel_data_type type, const GLvoid *pixels);

DLL_IMPORT void glBlendFunc(gl_blend_factor sfactor, gl_blend_factor dfactor);

DLL_IMPORT void glEnable(gl_capability cap);
DLL_IMPORT void glDisable(gl_capability cap);

DLL_IMPORT void glClear(GLbitfield mask);
DLL_IMPORT void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
DLL_IMPORT void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);

DLL_IMPORT void glDrawArrays(gl_draw_mode mode, GLint first, GLsizei count);
DLL_IMPORT void glDrawElements(gl_draw_mode mode, GLsizei count, gl_index_type type, const GLvoid *indices);

DLL_IMPORT void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);

typedef void (*glDebugProc_t)(gl_debug_source source, gl_debug_type type, GLuint id, gl_debug_severity severity, GLsizei length, const GLchar *message, const void *userParam);
typedef void (*glDebugMessageCallback_t)(glDebugProc_t callback, const void *userParam);
typedef void (*glDebugMessageInsert_t)(gl_debug_source source, gl_debug_type type, GLuint id, gl_debug_severity severity, GLsizei length, const char *message);
typedef void (*glDebugMessageControl_t)(gl_debug_source source, gl_debug_type type, gl_debug_severity severity, GLsizei count, const GLuint *ids, gl_bool enabled);

typedef void (*glAttachShader_t)(GLuint program, GLuint shader);
typedef void (*glCompileShader_t)(GLuint shader);
typedef GLuint (*glCreateProgram_t)(void);
typedef GLuint (*glCreateShader_t)(gl_shader_type type);
typedef void (*glDeleteProgram_t)(GLuint program);
typedef void (*glDeleteShader_t)(GLuint shader);
typedef void (*glLinkProgram_t)(GLuint program);
typedef void (*glShaderSource_t)(GLuint shader, GLsizei count, const GLchar* const* str, const GLint* length);
typedef void (*glUseProgram_t)(GLuint program);
typedef void (*glGetShaderiv_t)(GLuint shader, gl_shader_param pname, GLint *params);
typedef void (*glGetShaderInfoLog_t)(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);

typedef GLint (*glGetUniformLocation_t)(GLuint program, const GLchar *name);
typedef void (*glUniformMatrix4fv_t)(GLint location, GLsizei count, gl_bool transpose, const GLfloat *value);

typedef void (*glGenerateMipmap_t)(gl_tex_target target);
typedef void (*glActiveTexture_t)(GLenum texture);
typedef void (*glCreateTextures_t)(gl_tex_target target, GLsizei n, GLuint *textures);
typedef void (*glBindTextureUnit_t)(GLuint unit, GLuint texture);

typedef void (*glBindVertexArray_t)(GLuint array);
typedef void (*glDeleteVertexArrays_t)(GLsizei n, const GLuint *arrays);
typedef void (*glGenVertexArrays_t)(GLsizei n, GLuint *arrays);

typedef void (*glBindBuffer_t)(gl_buf_target target, GLuint buffer);
typedef void (*glDeleteBuffers_t)(GLsizei n, const GLuint *buffers);
typedef void (*glGenBuffers_t)(GLsizei n, GLuint *buffers);
typedef void (*glBufferData_t)(gl_buf_target target, GLsizeiptr size, const void *data, gl_buf_usage usage);

typedef void (*glVertexAttribPointer_t)(GLuint index, GLint size, gl_vert_attrib_type type, gl_bool normalized, GLsizei stride, const void *pointer);
typedef void (*glEnableVertexAttribArray_t)(GLuint index);

typedef void (*glTexParameterIiv_t)(gl_tex_target target, gl_tex_param pname, const GLint *params);

glDebugMessageCallback_t 	glDebugMessageCallback;
glDebugMessageInsert_t		glDebugMessageInsert;
glDebugMessageControl_t		glDebugMessageControl;

glAttachShader_t  			glAttachShader;
glCompileShader_t 			glCompileShader;
glCreateProgram_t 			glCreateProgram;
glCreateShader_t  			glCreateShader;
glDeleteProgram_t 			glDeleteProgram;
glDeleteShader_t  			glDeleteShader;
glLinkProgram_t   			glLinkProgram;
glShaderSource_t  			glShaderSource;
glUseProgram_t    			glUseProgram;
glGetUniformLocation_t 		glGetUniformLocation;
glUniformMatrix4fv_t   		glUniformMatrix4fv;
glGetShaderiv_t 			glGetShaderiv;
glGetShaderInfoLog_t		glGetShaderInfoLog;

glGenerateMipmap_t			glGenerateMipmap;
glActiveTexture_t			glActiveTexture;
glCreateTextures_t			glCreateTextures;
glBindTextureUnit_t			glBindTextureUnit;
glTexParameterIiv_t 		glTexParameterIiv;

glBindVertexArray_t    		glBindVertexArray; 		
glDeleteVertexArrays_t 		glDeleteVertexArrays;
glGenVertexArrays_t    		glGenVertexArrays;

glBindBuffer_t				glBindBuffer;
glDeleteBuffers_t			glDeleteBuffers;
glGenBuffers_t				glGenBuffers;
glBufferData_t				glBufferData;

glVertexAttribPointer_t		glVertexAttribPointer;
glEnableVertexAttribArray_t glEnableVertexAttribArray;
