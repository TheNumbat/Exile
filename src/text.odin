
#import . "fmt.odin";

node :: union {
	visited : bool,
	parent : ^node,

	Branch {
		options : [dynamic]string,
		children : [dynamic]node,
	},

	End {
		msg : string,
	},
}

main :: proc() {
	
	n : node.Branch;

	println(n.visited);
	println(n.children);

	match n {

	}

}

