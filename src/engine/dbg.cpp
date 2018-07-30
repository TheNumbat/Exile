
dbg_profiler dbg_profiler::make(allocator* alloc) { PROF

	dbg_profiler ret;

	ret.alloc = alloc;

	ret.thread_stats = map<platform_thread_id, thread_profile*>::make(global_api->get_num_cpus(), alloc);
	ret.alloc_stats  = map<allocator, alloc_profile*>::make(32, alloc);

	ret.selected_thread = global_api->this_thread_id();
	global_api->create_mutex(&ret.stats_map_mut, false);
	global_api->create_mutex(&ret.alloc_map_mut, false);

	return ret;
}

void dbg_profiler::destroy() { PROF

	PUSH_ALLOC(alloc);

	FORMAP(it, thread_stats) {
		it->value->destroy();
		free(it->value, sizeof(thread_profile));
	}
	thread_stats.destroy();
	global_api->destroy_mutex(&stats_map_mut);

	FORMAP(it, alloc_stats) {
		it->value->destroy();
		free(it->value, sizeof(alloc_profile));
	}
	alloc_stats.destroy();
	global_api->destroy_mutex(&alloc_map_mut);

	POP_ALLOC();
}

dbg_value_store dbg_value_store::make(allocator* alloc) { PROF

	dbg_value_store ret;

	ret.alloc = alloc;
	ret.value_store = dbg_value::make_sec(alloc);

	return ret;
}

void dbg_value_store::destroy() { PROF

	value_store.destroy(alloc);
}

void dbg_console::add_command(string input, _FPTR* func, void* param) { PROF

	console_cmd cmd;
	cmd.func.set(func);
	cmd.param = param;

	string name = string::make(input.len - 1, alloc);
	name.len = name.cap;
	_memcpy(input.c_str, name.c_str, name.len);

	commands.insert(name, cmd);
}

CALLBACK void console_cmd_clear(string, void* data) { PROF

	dbg_console* console = (dbg_console*)data;
	console->clear();
}

void dbg_console::clear() { PROF

	FORQ_BEGIN(it, lines) {
		if(it->lvl == log_level::console) {
			it->msg.destroy(alloc);
		} else {
			DESTROY_ARENA(&it->arena);
		}
	} FORQ_END(it, lines);
	lines.clear();
}

void dbg_console::init(allocator* a) { PROF

	alloc = a;

	history    = vector<string>::make(64, alloc);
	candidates = vector<string>::make(128, alloc);
	lines 	   = queue<console_msg>::make(4096, alloc);
	commands   = map<string, console_cmd>::make(128, alloc);

	add_command("clear"_, FPTR(console_cmd_clear), this);

	global_api->create_mutex(&lines_mut, false);
}

void dbg_console::destroy() { PROF

	// imgui pls
	filter.Filters.~ImVector<ImGuiTextFilter::TextRange>();

	FORMAP(it, commands) {
		it->key.destroy(alloc);
	}
	commands.destroy();
	candidates.destroy();

	FORVEC(it, history) {
		it->destroy(alloc);
	}
	history.destroy();

	clear();
	lines.destroy();
	global_api->destroy_mutex(&lines_mut);
}

void dbg_manager::init(allocator* alloc) { PROF

	console.init(alloc);
	store = dbg_value_store::make(alloc);
	profiler = dbg_profiler::make(alloc);
}

void dbg_manager::toggle_profile() { PROF
	show_profile = !show_profile;
}

void dbg_manager::toggle_vars() { PROF
	show_vars = !show_vars;
}

void dbg_manager::toggle_console() { PROF
	show_console = !show_console;
}

void alloc_frame_profile::destroy() { PROF

	allocs.destroy();
}

void frame_profile::destroy() { PROF
	
	DESTROY_POOL(&pool);
	FORMAP(a, allocations) {
		a->value.destroy();
	}
	allocations.destroy();
}

void thread_profile::destroy() { PROF

	FORQ_BEGIN(f, frames) {
		
		f->destroy();

	} FORQ_END(f, frames);

	frames.destroy();
	global_api->destroy_mutex(&mut);
}

void dbg_manager::destroy() { PROF

	store.destroy();
	console.destroy();

	profiler.collate_allocs();
	profiler.print_remaining();
}

void dbg_profiler::print_remaining() { PROF

	alloc_profile totals = get_totals();
	
	LOG_INFO_F("% allocations remaining at debug shutdown"_, totals.num_allocs - totals.num_frees);
	FORMAP(it, alloc_stats) {
		FORMAP(a, it->value->current_set) {
			LOG_DEBUG_F("\t% bytes in % @ %:%"_, a->value.size, it->key.name(), a->value.last_loc.file(), a->value.last_loc.line);
		}
	}
}

