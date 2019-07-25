
project('exile', 'cpp', default_options : ['cpp_std=c++14'])

# Linux deps : libclang-dev, libsdl2-dev

# Constants & Setup

project_dir  = meson.current_source_dir() 
build_dir    = meson.current_build_dir() 

cc = meson.get_compiler('cpp')
host_system = host_machine.system()
inc_dir = include_directories('deps', 'src')

if host_system == 'linux'
	assert(cc.get_id() == 'clang', 'only clang++ supported on linux')
endif

game_cpp_args = []
main_cpp_args = []
all_cpp_args = []

if get_option('buildtype') != 'release'
	if get_option('bounds_check') 
		game_cpp_args += '-DBOUNDS_CHECK' 
	endif
	if get_option('block_on_err') 
		game_cpp_args += '-DBLOCK_OR_EXIT_ON_ERROR' 
	endif
	if get_option('zero_arena') 
		game_cpp_args += '-DZERO_ARENA' 
	endif
	if get_option('check_plt') 
		game_cpp_args += '-DCHECK_PLT' 
	endif
	if get_option('profile') 
		game_cpp_args += '-DPROFILE' 
	endif
	if get_option('fast_close') 
		game_cpp_args += '-DFAST_CLOSE' 
	endif
	if get_option('gl_check') 
		game_cpp_args += '-DGL_CHECKS' 
	endif
	if get_option('leak_check') 
		main_cpp_args += '-DCHECK_NO_LEAKS' 
	endif
endif

if get_option('platform') == 'win32'
	all_cpp_args += '-DPLATFORM_WIN32'
elif get_option('platform') == 'sdl'
	all_cpp_args += '-DPLATFORM_SDL'
endif

if host_system == 'windows'
	game_cpp_args += ['-Zc:offsetof-', '-fp:fast', '-GR-', '-EHa-', '-Oi', 
			          '-MP', '-FC', '-D_HAS_EXCEPTIONS=0']
	all_cpp_args += ['-wd4201', '-wd4100']
elif host_system == 'linux'
	all_cpp_args += ['-fdeclspec', '-Wno-format-security', '-Wno-non-pod-varargs', 
					 '-Wno-unused-function', '-Wno-unused-const-variable', 
					 '-Wno-format', '-Wno-invalid-offsetof', '-msse4.1']
endif

# Build asset builder

asset_builder = executable('asset', 'src/tools/asset_builder.cpp', 
							include_directories : inc_dir, 
							native : true, cpp_args : all_cpp_args)

custom_target('assets',
  			   output : 'game.asset',
  			   input : 'data/assets/game.txt',
  			   build_by_default: true,
  			   command : [asset_builder, '@INPUT@', '@OUTPUT@'])

run_command('cp', 'build/game.asset', 'data/game.asset')

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
							native : true, cpp_args : all_cpp_args)

gen_src = custom_target('meta_output',
                        input : 'src/meta_input.h',
                        output : 'meta_types.cpp',
                        build_by_default : true, build_always_stale : true,
                        command : [meta_compiler, '@INPUT@', '@OUTPUT@', '-I../src/', '-I../deps/', '-xc++', '-DRUNNING_META', all_cpp_args])

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
elif host_system == 'linux'
	game_dependencies += dependency('gl')
endif

shared_library('exile', [game_sources, imgui_sources, engine_sources], 
	include_directories : inc_dir,
	dependencies : game_dependencies,
	cpp_args : [all_cpp_args, game_cpp_args])

# Build main EXE

main_sources = ['src/engine/platform/platform_main.cpp', 'src/engine/platform/platform_strings.cpp']
main_dependencies = []

if host_system == 'windows'
	main_dependencies += cc.find_library('opengl32')
endif

if get_option('platform') == 'win32'
	main_sources += 'src/engine/platform/windows/platform_win32.cpp'
elif get_option('platform') == 'sdl'
	main_sources += 'src/engine/platform/SDL/platform_SDL.cpp'

	if host_system == 'windows'
		sdl_dir = join_paths(project_dir, 'deps/SDL2/lib')
		main_dependencies += cc.find_library('SDL2', dirs : sdl_dir)
		main_dependencies += cc.find_library('SDL2main', dirs : sdl_dir)
		configure_file(input: 'deps/SDL2/lib/SDL2.dll', output: 'SDL2.dll', copy : true)
	elif host_system == 'linux'
		main_dependencies += dependency('sdl2')
	endif
endif

executable('main', main_sources,
	dependencies : main_dependencies,
	include_directories : inc_dir,
	cpp_args : [main_cpp_args, all_cpp_args])
