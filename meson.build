
project('exile', 'cpp', default_options : ['cpp_std=c++17'])

cc 			= meson.get_compiler('cpp')
host_system = host_machine.system()

project_dir = meson.current_source_dir()
inc_dir 	= include_directories('deps/imgui', 'deps/stb', 'deps/glad')

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

link = []
args = []
deps = []

if host_system == 'windows'

	inc_dir = include_directories('deps', 'deps/imgui', 'deps/stb', 'deps/glad')
	sdl_dir = join_paths(project_dir, 'deps/SDL2/lib')
	deps += cc.find_library('SDL2', dirs : sdl_dir)
	deps += cc.find_library('SDL2main', dirs : sdl_dir)
	configure_file(input: 'deps/SDL2/lib/SDL2.dll', output: 'SDL2.dll', copy : true)

elif host_system == 'linux'

	deps += dependency('sdl2')
	link += ['-ldl', '-pthread']

endif

executable('exile', sources,
	dependencies : deps,
	include_directories : inc_dir, 
	cpp_args : args,
	link_args : link)

