
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>

using namespace std;

struct entry {
	entry() {}
	entry(string _f, int _o) {f = _f; o = _o;}
	string f;
	int o;
};
vector<entry> occurs;

int main(int argc, char** argv) {

	ifstream in(argv[1]);

	while(in.good()) {
		string line;
		getline(in, line);
		if(!in.good()) break;

		auto trim = line.find_first_not_of(" ");
		line = line.substr(trim, line.size() - trim);

		auto itr = find_if(occurs.begin(), occurs.end(), [line](const entry& e) -> bool {return e.f == line;});
		if(itr == occurs.end()) {
			occurs.push_back(entry(line, 1));
		} else {
			itr->o++;
		}
	}

	sort(occurs.begin(), occurs.end(), [](const entry& l, const entry& r) -> bool {return l.o > r.o;});

	int total = 0;
	for(auto& e : occurs) {
		total += e.o;
	}
	for(auto& e : occurs) {
		float percent = 100 * e.o / (float)total;
		printf("%-*s%-*d%f%%\n", 150, e.f.c_str(), 15, e.o, percent);
	}

	return 0;
}