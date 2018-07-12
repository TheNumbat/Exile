
void setup_console_commands() {

	eng->dbg.console.add_command("exit"_, FPTR(console_cmd_ext), eng);
	eng->dbg.console.add_command("echo"_, FPTR(console_cmd_ext), eng);
}

CALLBACK void console_cmd_ext(string, void* e) {
	((engine*)e)->running = false;
}
