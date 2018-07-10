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
typedef long long int GLint64;

enum class gl_bool : GLboolean {
	_false						= 0,
	_true 						= 1
};

enum class gl_info : GLenum {
	vendor                  	= 0x1F00,
	renderer                	= 0x1F01,
	version                 	= 0x1F02,
	extensions              	= 0x1F03,
	shading_language_version	= 0x8B8C,
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
	rectangle              		= 0x84F5,

	cube_map_positive_x 		= 0x8515,
	cube_map_negative_x 		= 0x8516,
	cube_map_positive_y 		= 0x8517,
	cube_map_negative_y 		= 0x8518,
	cube_map_positive_z 		= 0x8519,
	cube_map_negative_z 		= 0x851A
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
	forward_compatible_bit_arb 	= 0x0002,
	profile_mask_arb      		= 0x9126,
	core_profile_bit_arb  		= 0x0001,
	sample_buffers_arb          = 0x2041,
	samples_arb                 = 0x2042,
	draw_to_window_arb          = 0x2001,
	support_opengl_arb          = 0x2010,
	double_buffer_arb           = 0x2011,
	pixel_type_arb              = 0x2013,
	color_bits_arb              = 0x2014,
	depth_bits_arb              = 0x2022,
	stencil_bits_arb            = 0x2023,
	alpha_bits_arb              = 0x201B,
	type_rgba_arb               = 0x202B
};

