
#import "platform.odin";
#import "fmt.odin";

#import gl "opengl.odin";
#import . "strings.odin";

test_alloc :: proc(allocator_data: rawptr, mode: Allocator_Mode,
                   size, alignment: int, old_memory: rawptr, old_size: int, 
                   flags: u64) -> rawptr {
	using Allocator_Mode;
	match mode {
		case ALLOC:
			fmt.println("allocating", size, "bytes");
		case FREE:
			fmt.println("freeing", old_memory);
		case RESIZE:
			fmt.println("resizing", size, "bytes at", old_memory);
	}
	return default_allocator_proc(allocator_data, mode, size, alignment, old_memory, 
								  old_size, flags);
}

main :: proc() {

	a := Allocator{test_alloc, nil};
	push_allocator a {

		window, error := platform.make_window("Game", 1280, 720);

		if error != 0 {
			fmt.println("Error in creating window: ", error);
		}
		defer {
			error := platform.destroy_window(window);
			if error != 0 {
				fmt.println("Error in destroying window: ", error);
			}
		} 

		gl.init();

		vendor 		:= to_odin_string(gl.GetString(gl.VENDOR));
		renderer 	:= to_odin_string(gl.GetString(gl.RENDERER));
		version 	:= to_odin_string(gl.GetString(gl.VERSION));

		fmt.println("Vendor:  ", vendor);
		fmt.println("Renderer:", renderer);
		fmt.println("Version: ", version);

		for platform.process_messages(window) {
			gl.ClearColor(0, 0, 0, 1);
			gl.Clear(gl.COLOR_BUFFER_BIT);
			platform.swap_window(window);
			platform.wait();
		}
	}
}