void dbg_manager::destroy_prof() { PROF
	profiler.destroy();
}

void dbg_profiler::recurse(vector<profile_node*> list) { PROF

	switch(prof_sort) {
	case prof_sort_type::none: break;
	case prof_sort_type::name: {
		list.stable_sort(prof_sort_name);
	} break;
	case prof_sort_type::heir: {
		list.stable_sort(prof_sort_heir);
	} break;
	case prof_sort_type::self: {
		list.stable_sort(prof_sort_self);
	} break;
	case prof_sort_type::calls: {
		list.stable_sort(prof_sort_calls);
	} break;
	}

	FORVEC(it, list) {
		ImGui::PushID(__it);

		profile_node* node = *it;

		if(node->children.size) {
			bool enabled = ImGui::TreeNode(node->context.c_function);
			ImGui::NextColumn();
			ImGui::Text("%d", node->heir);
			ImGui::NextColumn();
			ImGui::Text("%d", node->self);
			ImGui::NextColumn();
			ImGui::Text("%d", node->calls);
			ImGui::NextColumn();
			if(enabled) {
				recurse(node->children);
				ImGui::TreePop();
			}
		} else {
			ImGui::Indent();
			ImGui::Text(node->context.c_function);
			ImGui::Unindent();
			ImGui::NextColumn();
			ImGui::Text("%d", node->heir);
			ImGui::NextColumn();
			ImGui::Text("%d", node->self);
			ImGui::NextColumn();
			ImGui::Text("%d", node->calls);
			ImGui::NextColumn();
		}

		ImGui::PopID();
	}
}

bool operator<=(addr_info l, addr_info r) { PROF 

	return r.size <= l.size;
}

alloc_profile dbg_profiler::get_totals() { PROF

	alloc_profile ret;

	global_api->aquire_mutex(&alloc_map_mut);
	FORMAP(a, alloc_stats) {
		ret.current_size += a->value->current_size;
		ret.total_allocated += a->value->total_allocated;
		ret.total_freed += a->value->total_freed;
		ret.num_allocs += a->value->num_allocs;
		ret.num_frees += a->value->num_frees;
		ret.num_reallocs += a->value->num_reallocs;
	}
	global_api->release_mutex(&alloc_map_mut);

	return ret;
}

void dbg_manager::UI(platform_window* window) { PROF

	if(!show_ui) return;

	if(show_profile)
		profiler.UI(window);
	if(show_vars)
		store.UI(window);
	if(show_console)
		console.UI(window);
}

void dbg_value_store::recurse(map<string, dbg_value> store) { PROF

	FORMAP(it, store) {
		switch(it->value.type) {
		case dbg_value_class::section: {
			if(ImGui::TreeNodeNoNull(it->key)) {
				recurse(it->value.sec.children);
				ImGui::TreePop();
			}
		} break;
		case dbg_value_class::edit: {
			ImGui::Edit_T(it->key, it->value.edit.value, it->value.edit.info());
		} break;
		case dbg_value_class::view: {
			ImGui::View_T(it->key, it->value.view.value, it->value.view.info());
		} break;
		case dbg_value_class::callback: {
			it->value.cal.callback(it->value.cal.callback_param);
		} break;
		}
	}
}

void dbg_value_store::UI(platform_window* window) { PROF

	ImGui::Begin("Debug Vars", null, ImGuiWindowFlags_AlwaysAutoResize);
	recurse(value_store.sec.children);
	ImGui::End();
}

i32 dbg_console_text_edit(ImGuiTextEditCallbackData* data) { PROF

	dbg_console* console = (dbg_console*)data->UserData;
	console->on_text_edit(data);
	return 0;
}