enum class gl_get : GLenum {
	active_texture                            = 0x84E0,
	aliased_line_width_range                  = 0x846E,
	array_buffer_binding                      = 0x8894,
	blend                                     = 0x0BE2,
	blend_color                               = 0x8005,
	blend_dst_alpha                           = 0x80CA,
	blend_dst_rgb                             = 0x80C8,
	blend_equation_rgb                        = 0x8009,
	blend_equation_alpha                      = 0x883D,
	blend_src_alpha                           = 0x80CB,
	blend_src_rgb                             = 0x80C9,
	color_clear_value                         = 0x0C22,
	color_logic_op                            = 0x0BF2,
	color_writemask                           = 0x0C23,
	compressed_texture_formats                = 0x86A3,
	max_compute_shader_storage_blocks         = 0x90DB,
	max_combined_shader_storage_blocks        = 0x90DC,
	max_compute_uniform_blocks                = 0x91BB,
	max_compute_texture_image_units           = 0x91BC,
	max_compute_uniform_components            = 0x8263,
	max_compute_atomic_counters               = 0x8265,
	max_compute_atomic_counter_buffers        = 0x8264,
	max_combined_compute_uniform_components   = 0x8266,
	max_compute_work_group_invocations        = 0x90EB,
	max_compute_work_group_count              = 0x91BE,
	max_compute_work_group_size               = 0x91BF,
	dispatch_indirect_buffer_binding          = 0x90EF,
	max_debug_group_stack_depth               = 0x826C,
	debug_group_stack_depth                   = 0x826D,
	context_flags                             = 0x821E,
	cull_face                                 = 0x0B44,
	current_program                           = 0x8B8D,
	depth_clear_value                         = 0x0B73,
	depth_func                                = 0x0B74,
	depth_range                               = 0x0B70,
	depth_writemask                           = 0x0B72,
	dither                                    = 0x0BD0,
	doublebuffer                              = 0x0C32,
	draw_buffer                               = 0x0C01,
	draw_buffer0                              = 0x8825,
	draw_buffer1                              = 0x8826,
	draw_buffer2                              = 0x8827,
	draw_buffer3                              = 0x8828,
	draw_buffer4                              = 0x8829,
	draw_buffer5                              = 0x882A,
	draw_buffer6                              = 0x882B,
	draw_buffer7                              = 0x882C,
	draw_buffer8                              = 0x882D,
	draw_buffer9                              = 0x882E,
	draw_buffer10                             = 0x882F,
	draw_buffer11                             = 0x8830,
	draw_buffer12                             = 0x8831,
	draw_buffer13                             = 0x8832,
	draw_buffer14                             = 0x8833,
	draw_buffer15                             = 0x8834,
	draw_framebuffer_binding                  = 0x8CA6,
	read_framebuffer_binding                  = 0x8CAA,
	element_array_buffer_binding              = 0x8895,
	fragment_shader_derivative_hint           = 0x8B8B,
	implementation_color_read_format          = 0x8B9B,
	implementation_color_read_type            = 0x8B9A,
	line_smooth                               = 0x0B20,
	line_smooth_hint                          = 0x0C52,
	line_width                                = 0x0B21,
	layer_provoking_vertex                    = 0x825E,
	logic_op_mode                             = 0x0BF0,
	major_version                             = 0x821B,
	max_3d_texture_size                       = 0x8073,
	max_array_texture_layers                  = 0x88FF,
	max_clip_distances                        = 0x0D32,
	max_color_texture_samples                 = 0x910E,
	max_combined_atomic_counters              = 0x92D7,
	max_combined_fragment_uniform_components  = 0x8A33,
	max_combined_geometry_uniform_components  = 0x8A32,
	max_combined_texture_image_units          = 0x8B4D,
	max_combined_uniform_blocks               = 0x8A2E,
	max_combined_vertex_uniform_components    = 0x8A31,
	max_cube_map_texture_size                 = 0x851C,
	max_depth_texture_samples                 = 0x910F,
	max_draw_buffers                          = 0x8824,
	max_dual_source_draw_buffers              = 0x88FC,
	max_elements_indices                      = 0x80E9,
	max_elements_vertices                     = 0x80E8,
	max_fragment_atomic_counters              = 0x92D6,
	max_fragment_shader_storage_blocks        = 0x90DA,
	max_fragment_input_components             = 0x9125,
	max_fragment_uniform_components           = 0x8B49,
	max_fragment_uniform_vectors              = 0x8DFD,
	max_fragment_uniform_blocks               = 0x8A2D,
	max_framebuffer_width                     = 0x9315,
	max_framebuffer_height                    = 0x9316,
	max_framebuffer_layers                    = 0x9317,
	max_framebuffer_samples                   = 0x9318,
	max_geometry_atomic_counters              = 0x92D5,
	max_geometry_shader_storage_blocks        = 0x90D7,
	max_geometry_input_components             = 0x9123,
	max_geometry_output_components            = 0x9124,
	max_geometry_texture_image_units          = 0x8C29,
	max_geometry_uniform_blocks               = 0x8A2C,
	max_integer_samples                       = 0x9110,
	min_map_buffer_alignment                  = 0x90BC,
	max_label_length                          = 0x82E8,
	max_program_texel_offset                  = 0x8905,
	max_rectangle_texture_size                = 0x84F8,
	max_renderbuffer_size                     = 0x84E8,
	max_sample_mask_words                     = 0x8E59,
	max_server_wait_timeout                   = 0x9111,
	max_shader_storage_buffer_bindings        = 0x90DD,
	max_tess_control_atomic_counters          = 0x92D3,
	max_tess_evaluation_atomic_counters       = 0x92D4,
	max_tess_control_shader_storage_blocks    = 0x90D8,
	max_tess_evaluation_shader_storage_blocks = 0x90D9,
	max_texture_buffer_size                   = 0x8C2B,
	max_texture_image_units                   = 0x8872,
	max_texture_lod_bias                      = 0x84FD,
	max_texture_size                          = 0x0D33,
	max_uniform_buffer_bindings               = 0x8A2F,
	max_uniform_block_size                    = 0x8A30,
	max_uniform_locations                     = 0x826E,
	max_varying_components                    = 0x8B4B,
	max_varying_vectors                       = 0x8DFC,
	max_varying_floats                        = 0x8B4B,
	max_vertex_atomic_counters                = 0x92D2,
	max_vertex_attribs                        = 0x8869,
	max_vertex_shader_storage_blocks          = 0x90D6,
	max_vertex_texture_image_units            = 0x8B4C,
	max_vertex_uniform_components             = 0x8B4A,
	max_vertex_uniform_vectors                = 0x8DFB,
	max_vertex_output_components              = 0x9122,
	max_vertex_uniform_blocks                 = 0x8A2B,
	max_viewport_dims                         = 0x0D3A,
	max_viewports                             = 0x825B,
	minor_version                             = 0x821C,
	num_compressed_texture_formats            = 0x86A2,
	num_extensions                            = 0x821D,
	num_program_binary_formats                = 0x87FE,
	num_shader_binary_formats                 = 0x8DF9,
	pack_alignment                            = 0x0D05,
	pack_image_height                         = 0x806C,
	pack_lsb_first                            = 0x0D01,
	pack_row_length                           = 0x0D02,
	pack_skip_images                          = 0x806B,
	pack_skip_pixels                          = 0x0D04,
	pack_skip_rows                            = 0x0D03,
	pack_swap_bytes                           = 0x0D00,
	pixel_pack_buffer_binding                 = 0x88ED,
	pixel_unpack_buffer_binding               = 0x88EF,
	point_fade_threshold_size                 = 0x8128,
	primitive_restart_index                   = 0x8F9E,
	program_binary_formats                    = 0x87FF,
	program_pipeline_binding                  = 0x825A,
	program_point_size                        = 0x8642,
	provoking_vertex                          = 0x8E4F,
	point_size                                = 0x0B11,
	point_size_granularity                    = 0x0B13,
	point_size_range                          = 0x0B12,
	polygon_offset_factor                     = 0x8038,
	polygon_offset_units                      = 0x2A00,
	polygon_offset_fill                       = 0x8037,
	polygon_offset_line                       = 0x2A02,
	polygon_offset_point                      = 0x2A01,
	polygon_smooth                            = 0x0B41,
	polygon_smooth_hint                       = 0x0C53,
	read_buffer                               = 0x0C02,
	renderbuffer_binding                      = 0x8CA7,
	sample_buffers                            = 0x80A8,
	sample_coverage_value                     = 0x80AA,
	sample_coverage_invert                    = 0x80AB,
	sample_mask_value                         = 0x8E52,
	sampler_binding                           = 0x8919,
	samples                                   = 0x80A9,
	scissor_box                               = 0x0C10,
	scissor_test                              = 0x0C11,
	shader_compiler                           = 0x8DFA,
	shader_storage_buffer_binding             = 0x90D3,
	shader_storage_buffer_offset_alignment    = 0x90DF,
	shader_storage_buffer_start               = 0x90D4,
	shader_storage_buffer_size                = 0x90D5,
	smooth_line_width_range                   = 0x0B22,
	smooth_line_width_granularity             = 0x0B23,
	stencil_back_fail                         = 0x8801,
	stencil_back_func                         = 0x8800,
	stencil_back_pass_depth_fail              = 0x8802,
	stencil_back_pass_depth_pass              = 0x8803,
	stencil_back_ref                          = 0x8CA3,
	stencil_back_value_mask                   = 0x8CA4,
	stencil_back_writemask                    = 0x8CA5,
	stencil_clear_value                       = 0x0B91,
	stencil_fail                              = 0x0B94,
	stencil_func                              = 0x0B92,
	stencil_pass_depth_fail                   = 0x0B95,
	stencil_pass_depth_pass                   = 0x0B96,
	stencil_ref                               = 0x0B97,
	stencil_test                              = 0x0B90,
	stencil_value_mask                        = 0x0B93,
	stencil_writemask                         = 0x0B98,
	stereo                                    = 0x0C33,
	subpixel_bits                             = 0x0D50,
	texture_binding_1d                        = 0x8068,
	texture_binding_1d_array                  = 0x8C1C,
	texture_binding_2d                        = 0x8069,
	texture_binding_2d_array                  = 0x8C1D,
	texture_binding_2d_multisample            = 0x9104,
	texture_binding_2d_multisample_array      = 0x9105,
	texture_binding_3d                        = 0x806A,
	texture_binding_buffer                    = 0x8C2C,
	texture_binding_cube_map                  = 0x8514,
	texture_binding_cube_map_array            = 0x900A,
	texture_binding_rectangle                 = 0x84F6,
	texture_compression_hint                  = 0x84EF,
	texture_buffer_offset_alignment           = 0x919F,
	timestamp                                 = 0x8E28,
	transform_feedback_buffer_binding         = 0x8C8F,
	transform_feedback_buffer_start           = 0x8C84,
	transform_feedback_buffer_size            = 0x8C85,
	uniform_buffer_binding                    = 0x8A28,
	uniform_buffer_offset_alignment           = 0x8A34,
	uniform_buffer_size                       = 0x8A2A,
	uniform_buffer_start                      = 0x8A29,
	unpack_alignment                          = 0x0CF5,
	unpack_image_height                       = 0x806E,
	unpack_lsb_first                          = 0x0CF1,
	unpack_row_length                         = 0x0CF2,
	unpack_skip_images                        = 0x806D,
	unpack_skip_pixels                        = 0x0CF4,
	unpack_skip_rows                          = 0x0CF3,
	unpack_swap_bytes                         = 0x0CF0,
	vertex_array_binding                      = 0x85B5,
	vertex_binding_divisor                    = 0x82D6,
	vertex_binding_offset                     = 0x82D7,
	vertex_binding_stride                     = 0x82D8,
	max_vertex_attrib_relative_offset         = 0x82D9,
	max_vertex_attrib_bindings                = 0x82DA,
	viewport                                  = 0x0BA2,
	viewport_bounds_range                     = 0x825D,
	viewport_index_provoking_vertex           = 0x825F,
	viewport_subpixel_bits                    = 0x825C,
	max_element_index                         = 0x8D6B
};	

