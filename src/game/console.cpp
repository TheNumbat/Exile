
#include "console.h"
#include "exile.h"

void setup_console_commands() {

	exile->eng->dbg.console.add_command("exit"_, FPTR(console_exit), exile->eng);
	exile->eng->dbg.console.add_command("plight"_, FPTR(console_place_light), &exile->w);
	exile->eng->dbg.console.add_command("rlight"_, FPTR(console_rem_light), &exile->w);
	exile->eng->dbg.console.add_command("block"_, FPTR(console_set_block), &exile->w);
}

CALLBACK void console_exit(string, void* e) {
	((engine*)e)->running = false;
}

CALLBACK void console_place_light(string p, void* w_) {

	world* w = (world*)w_;

	i32 vals[4];
	u32 pos = 0;
	for(i32 i = 0; i < 4; i++) {
		u32 used = 0;
		vals[i] = p.parse_i32(pos, &used);
		pos += used;
	}

	iv3 loc = iv3(vals[0],vals[1],vals[2]);
	u8  val = (u8)vals[3];

	w->place_light(loc,val);
	exile->eng->dbg.console.add_console_msg(string::makef("Placed % light at (%,%,%)."_, val, loc.x, loc.y, loc.z)); //TODO(max): better print rule for the integer vecs
}

CALLBACK void console_rem_light(string p, void* w_) {

	world* w = (world*)w_;

	i32 vals[3];
	u32 pos = 0;
	for(i32 i = 0; i < 3; i++) {
		u32 used = 0;
		vals[i] = p.parse_i32(pos, &used);
		pos += used;
	}

	iv3 loc = iv3(vals[0],vals[1],vals[2]);

	w->rem_light(loc);
	exile->eng->dbg.console.add_console_msg(string::makef("Removed light at (%,%,%)."_, loc.x, loc.y, loc.z));
}

CALLBACK void console_set_block(string p, void* w_) {

	world* w = (world*)w_;

	i32 vals[3];
	u32 pos = 0;
	for(i32 i = 0; i < 3; i++) {
		u32 used = 0;
		vals[i] = p.parse_i32(pos, &used);
		pos += used;
	}

	string name = p.substring(pos, p.len).trim_copy();

	block_id id = string_to_enum<block_id>(name);
	iv3 block(vals[0],vals[1],vals[2]);

	w->set_block(block, id);
	exile->eng->dbg.console.add_console_msg(string::makef("Placed block % at (%,%,%)."_, id, block.x, block.y, block.z));
}