void dbg_console::on_text_edit(ImGuiTextEditCallbackData* data) { PROF

	switch(data->EventFlag) {
	case ImGuiInputTextFlags_CallbackCompletion: {

		// Find beginning of current word
		char* end = data->Buf + data->CursorPos;
		char* begin = end;
		while(begin > data->Buf) {
			char c = begin[-1];
			if (c == ' ' || c == '\t' || c == ',' || c == ';') break;
			begin--;
		}
		string word = string::from_c_str(begin, end);

		candidates.clear();

		FORMAP(it, commands) {
			if(it->key.starts_with_insensitive(word)) {
				candidates.push(it->key);
			}
		}

		if(candidates.size == 1) {
            
            data->DeleteChars((i32)(begin - data->Buf), (i32)(end - begin));
            data->InsertChars(data->CursorPos, candidates[0].c_str);
            data->InsertChars(data->CursorPos, " ");

		} else if(candidates.size > 1) {
			
			i32 len = word.len;
			for(;;) {
				char cur_char = 0;
				bool all_match = true;

				for(u32 i = 0; i < candidates.size; i++) {
					if(i == 0) {
						
						cur_char = uppercase(candidates[i].c_str[len]);

					} else if(cur_char == 0 || cur_char != uppercase(candidates[i].c_str[len])) {

						all_match = false;
					}
				}

				if(!all_match) break;
				len++;
			}

			if(len > 0) {
                data->DeleteChars((i32)(begin - data->Buf), (i32)(end - begin));
                data->InsertChars(data->CursorPos, candidates[0].c_str, candidates[0].c_str + len);
			}
		}

	} break;
	case ImGuiInputTextFlags_CallbackCharFilter: {
		
		if(data->EventChar == ' ') candidates.clear();

	} break;
	case ImGuiInputTextFlags_CallbackHistory: {
		
		i32 prev_idx = history_idx;
		if(data->EventKey == ImGuiKey_UpArrow) {
			
			if(history_idx == -1) history_idx = history.size - 1;
			else if(history_idx > 0) history_idx--;

		} else if(data->EventKey == ImGuiKey_DownArrow) {

			if(history_idx != -1 && ++history_idx >= (i32)history.size) history_idx = -1;
		}

		if(prev_idx != history_idx) {

			string history_str = history_idx >= 0 ? history[history_idx] : ""_;

			data->CursorPos = data->SelectionStart = data->SelectionEnd = data->BufTextLen = history_str.len - 1;
			_memset(data->Buf, data->BufSize, 0);
			_memcpy(history_str.c_str, data->Buf, max(data->BufSize, (i32)history_str.len - 1));
			
			data->BufDirty = true;
		}

	} break;
	}
}

void dbg_console::add_console_msg(string line) { PROF

	console_msg msg;
	msg.msg = string::make_copy(line, alloc);
	lines.push(msg);
}

void dbg_console::exec_command(string input) { PROF 

	string name = input.first_word_no_term();
	if(!name.len) return;

	console_cmd* cmd = commands.try_get(name);

	string* pos = history.find(input);
	if(pos) {
		pos->destroy(alloc);
		u32 idx = (u32)(pos - history.memory);
		history.erase(idx);
	}
	history_idx = -1;

	if(history.size == history.capacity) history.pop();
	history.push(string::make_copy(input, alloc));
	
	if(!cmd) {
		add_console_msg(input);
		return;
	}

	cmd->func(input.trim_first_word(), cmd->param);
}