enum class gl_poly : GLenum {
	front_and_back = 0x0408
};

enum class gl_cull_mode : GLenum {
	front 		   = 0x0404,
	back 		   = 0x0405,
	front_and_back = 0x0408
};

enum class gl_poly_mode : GLenum {
	point = 0x1B00,
	line  = 0x1B01,
	fill  = 0x1B02
};

enum class gl_blend_mode : GLenum {
	add                   = 0x8006,
	subtract              = 0x800A,
	reverse_subtract      = 0x800B,
	min                   = 0x8007,
	max                   = 0x8008
};

enum class gl_depth_factor : GLenum {
	never    = 0x0200,
	less     = 0x0201,
	equal    = 0x0202,
	lequal   = 0x0203,
	greater  = 0x0204,
	notequal = 0x0205,
	gequal   = 0x0206,
	always   = 0x0207
};

enum class gl_pix_store : GLenum {
	pack_alignment      = 0x0D05,
	pack_lsb_first      = 0x0D01,
	pack_row_length     = 0x0D02,
	pack_skip_pixels    = 0x0D04,
	pack_skip_rows      = 0x0D03,
	pack_swap_bytes     = 0x0D00,
	pack_image_height   = 0x806C,
	pack_skip_images    = 0x806B,
	unpack_alignment    = 0x0CF5,
	unpack_lsb_first    = 0x0CF1,
	unpack_skip_images  = 0x806D,
	unpack_image_height = 0x806E,
	unpack_row_length   = 0x0CF2,
	unpack_skip_pixels  = 0x0CF4,
	unpack_skip_rows    = 0x0CF3,
	unpack_swap_bytes   = 0x0CF0
};

