#pragma once

#include "hash_table.h"
#include "stb_truetype.h"


// TODO: Should suffice for testing but a proper hash function is probably needed.
inline u32 glyph_hash(u32 cp) {
    cp = ((cp >> 16) ^ cp) * 0x45d9f3b;
    cp = ((cp >> 16) ^ cp) * 0x45d9f3b;
    cp =  (cp >> 16) ^ cp;

    return cp;
}

struct CachedGlyph;
struct GlyphInfo {
    s16 x, y;
    s16 width, height;
    s16 bearing_x;
    s16 bearing_y;
	s16 advance;
};

struct Font {
    Allocator allocator;

    String file_content;
    stbtt_fontinfo info;

    r32 scale;

    r32 ascent;
    r32 descent;
    s32 line_height;

    s32 glyph_width;
    s32 glyph_height;

    s32 glyph_rows;
    s32 glyph_columns;
    s32 max_cached_glyphes;
    s32 cache_used;

    b32 is_dirty;

    s32 atlas_size;
    String atlas;
    HashTable<u32, CachedGlyph, u32, glyph_hash> glyphs;
};


b32  init(Font *font, String file_name, r32 height, s32 atlas_size, Allocator alloc = DefaultAllocator);
void destroy(Font *font);

GlyphInfo get_glyph(Font *font, u32 cp);