void dbg_console::UI(platform_window* window) { PROF

	f32 w, h;
	i32 win_w, win_h;
	global_api->get_window_drawable(window, &win_w, &win_h);
	w = (f32)win_w; h = (f32)win_h;

	ImGui::SetNextWindowPos({0.0f, floor(h * 0.65f)});
	ImGui::SetNextWindowSize({w, ceil(h * 0.35f)});
	ImGui::Begin("Console"_, null, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings);

	f32 footer = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("Scroll", ImVec2(0, -footer), false, ImGuiWindowFlags_HorizontalScrollbar);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,1));

    if(copy_clipboard) {
    	ImGui::LogToClipboard();
    }

    global_api->aquire_mutex(&lines_mut);
	FORQ_BEGIN(it, lines) {

		if((u8)it->lvl >= (u8)base_level) {

			string level = enum_to_string(it->lvl);
		
			if(filter.PassFilter(level) || filter.PassFilter(it->msg)) {

				ImVec4 col = ImGui::GetStyleColorVec4(ImGuiCol_Text);
				switch(it->lvl) {
				case log_level::debug: col = col * 0.7f; break;
				case log_level::console: col = ImColor(1.0f,0.78f,0.58f,1.0f); break;
				case log_level::warn: col = ImColor(1.0f,0.4f,0.4f,1.0f); break;
				case log_level::error: col = ImColor(1.0f,0.0f,0.0f,1.0f); break;
				}

				ImGui::PushStyleColor(ImGuiCol_Text, col);
				ImGui::Text("[%s] ", level.c_str);
				if(ImGui::IsItemClicked() && it->call_stack.capacity) {
					code_context* last = it->call_stack.get(it->call_stack.capacity - 1);
					CHECKED(shell_exec, string::makef("subl_remote_open.bat %:%"_, last->path(), last->line));
				}
				if(ImGui::IsItemHovered() && it->lvl != log_level::console) {
					ImGui::PopStyleColor();
					ImGui::BeginTooltip();
					ImGui::TextUnformatted(it->thread);
					FORARR(loc, it->call_stack) {
						ImGui::TextUnformatted(loc->c_function);
					}
					ImGui::EndTooltip();
					ImGui::PushStyleColor(ImGuiCol_Text, col);
				}

				ImGui::SameLine(0, 0);
				ImGui::TextUnformatted(it->msg);
				ImGui::PopStyleColor();
			}
		}
	
	} FORQ_END(it, lines);
	global_api->release_mutex(&lines_mut);

	if(copy_clipboard) {
		ImGui::LogFinish();
		copy_clipboard = false;
	}
	if(scroll_bottom) {
		ImGui::SetScrollHere(1.0f);
		scroll_bottom = false;
	}

	ImGui::PopStyleVar();
	ImGui::EndChild();
	ImGui::Separator();

	ImGui::Columns(4);

	ImGui::SetColumnWidth(0, w * 0.40f);
	ImGui::SetColumnWidth(1, w * 0.30f);
	ImGui::SetColumnWidth(2, w * 0.15f);
	ImGui::SetColumnWidth(3, w * 0.15f);

	bool reclaim_focus = false;
	if(ImGui::InputText("Input", input_buffer, 1024, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter | 
						ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory, &dbg_console_text_edit, this)) {
		
		string input = string::from_c_str(input_buffer).trim_no_term();
		if (input.len) {
		    candidates.clear();
		    exec_command(input);
		    scroll_bottom = true;
		}
		_memset(input_buffer, 1024, 0);
		reclaim_focus = true;
	}

	ImGui::SetItemDefaultFocus();
	if (reclaim_focus)
	    ImGui::SetKeyboardFocusHere(-1);

	bool focused = ImGui::IsItemActive();

	ImGui::NextColumn();

	filter.Draw("Filter (inc,-exc)", 180);

	ImGui::NextColumn();

	ImGui::EnumCombo("Level"_, &base_level);

	ImGui::NextColumn();

	if(ImGui::Button("Clear")) {
		clear();
		scroll_bottom = true;
	}
	ImGui::SameLine();
	if(ImGui::Button("Bottom")) {
		scroll_bottom = true;
	}
	ImGui::SameLine();
	if(ImGui::Button("Copy")) {
		copy_clipboard = true;
	}

	ImGui::Columns(1);
	ImGui::End();

	if(!focused) {
		candidates.clear();
	}
	if(focused && candidates.size > 1) {
		
		f32 element = 2.0f * ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();

		ImGui::SetNextWindowFocus();
		ImGui::Begin("Completions", null, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_AlwaysAutoResize);
		FORVEC(it, candidates) {
			ImGui::TextUnformatted(*it);
		}

		f32 tip_h = ImGui::GetWindowHeight();
		ImGui::SetWindowPos({0, h - tip_h - element}, ImGuiCond_Always);

		ImGui::End();
	}
}

