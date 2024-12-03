#include "font.h"

#include "platform.h"
#include "string2.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_STATIC
#define STBTT_assert
#include "stb_truetype.h"


// TODO: Using 16bit integers because it should be more than enough.
struct CachedGlyph {
    s16 x, y;
    s16 width, height;
    s16 bearing_x;
    s16 bearing_y;
	s16 advance;

    s32 table_index;
};

INTERNAL CachedGlyph *load_glyph(Font *font, u32 cp);

b32 init(Font *font, String file_name, r32 height, s32 atlas_size, Allocator alloc) {
    destroy(font); // NOTE: Checks for initialised font anyway.

    PlatformReadResult read_result = platform_read_entire_file(file_name, font->allocator);
    if (read_result.error) {
        log_error("Could not read font %S\n", file_name);

        return false;
    }

    String ttf = read_result.content;

    stbtt_fontinfo stb = {};
    stbtt_InitFont(&stb, ttf.data, stbtt_GetFontOffsetForIndex(ttf.data, 0));
    r32 scale = stbtt_ScaleForPixelHeight(&stb, height);

    font->atlas_size = atlas_size;

    int unscaled_ascent, unscaled_descent, unscaled_line_gap;
    stbtt_GetFontVMetrics(&stb, &unscaled_ascent, &unscaled_descent, &unscaled_line_gap);

    font->ascent      = unscaled_ascent * scale;
    font->descent     = unscaled_descent * scale;
    font->line_height = (s32)ceil(font->ascent - font->descent + (unscaled_line_gap * scale));

    int x0, y0, x1, y1;
    stbtt_GetFontBoundingBox(&stb, &x0, &y0, &x1, &y1);
    font->glyph_width  = (s32)ceil((x1 - x0) * scale);
    font->glyph_height = (s32)ceil((y1 - y0) * scale);

    font->glyph_rows    = atlas_size / font->glyph_height;
    font->glyph_columns = atlas_size / font->glyph_width;

    font->max_cached_glyphes = font->glyph_rows * font->glyph_columns;

    font->atlas = allocate_string(atlas_size * atlas_size, alloc);

    font->info  = stb;
    font->scale = scale;

    // TODO: Currently only loading the ascii characters.
    //       Depending on the language should other characters be cached?
    for (s32 i = 32; i < 127; i += 1) {
        load_glyph(font, i);
    }

    return true;
}

void destroy(Font *font) {
    if (font->allocator.allocate) {
        destroy(&font->file_content);
        destroy(&font->atlas);
        destroy(&font->glyphs);
    }
}

INTERNAL void copy_image(String atlas, s32 stride, s32 x, s32 y, u8 *bitmap, s32 width, s32 height) {
    u8 *dest = &atlas[y * stride + x];
    for (s32 h = 0; h < height; h += 1) {
        for (s32 w = 0; w < width; w += 1) {
            *dest = *bitmap;

            dest   += 1;
            bitmap += 1;
        }
        dest += stride;
    }
}

INTERNAL CachedGlyph *load_glyph(Font *font, u32 cp) {
    CachedGlyph *result = 0;

    if (font->cache_used == font->max_cached_glyphes) {
        die("Not implemented");
    } else {
        int glyph = stbtt_FindGlyphIndex(&font->info, cp);
        assert(glyph); // TODO: Replacement character.

        int w = 0;
        int h = 0;
        int x_offset = 0;
        int y_offset = 0;
        u8 *bitmap = stbtt_GetGlyphSDF(&font->info, font->scale, glyph, 0, 180, 36, &w, &h, &x_offset, &y_offset);
        DEFER(stbtt_FreeSDF(bitmap, 0));

        s32 x = (font->cache_used % font->glyph_columns) * font->glyph_width;
        s32 y = (font->cache_used / font->glyph_columns) * font->glyph_height;

        if (bitmap) { // NOTE: Space can be empty for example.
            copy_image(font->atlas, font->atlas_size, x, y, bitmap, w, h);
        }

        int advance_width, lsb;
        stbtt_GetGlyphHMetrics(&font->info, glyph, &advance_width, &lsb);

        CachedGlyph info = {};
        info.x = x;
        info.y = y;
        info.width   = w;
        info.height  = h;
        info.bearing_x = x_offset;
        info.bearing_y = y_offset;
        info.advance   = advance_width * font->scale;
        info.table_index = font->cache_used;

        result = insert(&font->glyphs, cp, info);

        font->cache_used += 1;
        font->is_dirty = true;
    }

    return result;
}

// TODO: Skip the copy?
GlyphInfo get_glyph(Font *font, u32 cp) {
    CachedGlyph *glyph = 0;

    auto result = find(&font->glyphs, cp);
    if (result.found) glyph = result.found;
    else              glyph = load_glyph(font, cp);

    GlyphInfo info = {};
    info.x = glyph->x;
    info.y = glyph->y;
    info.width   = glyph->width;
    info.height  = glyph->height;
    info.bearing_x = glyph->bearing_x;
    info.bearing_y = glyph->bearing_y;
    info.advance   = glyph->advance;

    return info;
}