DLL_IMPORT const GLubyte* glGetString(gl_info name);

DLL_IMPORT void glDeleteTextures(GLsizei n, const GLuint *textures);
DLL_IMPORT void glTexParameteri(gl_tex_target target, gl_tex_param pname, GLint param);
DLL_IMPORT void glTexParameterfv(gl_tex_target target, gl_tex_param pname, const GLfloat *params);
DLL_IMPORT void glTexImage2D(gl_tex_target target, GLint level, gl_tex_format internalformat, GLsizei width, GLsizei height, GLint border, gl_pixel_data_format format, gl_pixel_data_type type, const GLvoid *pixels);
DLL_IMPORT void glTexParameteriv(gl_tex_target target, gl_tex_param pname, const GLint *params);
DLL_IMPORT void glTexParameterf(gl_tex_target target, gl_tex_param pname, GLfloat param);
DLL_IMPORT void glBindTexture(gl_tex_target target, GLuint handle);
DLL_IMPORT void glGenTextures(GLsizei n, GLuint* textures);

DLL_IMPORT void glBlendFunc(gl_blend_factor sfactor, gl_blend_factor dfactor);
DLL_IMPORT void glDepthFunc(gl_depth_factor factor);
typedef void (*glBlendEquation_t)(gl_blend_mode mode);

DLL_IMPORT void glEnable(gl_capability cap);
DLL_IMPORT void glDisable(gl_capability cap);

DLL_IMPORT void glClear(GLbitfield mask);
DLL_IMPORT void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
DLL_IMPORT void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
DLL_IMPORT void glDepthMask(gl_bool enable);

DLL_IMPORT void glDrawArrays(gl_draw_mode mode, GLint first, GLsizei count);
DLL_IMPORT void glDrawElements(gl_draw_mode mode, GLsizei count, gl_index_type type, const GLvoid *indices);

DLL_IMPORT void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
DLL_IMPORT void glCullFace(gl_cull_mode mode);
DLL_IMPORT void glPolygonMode(gl_poly face, gl_poly_mode mode);
DLL_IMPORT void glLineWidth(GLfloat width);

typedef gl_bool (*glIs_t)(GLuint handle);
typedef void (*glMinSampleShading_t)(GLfloat value);

DLL_IMPORT void glGetBooleanv(gl_get name, GLboolean* data);
DLL_IMPORT void glGetDoublev(gl_get name, GLdouble* data);
DLL_IMPORT void glGetFloatv(gl_get name, GLfloat* data);
DLL_IMPORT void glGetIntegerv(gl_get name, GLint* data);
typedef void (*glGetInteger64v_t)(gl_get name, GLint64* data);
typedef void (*glGetBooleani_v_t)(gl_get name, GLuint idx, GLboolean* data);
typedef void (*glGetDoublei_v_t)(gl_get name, GLuint idx, GLdouble* data);
typedef void (*glGetFloati_v_t)(gl_get name, GLuint idx, GLfloat* data);
typedef void (*glGetIntegeri_v_t)(gl_get name, GLuint idx, GLint* data);
typedef void (*glGetInteger64i_v_t)(gl_get name, GLuint idx, GLint64* data);
typedef const GLubyte* (*glGetStringi_t)(gl_info name, GLuint idx);