void dbg_profiler::UI(platform_window* window) { PROF

	ImGui::SetNextWindowSize({800, 600}, ImGuiCond_Once);
	ImGui::Begin("Profile"_, null, ImGuiWindowFlags_NoSavedSettings);
	
	ImGui::Text(string::makef("FPS: %"_, 1.0f / last_frame_time));
	ImGui::Checkbox("Pause"_, &frame_pause);

	global_api->aquire_mutex(&alloc_map_mut);
	if(ImGui::TreeNodeEx("Allocation Stats"_, ImGuiTreeNodeFlags_Framed)) {

		alloc_profile totals = get_totals();
		ImGui::Text(string::makef("Total size: %, total allocs: %, total frees: %"_, totals.current_size, totals.num_allocs, totals.num_frees));

		FORMAP(it, alloc_stats) {
			global_api->aquire_mutex(&it->value->mut);
			if(ImGui::TreeNode(string::makef("%: size: %, allocs: %, frees: %"_, it->key.name(), it->value->current_size, it->value->num_allocs, it->value->num_frees))) {

				vector<addr_info> allocs = vector<addr_info>::make(it->value->current_set.size);
				FORMAP(a, it->value->current_set) {
					allocs.push(a->value);
				}
				
				allocs.stable_sort();
				
				FORVEC(a, allocs) {
					ImGui::Text(string::makef("% bytes @ %:%"_, a->size, a->last_loc.file(), a->last_loc.line));
				}

				allocs.destroy();
				ImGui::TreePop();
			}
			global_api->release_mutex(&it->value->mut);
		}

		ImGui::TreePop();
	}
	global_api->release_mutex(&alloc_map_mut);

	global_api->aquire_mutex(&stats_map_mut);
	map<string, platform_thread_id> threads = map<string, platform_thread_id>::make(global_api->get_num_cpus());
	FORMAP(it, thread_stats) {
		threads.insert(it->value->name, it->key);
	}
	ImGui::MapCombo("Select Thread"_, threads, &selected_thread);
	threads.destroy();

	thread_profile* thread = *thread_stats.get(selected_thread);
	global_api->release_mutex(&stats_map_mut);
	
	global_api->aquire_mutex(&thread->mut);

	ImGui::PushID(thread->name);
	ImGui::SliderInt("Buffer Position: "_, &thread->selected_frame, 1, thread->frame_buf_size);

	if(thread->frames.len()) {
		
		frame_profile* frame = thread->frames.get(thread->selected_frame - 1);

		f32 frame_time = (f32)(frame->perf_end - frame->perf_start) / (f32)global_api->get_perfcount_freq() * 1000.0f;
		ImGui::Text(string::makef("Frame: %, Time: %ms"_, frame->number, frame_time));

		ImGui::PushID(frame->number);		
		
		if(ImGui::TreeNodeEx("Profile"_, ImGuiTreeNodeFlags_DefaultOpen)) {

			ImGui::EnumCombo("Sort By: "_, &prof_sort);

			ImGui::Columns(4);
			ImGui::SetColumnWidth(0, 300);
			ImGui::SetColumnWidth(1, 75);
			ImGui::SetColumnWidth(2, 75);
			ImGui::SetColumnWidth(3, 75);
			ImGui::Text("Function"_);
			ImGui::NextColumn();
			ImGui::Text("Heir"_);
			ImGui::NextColumn();
			ImGui::Text("Self"_);
			ImGui::NextColumn();
			ImGui::Text("Calls"_);
			ImGui::NextColumn();
			ImGui::Separator();
			recurse(frame->heads);
			ImGui::Columns(1);

			ImGui::TreePop();
		}

		if(ImGui::TreeNode("Allocations"_)) {

			FORMAP(it, frame->allocations) {
				if(ImGui::TreeNode(it->key.name())) {
					FORVEC(msg, it->value.allocs) {
						switch(msg->type) {
						case dbg_msg_type::allocate: {
							ImGui::Text(string::makef("% bytes @ %:%"_, msg->allocate.bytes, msg->context.file(), msg->context.line));
						} break;
						case dbg_msg_type::reallocate: {
							ImGui::Text(string::makef("% bytes re@ %:%"_, msg->reallocate.to_bytes, msg->context.file(), msg->context.line));
						} break;
						case dbg_msg_type::free: {
							ImGui::Text(string::makef("free @ %:%"_, msg->context.file(), msg->context.line));
						} break;
						}
					}
					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		ImGui::PopID();
	}

	global_api->release_mutex(&thread->mut);
	ImGui::PopID();
	ImGui::End();
}

void dbg_console::shutdown_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::debug;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->rem_custom_output(dbg_log);
}

void dbg_console::setup_log(log_manager* log) { PROF
	log_out dbg_log;
	dbg_log.level = log_level::debug;
	dbg_log.type = log_out_type::custom;
	dbg_log.write.set(FPTR(dbg_add_log));
	dbg_log.param = this;
	log->add_custom_output(dbg_log);
}

void dbg_profiler::register_thread(u32 frames) { PROF

	PUSH_ALLOC(alloc);

	thread_profile* thread = NEW(thread_profile);
	thread->frame_buf_size = frames;
	thread->frames = queue<frame_profile>::make(frames, alloc);
	thread->name = this_thread_data.name;
	global_api->create_mutex(&thread->mut, false);

	POP_ALLOC();

	global_api->aquire_mutex(&stats_map_mut);
	thread_stats.insert(global_api->this_thread_id(), thread);
	global_api->release_mutex(&stats_map_mut);
}

void frame_profile::setup(string name, allocator* alloc, clock time, u64 p, u32 num) { PROF

	pool = MAKE_POOL(name, KILOBYTES(8), alloc, false);
	heads = vector<profile_node*>::make(2, &pool);
	allocations = map<allocator, alloc_frame_profile>::make(8, alloc);
	clock_start = time;
	perf_start = p;
	number = num;
}

alloc_frame_profile alloc_frame_profile::make(allocator* alloc) { PROF

	alloc_frame_profile ret;
	
	ret.allocs = vector<dbg_msg>::make(8, alloc);

	return ret;
}

bool operator>(dbg_msg& l, dbg_msg& r) { PROF
	return l.time < r.time;
}

void dbg_profiler::collate() { PROF
	
	PUSH_PROFILE(false) {
		
		collate_allocs();
		collate_timings();

	} POP_PROFILE();

	this_thread_data.dbg_queue.clear();
}

void dbg_profiler::collate_allocs() { PROF

	FORQ_BEGIN(msg, this_thread_data.dbg_queue) {

		if(msg->type == dbg_msg_type::allocate || msg->type == dbg_msg_type::reallocate || msg->type == dbg_msg_type::free) {
			process_alloc_msg(msg);
		}

	} FORQ_END(msg, this_thread_data.dbg_queue);
}

void dbg_profiler::collate_timings() { PROF

	bool got_a_frame = false;
	u64 frame_perf_start = 0;

	global_api->aquire_mutex(&stats_map_mut);
	thread_profile* thread = *thread_stats.get(global_api->this_thread_id());
	global_api->release_mutex(&stats_map_mut);

	global_api->aquire_mutex(&thread->mut);

	FORQ_BEGIN(msg, this_thread_data.dbg_queue) {

		if(msg->type == dbg_msg_type::begin_frame) {
		
			frame_perf_start = msg->begin_frame.perf;
			thread->num_frames++;

			if(thread->frames.full()) {
				if(!frame_pause) {
					thread->frames.pop().destroy();
					got_a_frame = true;
				}
			} else {
				got_a_frame = true;
			}

			if(got_a_frame) {
				frame_profile* frame = thread->frames.push(frame_profile());
				string name = string::makef("frame %"_, thread->num_frames);
				frame->setup(name, alloc, msg->time, msg->begin_frame.perf, thread->num_frames);
				name.destroy();
			}
		}

		frame_profile* frame = thread->frames.back();
		if(frame && got_a_frame) {

			PUSH_ALLOC(&frame->pool) {
				switch(msg->type) {
				case dbg_msg_type::enter_func: {

					if(!frame->current) { 	
						bool found_repeat_head = false;
						FORVEC(head, frame->heads) {
							if((*head)->context.function() == msg->context.function()) {
								frame->current = *head;
								found_repeat_head = true;
							}
						}
						if(!found_repeat_head) {
							frame->current = *frame->heads.push(NEW(profile_node));
							frame->current->children = vector<profile_node*>::make(4);
							frame->current->context = msg->context;
						}
						frame->current->begin = msg->time;
						frame->current->calls++;
						break;
					}

					profile_node* here = frame->current;
					FORVEC(node, here->children) {
						if((*node)->context.function() == msg->context.function()) {
							here = *node;
							break;
						} 
					}
					if(here == frame->current) {
						profile_node* new_node = *here->children.push(NEW(profile_node));
						new_node->children = vector<profile_node*>::make(4);
						new_node->parent = here;
						here = new_node;
					}
					
					here->context = msg->context;
					here->calls++;
					here->begin = msg->time;
					frame->current = here;

				} break;

				case dbg_msg_type::exit_func: {

					clock runtime = msg->time - frame->current->begin;
					
					frame->current->heir += runtime;
					frame->current->begin = 0;
					frame->current = frame->current->parent;

				} break;

				case dbg_msg_type::end_frame: {
					frame->clock_end = msg->time;
					frame->perf_end = msg->end_frame.perf;

					FORVEC(it, frame->heads) {
						fixdown_self_timings(*it);
					}

					got_a_frame = false;
				} break;

				case dbg_msg_type::allocate: 
				case dbg_msg_type::reallocate:
				case dbg_msg_type::free: {

					process_frame_alloc_msg(frame, msg);

				} break;
				}
			} POP_ALLOC();
		}

		if(msg->type == dbg_msg_type::end_frame) {
			last_frame_time = (f32)(msg->end_frame.perf - frame_perf_start) / (f32)global_api->get_perfcount_freq();
		}

	} FORQ_END(msg, this_thread_data.dbg_queue);

	global_api->release_mutex(&thread->mut);
}

alloc_profile alloc_profile::make(allocator* alloc) { PROF

	alloc_profile ret;

	ret.current_set = map<void*, addr_info>::make(64, alloc);
	global_api->create_mutex(&ret.mut, false);

	return ret;
}

alloc_profile* alloc_profile::make_new(allocator* alloc) { PROF

	PUSH_ALLOC(alloc);

	alloc_profile* ret = NEW(alloc_profile);

	ret->current_set = map<void*, addr_info>::make(64, alloc);
	global_api->create_mutex(&ret->mut, false);

	POP_ALLOC();

	return ret;
}

void alloc_profile::destroy() { PROF

	current_set.destroy();
	global_api->destroy_mutex(&mut);
}

void dbg_profiler::process_frame_alloc_msg(frame_profile* frame, dbg_msg* msg) { PROF

	allocator a;
	switch(msg->type) {
	case dbg_msg_type::allocate: 	a = msg->allocate.alloc; break;
	case dbg_msg_type::reallocate: 	a = msg->reallocate.alloc; break;
	case dbg_msg_type::free: 		a = msg->free.alloc; break;
	}

	alloc_frame_profile* fprofile = frame->allocations.try_get(a);

	if(!fprofile) {

		fprofile = frame->allocations.insert(a, alloc_frame_profile::make(alloc));
	}
	fprofile->allocs.push(*msg);
}

void dbg_profiler::process_alloc_msg(dbg_msg* msg) { PROF

	allocator a;
	switch(msg->type) {
	case dbg_msg_type::allocate: 	a = msg->allocate.alloc; break;
	case dbg_msg_type::reallocate: 	a = msg->reallocate.alloc; break;
	case dbg_msg_type::free: 		a = msg->free.alloc; break;
	}
	
	global_api->aquire_mutex(&alloc_map_mut);
	alloc_profile** p = alloc_stats.try_get(a);
	if(!p) {
		p = alloc_stats.insert(a, alloc_profile::make_new(alloc));
	}
	alloc_profile* profile = *p;
	global_api->release_mutex(&alloc_map_mut);

	global_api->aquire_mutex(&profile->mut);
	switch(msg->type) {
	case dbg_msg_type::allocate: {

		addr_info* info = profile->current_set.try_get(msg->allocate.to);
		if(!info) {
			info = profile->current_set.insert(msg->allocate.to, addr_info());
		}

		info->last_loc = msg->context;
		info->size += msg->allocate.bytes;

		profile->current_size += msg->allocate.bytes;
		profile->total_allocated += msg->allocate.bytes;
		profile->num_allocs++;

		if(info->size == 0) {
			profile->current_set.erase(msg->allocate.to);
		}

	} break;
	case dbg_msg_type::reallocate: {

		addr_info* from_info = profile->current_set.try_get(msg->reallocate.from);
		if(!from_info) {
			from_info = profile->current_set.insert(msg->reallocate.from, addr_info());
		}
		addr_info* to_info = profile->current_set.try_get(msg->reallocate.to);
		if(!to_info) {
			to_info = profile->current_set.insert(msg->reallocate.to, addr_info());
			from_info = profile->current_set.get(msg->reallocate.from);
		}

		from_info->last_loc = msg->context;
		from_info->size -= msg->reallocate.from_bytes;

		profile->current_size -= msg->reallocate.from_bytes;
		profile->total_freed += msg->reallocate.from_bytes;

		if(from_info->size == 0) {
			profile->current_set.erase(msg->reallocate.from);
		}

		to_info->last_loc = msg->context;
		to_info->size += msg->reallocate.to_bytes;

		profile->current_size += msg->reallocate.to_bytes;
		profile->total_allocated += msg->reallocate.to_bytes;
		profile->num_reallocs++;

		if(to_info->size == 0) {
			profile->current_set.erase(msg->reallocate.to);
		}

	} break;
	case dbg_msg_type::free: {

		addr_info* info = profile->current_set.try_get(msg->free.from);
		if(!info) {
			info = profile->current_set.insert(msg->free.from, addr_info());
		}

		info->last_loc = msg->context;
		info->size -= msg->free.bytes;

		profile->current_size -= msg->free.bytes;
		profile->total_freed += msg->free.bytes;
		profile->num_frees++;

		if(info->size == 0) {
			profile->current_set.erase(msg->free.from);
		}
		
	} break;
	}
	global_api->release_mutex(&profile->mut);
}

void dbg_profiler::fixdown_self_timings(profile_node* node) { PROF

	clock children = 0;
	FORVEC(it, node->children) {
		fixdown_self_timings(*it);
		children += (*it)->heir;
	}

	node->self = node->heir - children;
}

void dbg_value_store::add_ele(string path, _FPTR* callback, void* param) { PROF

	dbg_value* value = &value_store;
	path.len--;

	for(;;) {
		
		string key;
		i32 slash = path.first_slash();
		if(slash != -1) {
			key  = path.substring(0, (u32)slash - 1);
			path = path.substring((u32)slash + 1, path.len);
		} else {
			key = path;
			if(!value->sec.children.try_get(key))
				value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_cal(callback, param));
			break;
		}

		dbg_value* next = value->sec.children.try_get(key);
		if(!next) value = value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_sec(alloc));
		else if (next->type != dbg_value_class::section) break;
		else value = next;
	}
}

