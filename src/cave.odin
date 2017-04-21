
#import . "fmt.odin";

main :: proc()  {
	println("Hello");
=======
main :: proc() {

	when true {
		output :: proc() {
			println("output");
		}
	}

	output();
}
