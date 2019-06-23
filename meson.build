
project('exile', 'cpp')

# To-Do 
# 	shaders as assets
# 	SDL platform layer
#	add compile flags / release / debug
# 	re-write metaprogram
# 	linux/macos build
# 	maybe add testing infra

# Constants

project_dir  = meson.current_source_dir() 
build_dir    = meson.current_build_dir() 

cc = meson.get_compiler('cpp')
host_system = host_machine.system()
inc_dir = include_directories('deps', 'src')

if host_system == 'windows'
	add_global_arguments('/Zc:offsetof-', language : 'cpp')
endif

# Build asset builder

asset_builder = executable('asset', 'src/tools/asset_builder.cpp', 
							include_directories : inc_dir, 
							native : true)

custom_target('assets',
  			   output : 'game.asset',
  			   input : 'data/assets/game.txt',
  			   build_by_default: true,
  			   command : [asset_builder, '@INPUT@', '@OUTPUT@'])

# Build meta-program

clang_dir = join_paths(project_dir, 'deps/clang-c')

libclang = cc.find_library('libclang', dirs : clang_dir)

meta_compiler = executable('meta', 'src/tools/meta.cpp', 
							include_directories : inc_dir, 
							dependencies : libclang, 
							native : true)

clang_dll = configure_file(
	input: 'deps/clang-c/libclang.dll',
	output: 'libclang.dll',
	copy : true)

gen_src = custom_target('meta_output',
                        input : 'src/meta_input.h',
                        output : 'meta_types.cpp',
                        command : [meta_compiler, '@INPUT@'])

# Build Game DLL

imgui_sources = [
	'deps/imgui/imgui.cpp',
	'deps/imgui/imgui_draw.cpp',
	'deps/imgui/imgui_demo.cpp']
engine_sources = [
	'src/engine/util/fptr.cpp',
	'src/engine/util/context.cpp',
	'src/engine/util/threadstate.cpp',
	'src/engine/util/reflect.cpp',
	'src/engine/log.cpp',
	'src/engine/events.cpp',
	'src/engine/render.cpp',
	'src/engine/dbg.cpp',
	'src/engine/asset.cpp',
	'src/engine/threads.cpp',
	'src/engine/engine.cpp',
	'src/engine/imgui.cpp',
	'src/engine/ds/alloc.cpp',
	'src/engine/ds/string.cpp',
	'src/engine/math.cpp',
	'src/engine/platform/gl.cpp']
game_sources = [
	'src/game/console.cpp',
	'src/game/gfx.cpp',
	'src/game/world.cpp',
	'src/game/exile.cpp', 
	gen_src]

game_dependencies = []
if host_system == 'windows'
	game_dependencies += cc.find_library('opengl32')
endif

shared_library('exile', [game_sources, imgui_sources, engine_sources], 
	include_directories : inc_dir,
	dependencies : game_dependencies)

# Build main EXE

main_sources = ['src/engine/platform/platform_main.cpp', 'src/engine/platform/platform_strings.cpp']
main_dependencies = []

if host_system == 'windows'
	main_dependencies += cc.find_library('opengl32')
	main_sources += 'src/engine/platform/windows/platform_win32.cpp'
endif

executable('main', main_sources,
	dependencies : main_dependencies)