template<typename T>
void dbg_value_store::add_val(string path, T* val) { PROF

	dbg_value* value = &value_store;
	path.len--;

	for(;;) {
		
		string key;
		i32 slash = path.first_slash();
		if(slash != -1) {
			key  = path.substring(0, (u32)slash - 1);
			path = path.substring((u32)slash + 1, path.len);
		} else {
			key = path;
			if(!value->sec.children.try_get(key))
				value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_view(any::make(val)));
			break;
		}

		dbg_value* next = value->sec.children.try_get(key);
		if(!next) value = value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_sec(alloc));
		else if (next->type != dbg_value_class::section) break;
		else value = next;
	}
}

template<typename T>
void dbg_value_store::add_var(string path, T* val) { PROF

	dbg_value* value = &value_store;
	path.len--;

	for(;;) {
		
		string key;
		i32 slash = path.first_slash();
		if(slash != -1) {
			key  = path.substring(0, (u32)slash - 1);
			path = path.substring((u32)slash + 1, path.len);
		} else {
			key = path;
			if(!value->sec.children.try_get(key))
				value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_edit(any::make(val)));
			break;
		}

		dbg_value* next = value->sec.children.try_get(key);
		if(!next) value = value->sec.children.insert(string::make_copy(key, alloc), dbg_value::make_sec(alloc));
		else if (next->type != dbg_value_class::section) break;
		else value = next;
	}
}

