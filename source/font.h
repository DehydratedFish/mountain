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

struct FontDimensions {
    r32 width;
    r32 height;
};

struct CachedGlyph;
struct GlyphInfo {
    r32 u0, v0, u1, v1;
    r32 x0, y0, x1, y1;

    r32 advance;
};

struct ScaledFontMetrics {
    r32 ascent;
    r32 descent;
    r32 line_height;
};

struct Font {
    Allocator allocator;

    String file_content;
    stbtt_fontinfo info;

    r32 pixel_height;
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

ScaledFontMetrics scaled_line_metrics(Font *font, r32 height);

GlyphInfo get_glyph(Font *font, u32 cp, r32 height);
FontDimensions text_dimensions(Font *font, String text, r32 height, b32 floor_advance = false);