typedef void (*glDrawElementsInstanced_t)(gl_draw_mode mode, GLsizei count,	gl_index_type type,	const void *indices, GLsizei primcount);
typedef void (*glDrawElementsInstancedBaseVertex_t)(gl_draw_mode mode, GLsizei count, gl_index_type type, GLvoid *indices, GLsizei primcount, GLint basevertex);
typedef void (*glVertexAttribDivisor_t)(GLuint index, GLuint divisor);

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
typedef void (*glGetShaderSource_t)(GLuint shader, GLsizei buffer, GLsizei* length, GLchar* source);

typedef GLint (*glGetUniformLocation_t)(GLuint program, const GLchar *name);
typedef GLint (*glGetAttribLocation_t)(GLuint program, 	const GLchar *name);
typedef void (*glUniformMatrix4fv_t)(GLint location, GLsizei count, gl_bool transpose, const GLfloat *value);
typedef void (*glUniform1f_t)(GLint location, GLfloat f);
typedef void (*glUniform1i_t)(GLint location, GLint i);

typedef void (*glGenerateMipmap_t)(gl_tex_target target);
typedef void (*glActiveTexture_t)(GLenum texture);
typedef void (*glCreateTextures_t)(gl_tex_target target, GLsizei n, GLuint *textures);
typedef void (*glBindTextureUnit_t)(GLuint unit, GLuint texture);
typedef void (*glTexStorage3D_t)(gl_tex_target target, GLsizei levels, gl_tex_format format, GLsizei w, GLsizei h, GLsizei z);
typedef void (*glTexSubImage3D_t)(gl_tex_target target, GLint level, GLint xoff, GLint yoff, GLint zoff, GLsizei w, GLsizei h, GLsizei z, gl_pixel_data_format format, gl_pixel_data_type type, void* data);
typedef void (*glBindSampler_t)(GLuint unit, GLuint sampler);
DLL_IMPORT void glPixelStorei(gl_pix_store type, GLint param);

typedef void (*glBindVertexArray_t)(GLuint array);
typedef void (*glDeleteVertexArrays_t)(GLsizei n, const GLuint *arrays);
typedef void (*glGenVertexArrays_t)(GLsizei n, GLuint *arrays);

typedef void (*glBindBuffer_t)(gl_buf_target target, GLuint buffer);
typedef void (*glDeleteBuffers_t)(GLsizei n, const GLuint *buffers);
typedef void (*glGenBuffers_t)(GLsizei n, GLuint *buffers);
typedef void (*glBufferData_t)(gl_buf_target target, GLsizeiptr size, const void *data, gl_buf_usage usage);

typedef void (*glVertexAttribPointer_t)(GLuint index, GLint size, gl_vert_attrib_type type, gl_bool normalized, GLsizei stride, const void *pointer);
typedef void (*glVertexAttribIPointer_t)(GLuint index, GLint size, gl_vert_attrib_type type, GLsizei stride, const void *pointer);
typedef void (*glEnableVertexAttribArray_t)(GLuint index);
typedef void (*glDrawElementsBaseVertex_t)(gl_draw_mode mode, GLsizei count, gl_index_type type, const GLvoid *indices, GLint base);

typedef void (*glTexParameterIiv_t)(gl_tex_target target, gl_tex_param pname, const GLint *params);

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

glDrawElementsInstanced_t 			glDrawElementsInstanced;
glDrawElementsInstancedBaseVertex_t glDrawElementsInstancedBaseVertex;
glVertexAttribDivisor_t				glVertexAttribDivisor;

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
glGetShaderiv_t 			glGetShaderiv;
glGetShaderInfoLog_t		glGetShaderInfoLog;
glGetShaderSource_t 		glGetShaderSource;
glMinSampleShading_t		glMinSampleShading;

glGetUniformLocation_t 		glGetUniformLocation;
glGetAttribLocation_t		glGetAttribLocation;
glUniformMatrix4fv_t   		glUniformMatrix4fv;
glUniform1f_t				glUniform1f;
glUniform1i_t				glUniform1i;

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

glVertexAttribPointer_t		glVertexAttribPointer;
glVertexAttribIPointer_t	glVertexAttribIPointer;
glEnableVertexAttribArray_t glEnableVertexAttribArray;
glDrawElementsBaseVertex_t	glDrawElementsBaseVertex;