template<typename T>
T dbg_value_store::get_var(string path) { PROF

	dbg_value* value = &value_store;
	path.len--;

	for(;;) {
		
		string key;
		i32 slash = path.first_slash();
		if(slash != -1) {
			key  = path.substring(0, (u32)slash - 1);
			path = path.substring((u32)slash + 1, path.len);
		} else {
			key = path;
			value = value->children.try_get(key);
			break;
		}

		value = value->children.try_get(key);
		if(!value) return null;
	}

	LOG_DEBUG_ASSERT(value->type == dbg_value_class::value);
	LOG_DEBUG_ASSERT(value->info == TYPEINFO(T));
	return *(T*)value->value;
}

CALLBACK void dbg_reup_window(void* e) { PROF

	engine* eng = (engine*)e;
	if(ImGui::Button("Apply Settings")) {

		eng->apply_window_settings = true;
	}
}

CALLBACK void dbg_add_log(log_message* msg, void* param) { PROF

	dbg_console* console = (dbg_console*)param;

	global_api->aquire_mutex(&console->lines_mut);

	if(console->lines.len() == console->lines.capacity) {

		console_msg* m = console->lines.front();
		DESTROY_ARENA(&m->arena);
		console->lines.pop();
	}

	console->scroll_bottom = true;

	console_msg* m = console->lines.push(console_msg());
	
	m->lvl = msg->level;
	m->arena = MAKE_ARENA("cons"_, msg->arena.size, console->alloc);
	
	m->msg = string::make_copy(msg->msg, &m->arena);
	m->thread = string::make_copy(msg->thread_name, &m->arena);
	m->call_stack = array<code_context>::make_copy(&msg->call_stack, &m->arena);

	global_api->release_mutex(&console->lines_mut);
}

