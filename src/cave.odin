
#import . "fmt.odin";

<<<<<<< HEAD
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
>>>>>>> 8b35ff5345c2316b9f99fbf8ee93f64de8e76dfa
}
