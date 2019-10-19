
project('exile', 'cpp', default_options : ['cpp_std=c++17'])

# Linux deps : libsdl2-dev

project_dir  = meson.current_source_dir() 
build_dir    = meson.current_build_dir() 

cc = meson.get_compiler('cpp')
host_system = host_machine.system()

inc_dir = include_directories('deps', 'build')
if host_system == 'windows'
	inc_dir = include_directories('deps', 'build', 'deps/win')
endif

if host_system == 'linux'
	assert(cc.get_id() == 'clang', 'only clang++ supported on linux')
endif

cpp_args = []

if host_system == 'windows'
	cpp_args += ['-fp:fast', '-GR-', '-EHa-', '-Oi', 
				 '-MP', '-FC', '-D_HAS_EXCEPTIONS=0']
elif host_system == 'linux'
	cpp_args += ['-msse4.1', '-Wno-format', '-Wno-unknown-pragmas', '-Wno-missing-braces']
endif

sources = [
	'deps/imgui/imgui_compile.cpp',
	'deps/glad/glad.cpp',

	'src/platform.cpp',
	'src/engine.cpp',
	'src/gui.cpp',
	'src/main.cpp']

dependencies = []

if host_system == 'windows'
	sdl_dir = join_paths(project_dir, 'deps/win/SDL2/lib')
	dependencies += cc.find_library('SDL2', dirs : sdl_dir)
	dependencies += cc.find_library('SDL2main', dirs : sdl_dir)
	configure_file(input: 'deps/win/SDL2/lib/SDL2.dll', output: 'SDL2.dll', copy : true)
elif host_system == 'linux'
	dependencies += cc.find_library('dl')
	dependencies += cc.find_library('SDL2')
endif

executable('exile', sources,
	dependencies : dependencies,
	include_directories : inc_dir,
	cpp_args : cpp_args)


