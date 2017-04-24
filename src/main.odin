
#import "platform.odin";
#import "fmt.odin";
#import gl "opengl.odin";
#import . "strings.odin";

main :: proc() {
	
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

	fmt.println("Vendor:   ", vendor);
	fmt.println("Renderer: ", renderer);
	fmt.println("Version:  ", version);

	for platform.process_messages(window) {
		gl.ClearColor(0, 0, 0, 1);
		gl.Clear(gl.COLOR_BUFFER_BIT);
		platform.swap_window(window);
		platform.wait();
	}
}
