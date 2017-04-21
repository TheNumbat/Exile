
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
	e : node.End;

	println(n.visited);
	println(n.children);
	println(e.visited);
	println(e.msg);

	a : any = n;
	println(a);

	match b in cast(node)n {
		case node.Branch: println("branch");
		case node.End:	println("end");
	}
}

