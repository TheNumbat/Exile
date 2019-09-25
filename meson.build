
project('exile', 'cpp', default_options : ['cpp_std=c++17'])

cc 			= meson.get_compiler('cpp')
host_system = host_machine.system()

project_dir = meson.current_source_dir()
inc_dir 	= include_directories('deps')

# Build meta-program

libclang = []

clang_dir = join_paths(project_dir, 'deps/clang-c')
libclang = cc.find_library('libclang', dirs : clang_dir)

if host_system == 'windows'
	configure_file(input: 'deps/clang-c/libclang.dll', output: 'libclang.dll', copy : true)
endif

meta_compiler = executable('meta', 'src/meta.cpp', 
							include_directories : inc_dir, 
							dependencies : libclang, 
							native : true)

gen_src = custom_target('meta_output',
                        input : 'src/meta_input.h',
                        output : 'meta_types.cpp',
                        build_by_default : true, build_always_stale : true,
                        command : [meta_compiler, '@INPUT@', '@OUTPUT@', '-I../src/', '-I../deps/', '-xc++', '-DRUNNING_META'])

# Build project

sources = [
	'deps/imgui/imgui.cpp',
	'deps/imgui/imgui_draw.cpp',
	'deps/imgui/imgui_widgets.cpp',
	'deps/imgui/imgui_impl_sdl.cpp',
	'deps/imgui/imgui_impl_opengl3.cpp',
	'deps/glad/glad.cpp',
	'src/lib/string.cpp',
	'src/lib/reflect.cpp',
	'src/lib/alloc.cpp',
	'src/platform.cpp',
	'src/engine.cpp',
	'src/gui.cpp',
	'src/main.cpp']

link = []
args = []
deps = []

if host_system == 'windows'

	sdl_dir = join_paths(project_dir, 'deps/sdl/lib')
	deps += cc.find_library('SDL2', dirs : sdl_dir)
	deps += cc.find_library('SDL2main', dirs : sdl_dir)
	configure_file(input: 'deps/sdl/lib/SDL2.dll', output: 'SDL2.dll', copy : true)

elif host_system == 'linux'

	deps += dependency('sdl2')
	link += ['-ldl', '-pthread']

endif

executable('exile', sources,
	dependencies : deps,
	include_directories : inc_dir, 
	cpp_args : args,
	link_args : link)

