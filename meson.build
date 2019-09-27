
project('exile', 'cpp', default_options : ['cpp_std=c++17'])

# Linux deps : libclang-dev, libsdl2-dev

project_dir  = meson.current_source_dir() 
build_dir    = meson.current_build_dir() 

cc = meson.get_compiler('cpp')
host_system = host_machine.system()
inc_dir = include_directories('deps', 'build')

if host_system == 'linux'
	assert(cc.get_id() == 'clang', 'only clang++ supported on linux')
endif

cpp_args = []

if host_system == 'windows'
	cpp_args += ['-fp:fast', '-GR-', '-EHa-', '-Oi', 
			     '-MP', '-FC', '-D_HAS_EXCEPTIONS=0']
elif host_system == 'linux'
	cpp_args += ['-msse4.1', '-Wno-format', '-Wno-unknown-pragmas']
endif

# Build meta-program

libclang = []

clang_dir = join_paths(project_dir, 'deps/clang-c')
libclang = cc.find_library('libclang', dirs : clang_dir)

if host_system == 'windows'
	configure_file(input: 'deps/clang-c/libclang.dll', output: 'libclang.dll', copy : true)
endif

meta_compiler = executable('meta', 'src/tools/meta.cpp', 
							include_directories : inc_dir, 
							dependencies : libclang, 
							native : true, cpp_args : cpp_args)

gen_src = custom_target('meta_output',
                        input : 'src/meta_input.h',
                        output : 'meta_types.h',
                        build_by_default : true, build_always_stale : true,
                        command : [meta_compiler, '@INPUT@', '@OUTPUT@', '-I../src/', '-I../deps/', '-xc++', '-DRUNNING_META', cpp_args])

sources = [
	'deps/imgui/imgui.cpp',
	'deps/imgui/imgui_draw.cpp',
	'deps/imgui/imgui_widgets.cpp',
	'deps/imgui/imgui_impl_sdl.cpp',
	'deps/imgui/imgui_impl_opengl3.cpp',
	'deps/glad/glad.cpp',
	'src/lib/string.cpp',
	'src/lib/alloc.cpp',
	'src/platform.cpp',
	'src/engine.cpp',
	'src/gui.cpp',
	'src/main.cpp']

dependencies = [declare_dependency(sources : gen_src)]

if host_system == 'windows'
	sdl_dir = join_paths(project_dir, 'deps/SDL2/lib')
	dependencies += cc.find_library('SDL2', dirs : sdl_dir)
	dependencies += cc.find_library('SDL2main', dirs : sdl_dir)
	configure_file(input: 'deps/SDL2/lib/SDL2.dll', output: 'SDL2.dll', copy : true)
elif host_system == 'linux'
	dependencies += dependency('sdl2')
	dependencies += cc.find_library('dl')
endif

executable('exile', sources,
	dependencies : dependencies,
	include_directories : inc_dir,
	cpp_args : cpp_args)


