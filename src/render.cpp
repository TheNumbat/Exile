
mesh_2d make_mesh_2d(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d ret;

	ret.alloc = alloc;

	ret.verticies = vector<v2>::make(verts, alloc);
	ret.texCoords = vector<v3>::make(verts, alloc);
	ret.colors 	  =	vector<v4>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	return ret;
}

void destroy_mesh(mesh_2d* m) { PROF

	m->verticies.destroy();
	m->texCoords.destroy();
	m->colors.destroy();
	m->elements.destroy();
	m->alloc = null;
}

void mesh_push_cutrect(mesh_2d* m, r2 r, f32 round, color c) { PROF

	u32 idx = m->verticies.size;

	m->verticies.push(V2(r.x, r.y + round));
	m->verticies.push(V2(r.x, r.y + r.h - round));
	m->verticies.push(V2(r.x + round, r.y + r.h));
	m->verticies.push(V2(r.x + r.w - round, r.y + r.h));
	m->verticies.push(V2(r.x + r.w, r.y + r.h - round));
	m->verticies.push(V2(r.x + r.w, r.y + round));
	m->verticies.push(V2(r.x + r.w - round, r.y));
	m->verticies.push(V2(r.x + round, r.y));

	FOR(8) m->texCoords.push(V3(0.0f, 0.0f, 0.0f));

	colorf cf = color_to_f(c);
	FOR(8) m->colors.push(cf);

	m->elements.push(V3u(idx, idx + 1, idx + 2));
	m->elements.push(V3u(idx, idx + 2, idx + 7));
	m->elements.push(V3u(idx + 7, idx + 2, idx + 3));
	m->elements.push(V3u(idx + 7, idx + 6, idx + 3));
	m->elements.push(V3u(idx + 3, idx + 4, idx + 5));
	m->elements.push(V3u(idx + 3, idx + 5, idx + 6));
}

void mesh_push_rect(mesh_2d* m, r2 r, color c) { PROF

	u32 idx = m->verticies.size;

	m->verticies.push(V2(r.x, r.y + r.h));	// BLC
	m->verticies.push(r.xy);				// TLC
	m->verticies.push(add(r.xy, r.wh));		// BRC
	m->verticies.push(V2(r.x + r.w, r.y));	// TRC

	FOR(4) m->texCoords.push(V3(0.0f, 0.0f, 0.0f));

	colorf cf = color_to_f(c);
	FOR(4) m->colors.push(cf);

	m->elements.push(V3u(idx, idx + 1, idx + 2));
	m->elements.push(V3u(idx + 1, idx + 2, idx + 3));
}

v2 size_text(asset* font, string text_utf8, f32 point) { PROF

	v2 ret;

	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}

	u32 index = 0;
	while(u32 codepoint = get_next_codepoint(text_utf8, &index)) {

		glyph_data glyph = get_glyph_data(font, codepoint);
		ret.x += scale * glyph.advance;
	}

	ret.y = scale * font->font.linedist;
	return ret;
}

f32 mesh_push_text_line(mesh_2d* m, asset* font, string text_utf8, v2 pos, f32 point, color c) { PROF

	colorf cf = color_to_f(c);
	f32 x = pos.x;
	f32 y = pos.y;
	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}
	y += scale * font->font.linedist;

	u32 index = 0;
	while(u32 codepoint = get_next_codepoint(text_utf8, &index)) {

		u32 idx = m->verticies.size;
		glyph_data glyph = get_glyph_data(font, codepoint);

		f32 w = (f32)font->font.width;
		f32 h = (f32)font->font.height;
		v3 tlc = V3(glyph.x1/w, 1.0f - glyph.y1/h, 1.0f);
		v3 brc = V3(glyph.x2/w, 1.0f - glyph.y2/h, 1.0f);
		v3 trc = V3(glyph.x2/w, 1.0f - glyph.y1/h, 1.0f);
		v3 blc = V3(glyph.x1/w, 1.0f - glyph.y2/h, 1.0f);

		m->verticies.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		m->verticies.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		m->verticies.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		m->verticies.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		m->texCoords.push(blc);
		m->texCoords.push(tlc);
		m->texCoords.push(brc);
		m->texCoords.push(trc);

		FOR(4) m->colors.push(cf);

		m->elements.push(V3u(idx, idx + 1, idx + 2));
		m->elements.push(V3u(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	return scale * font->font.linedist;
}

mesh_3d make_mesh_3d(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_3d ret;

	ret.alloc = alloc;

	ret.verticies = vector<v3>::make(verts, alloc);
	ret.texCoords = vector<v2>::make(verts, alloc);

	return ret;
}

void destroy_mesh(mesh_3d* m) { PROF

	m->verticies.destroy();
	m->texCoords.destroy();

	m->alloc = null;	
}

render_command make_render_command(render_command_type type, void* data, u32 key) { PROF

	render_command ret;

	ret.cmd = type;
	ret.data = data;
	ret.sort_key = key;

	if(type == render_command_type::mesh_2d) {
		mesh_2d* m = (mesh_2d*)data;
		ret.elements = m->elements.size * 3;
	}

	return ret;
}

bool operator<(render_command first, render_command second) { PROF
	return first.sort_key < second.sort_key;
}

render_command_list make_command_list(allocator* alloc, u32 cmds) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	render_command_list ret;

	ret.alloc = alloc;

	ret.commands = vector<render_command>::make(cmds, alloc);

	return ret;
}

void destroy_command_list(render_command_list* rcl) { PROF

	rcl->commands.destroy();
	rcl->alloc = null;
}

void render_add_command(render_command_list* rcl, render_command rc) { PROF

	rcl->commands.push(rc);
}

void sort_render_commands(render_command_list* rcl) { PROF

	rcl->commands.qsort();
}

void clear_mesh(mesh_2d* m) { PROF
	m->verticies.clear();
	m->colors.clear();
	m->texCoords.clear();
	m->elements.clear();
}

void clear_mesh(mesh_3d* m) { PROF
	m->verticies.clear();
	m->texCoords.clear();
}
