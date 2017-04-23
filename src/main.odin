
#import "platform.odin";
#import "fmt.odin";

main :: proc() {
	
	window, error := platform.make_window("Game", 1280, 720);
	
	if(error != 0) {
		fmt.println("Failed to create window! Error: ", error);
	}
	defer platform.destroy_window(window);

	fmt.println(window);

	for platform.process_messages(window) {}
}
