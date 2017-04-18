
#import . "fmt.odin";

main :: proc() {

	ints : [500]int;

	for _, idx in ints {
		ints[idx] = 500 - idx;
	}

	radix(ints[..]);

	for i in ints {
		println(i);
	}
}

radix :: proc(ints : []int) {
	
	buckets : [10][dynamic]int;

	for digit in 1..4 {
		for i in ints {
			append(buckets[(i / pow(10, digit - 1)) % 10], i);
		}

		idx := 0;
		for bucket, bdx in buckets {
			for i in bucket {
				ints[idx] = i;
				idx++;
			}
			clear(buckets[bdx]);
		}
	}
}

pow :: proc(num, pow : int) -> int {
	acc := 1;
	for i in 0..pow {
		acc *= num;
	}
	return acc;
}