
#import . "fmt.odin";

main :: proc() {

	when true {
		output :: proc() {
			println("output");
		}
	}

	output();
}