dbg_value dbg_value::make_sec(allocator* alloc) { PROF

	dbg_value ret;
	ret.type = dbg_value_class::section;
	ret.sec.children = map<string,dbg_value>::make(8, alloc);
	return ret;
}

dbg_value dbg_value::make_view(any a) { PROF

	dbg_value ret;
	ret.type = dbg_value_class::view;
	ret.view = a;
	return ret;
}

dbg_value dbg_value::make_edit(any a) { PROF

	dbg_value ret;
	ret.type = dbg_value_class::edit;
	ret.edit = a;
	return ret;
}

dbg_value dbg_value::make_cal(_FPTR* c, void* p) { PROF

	dbg_value ret;
	ret.type = dbg_value_class::callback;
	ret.cal.callback.set(c);
	ret.cal.callback_param = p;
	return ret;
}

void dbg_value::destroy(allocator* alloc) {

	if(type == dbg_value_class::section) {
		FORMAP(it, sec.children) {
			it->key.destroy(alloc);
			it->value.destroy(alloc);
		}
		sec.children.destroy();
	} 
}

bool prof_sort_name(profile_node* l, profile_node* r) {

	return l->context.function() <= r->context.function();
}

bool prof_sort_heir(profile_node* l, profile_node* r) {

	return r->heir <= l->heir;
}

bool prof_sort_self(profile_node* l, profile_node* r) {

	return r->self <= l->self;
}

bool prof_sort_calls(profile_node* l, profile_node* r) {

	return r->calls <= l->calls;
}

void _prof_sec(string name, code_context context) { 
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::enter_func;
		m.context = context;
		_memcpy_ctx(name.c_str, m.context.c_function, name.len);
	
		POST_MSG(m);
	}
}

void _prof_sec_end() { 
	if(this_thread_data.profiling) {
		dbg_msg m;
		m.type = dbg_msg_type::exit_func;
		POST_MSG(m);
	}
}
