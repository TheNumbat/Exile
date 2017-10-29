
mesh_2d_col mesh_2d_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_col ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.colors 	  =	vector<colorf>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	return ret;
}

void mesh_2d_col::destroy() { PROF

	vertices.destroy();
	colors.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_col::clear() { PROF
	vertices.clear();
	colors.clear();
	elements.clear();
}

void mesh_2d_col::push_cutrect(r2 r, f32 round, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(V2(r.x, r.y + round));
	vertices.push(V2(r.x, r.y + r.h - round));
	vertices.push(V2(r.x + round, r.y + r.h));
	vertices.push(V2(r.x + r.w - round, r.y + r.h));
	vertices.push(V2(r.x + r.w, r.y + r.h - round));
	vertices.push(V2(r.x + r.w, r.y + round));
	vertices.push(V2(r.x + r.w - round, r.y));
	vertices.push(V2(r.x + round, r.y));

	colorf cf = color_to_f(c);
	DO(8) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx, idx + 2, idx + 7));
	elements.push(V3u(idx + 7, idx + 2, idx + 3));
	elements.push(V3u(idx + 7, idx + 6, idx + 3));
	elements.push(V3u(idx + 3, idx + 4, idx + 5));
	elements.push(V3u(idx + 3, idx + 5, idx + 6));
}

void mesh_2d_col::push_rect(r2 r, color c) { PROF

	u32 idx = vertices.size;

	vertices.push(V2(r.x, r.y + r.h));	// BLC
	vertices.push(r.xy);				// TLC
	vertices.push(add(r.xy, r.wh));		// BRC
	vertices.push(V2(r.x + r.w, r.y));	// TRC

	colorf cf = color_to_f(c);
	DO(4) colors.push(cf);

	elements.push(V3u(idx, idx + 1, idx + 2));
	elements.push(V3u(idx + 1, idx + 2, idx + 3));
}


mesh_2d_tex mesh_2d_tex::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.texCoords =	vector<v2>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	return ret;

}

void mesh_2d_tex::destroy() { PROF

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

mesh_2d_tex_col mesh_2d_tex_col::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_2d_tex_col ret;

	ret.alloc = alloc;

	ret.vertices = vector<v2>::make(verts, alloc);
	ret.texCoords =	vector<v2>::make(verts, alloc);
	ret.colors 	  = vector<colorf>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc); 

	return ret;
}

void mesh_2d_tex_col::destroy() { PROF

	vertices.destroy();
	texCoords.destroy();
	colors.destroy();
	elements.destroy();
	alloc = null;
}

void mesh_2d_tex_col::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
	colors.clear();
}

mesh_3d_tex mesh_3d_tex::make(u32 verts, allocator* alloc) { PROF

	if(alloc == null) {
		alloc = CURRENT_ALLOC();
	}

	mesh_3d_tex ret;

	ret.alloc = alloc;

	ret.vertices = vector<v3>::make(verts, alloc);
	ret.texCoords = vector<v2>::make(verts, alloc);
	ret.elements  = vector<uv3>::make(verts, alloc);

	return ret;
}

void mesh_3d_tex::destroy() { PROF

	vertices.destroy();
	texCoords.destroy();
	elements.destroy();
	alloc = null;	
}

void mesh_3d_tex::clear() { PROF
	vertices.clear();
	texCoords.clear();
	elements.clear();
}

render_command render_command::make(render_command_type type, void* data, u32 key) { PROF

	render_command ret;

	ret.cmd = type;
	ret.data = data;
	ret.sort_key = key;

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

f32 mesh_2d_tex_col::push_text_line(asset* font, string text_utf8, v2 pos, f32 point, color c) { PROF

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

		u32 idx = vertices.size;
		glyph_data glyph = font->font.get_glyph(codepoint);

		f32 w = (f32)font->font.width;
		f32 h = (f32)font->font.height;
		v2 tlc = V2(glyph.x1/w, 1.0f - glyph.y1/h);
		v2 brc = V2(glyph.x2/w, 1.0f - glyph.y2/h);
		v2 trc = V2(glyph.x2/w, 1.0f - glyph.y1/h);
		v2 blc = V2(glyph.x1/w, 1.0f - glyph.y2/h);

		vertices.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff2)); 	// BLC
 		vertices.push(V2(x + scale*glyph.xoff1, y + scale*glyph.yoff1));	// TLC
 		vertices.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff2));	// BRC
 		vertices.push(V2(x + scale*glyph.xoff2, y + scale*glyph.yoff1));	// TRC

		texCoords.push(blc);
		texCoords.push(tlc);
		texCoords.push(brc);
		texCoords.push(trc);

		DO(4) colors.push(cf);

		elements.push(V3u(idx, idx + 1, idx + 2));
		elements.push(V3u(idx + 1, idx + 2, idx + 3));

		x += scale * glyph.advance;
	}

	return scale * font->font.linedist;
}

v2 size_text(asset* font, string text_utf8, f32 point) { PROF

	v2 ret;

	f32 scale = point / font->font.point;
	if(point == 0.0f) {
		scale = 1.0f;
	}

	u32 index = 0;
	while(u32 codepoint = text_utf8.get_next_codepoint(&index)) {

		glyph_data glyph = font->font.get_glyph(codepoint);
		ret.x += scale * glyph.advance;
	}

	ret.y = scale * font->font.linedist;
	return ret;
}
