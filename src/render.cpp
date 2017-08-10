
mesh_2d mesh_2d::make(u32 verts, allocator* alloc) { PROF

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

void mesh_2d::destroy() { PROF

	verticies.destroy();
	texCoords.destroy();
	colors.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d::push_cutrect(r2 r, f32 round, color c) { PROF

	u32 idx = verticies.size;

	verticies.push(V2(r.x, r.y + round));
	verticies.push(V2(r.x, r.y + r.h - round));
	verticies.push(V2(r.x + round, r.y + r.h));
	verticies.push(V2(r.x + r.w - round, r.y + r.h));
	verticies.push(V2(r.x + r.w, r.y + r.h - round));
	verticies.push(V2(r.x + r.w, r.y + round));
	verticies.push(V2(r.x + r.w - round, r.y));
	verticies.push(V2(r.x + round, r.y));

	FOR(8) texCoords.push(V3(0.0f, 0.0f, 0.0f));

	colorf cf = color_to_f(c);
	FOR(8) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx, idx + 2, idx + 7));
	elements.push(V3u(idx + 7, idx + 2, idx + 3));
	elements.push(V3u(idx + 7, idx + 6, idx + 3));
	elements.push(V3u(idx + 3, idx + 4, idx + 5));
	elements.push(V3u(idx + 3, idx + 5, idx + 6));
}

void mesh_2d::push_rect(r2 r, color c) { PROF

	u32 idx = verticies.size;

	verticies.push(V2(r.x, r.y + r.h));	// BLC
	verticies.push(r.xy);				// TLC
	verticies.push(add(r.xy, r.wh));		// BRC
	verticies.push(V2(r.x + r.w, r.y));	// TRC

	FOR(4) texCoords.push(V3(0.0f, 0.0f, 0.0f));

	colorf cf = color_to_f(c);
	FOR(4) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx + 1, idx + 2, idx + 3));
}

v2 size_text(asset* font, string text_utf8, f32 point) { PROF

	v2 ret;

	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		glyph_data glyph = font->font.get_glyph_data(codepoint);
		ret.x += scale * glyph.advance;
	}

	ret.y = scale * font->font.linedist;
	return ret;
}

f32 mesh_2d::push_text_line(asset* font, string text_utf8, v2 pos, f32 point, color c) { PROF

	colorf cf = color_to_f(c);
	f32 x = pos.x;
	f32 y = pos.y;
	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}
	y += scale * font->font.linedist;

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		u32 idx = verticies.size;
		glyph_data glyph = font->font.get_glyph_data(codepoint);

		f32 w = (f32)font->font.width;
		f32 h = (f32)font->font.height;
		v3 tlc = V3(glyph.x1/w, 1.0f - glyph.y1/h, 1.0f);
		v3 brc = V3(glyph.x2/w, 1.0f - glyph.y2/h, 1.0f);
		v3 trc = V3(glyph.x2/w, 1.0f - glyph.y1/h, 1.0f);
		v3 blc = V3(glyph.x1/w, 1.0f - glyph.y2/h, 1.0f);

		verticies.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		verticies.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		verticies.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		verticies.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		texCoords.push(blc);
		texCoords.push(tlc);
		texCoords.push(brc);
		texCoords.push(trc);

		FOR(4) colors.push(cf);

		elements.push(V3u(idx, idx + 1, idx + 2));
		elements.push(V3u(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	return scale * font->font.linedist;
}

mesh_3d mesh_3d::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_3d ret;

	ret.alloc = alloc;

	ret.verticies = vector<v3>::make(verts, alloc);
	ret.texCoords = vector<v2>::make(verts, alloc);

	return ret;
}

void mesh_3d::destroy() { PROF

	verticies.destroy();
	texCoords.destroy();

	alloc = null;	
}

render_command render_command::make(render_command_type type, void* data, u32 key) { PROF

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

render_command_list render_command_list::make(allocator* alloc, u32 cmds) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	render_command_list ret;

	ret.alloc = alloc;

	ret.commands = vector<render_command>::make(cmds, alloc);

	return ret;
}

void render_command_list::destroy() { PROF

	commands.destroy();
	alloc = null;
}

void render_command_list::add_command(render_command rc) { PROF

	commands.push(rc);
}

void render_command_list::sort() { PROF

	commands.qsort();
}

void mesh_2d::clear() { PROF
	verticies.clear();
	colors.clear();
	texCoords.clear();
	elements.clear();
}

void mesh_3d::clear() { PROF
	verticies.clear();
	texCoords.clear();
}
