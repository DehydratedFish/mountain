#include "config.h"

#include "platform.h"
#include "string2.h"


enum TokenKind {
    TOKEN_UNKOWN,
    TOKEN_ERROR,
    TOKEN_STRING,
    TOKEN_COMMENT,
    TOKEN_EQUAL,
    TOKEN_LEFT_BRACKET,
    TOKEN_RIGHT_BRACKET,
    TOKEN_NEW_LINE,
    TOKEN_END_OF_INPUT,
};

struct Token {
    TokenKind kind;
    String content;
    s32 line;
};

enum ParseStatus {
    PARSE_OK,
    PARSE_ERROR,
    PARSE_END,
};

struct Parser {
    ParseStatus status;
    String source;

    s64 current_pos;
    s32 current_line;

    Token current_token;
    Token previous_token;

    ConfigurationSection *current_section;
};


INTERNAL b32 peek_char(Parser *parser, u8 *c) {
    if (parser->current_pos < parser->source.size) {
        *c = parser->source[parser->current_pos];

        return true;
    }

    return false;
}

INTERNAL b32 get_char(Parser *parser, u8 *c) {
    if (peek_char(parser, c)) {
        parser->current_pos += 1;

        return true;
    }

    return false;
}

INTERNAL void skip_whitespaces(Parser *parser) {
    u8 c;
    while (peek_char(parser, &c) && is_any(c, " \t")) {
        parser->current_pos += 1;
    }
}

INTERNAL String make_token_content(Parser *parser, s64 size) {
    String result = {};
    result.data = &parser->source[parser->current_pos];
    result.size = size;

    return result;
}

INTERNAL Token parse_comment(Parser *parser) {
    Token token = {};
    token.kind  = TOKEN_COMMENT;
    token.line  = parser->current_line;

    parser->current_pos += 1;
    token.content.data = &parser->source[parser->current_pos];

    u8 c;
    while (peek_char(parser, &c) && (c != '\n' && c != '\r')) {
        token.content.size += 1;
        parser->current_pos += 1;
    }

    return token;
}

INTERNAL Token parse_quoted_string(Parser *parser) {
    Token token = {};
    token.kind  = TOKEN_STRING;
    token.line  = parser->current_line;

    u8 c;
    if (!get_char(parser, &c) || c != '"') {
        token.kind = TOKEN_ERROR;
        token.content = "Expected \" to start string.";

        parser->status = PARSE_ERROR;

        return token;
    }

    token.content.data = &parser->source[parser->current_pos];

    while (get_char(parser, &c)) {
        if (c == '"') break;

        token.content.size += 1;
    }

    if (c != '"') {
        token.kind = TOKEN_ERROR;
        token.content = "Unterminated string.";

        parser->status = PARSE_ERROR;
    }

    return token;
}

INTERNAL Token parse_string(Parser *parser) {
    Token token = {};
    token.kind  = TOKEN_STRING;
    token.line  = parser->current_line;
    token.content.data = &parser->source[parser->current_pos];

    u8 c;
    while (peek_char(parser, &c)) {
        if (is_any(c, "=[];\n\r\"")) break;

        token.content.size  += 1;
        parser->current_pos += 1;
    };

    token.content = trim(token.content);

    return token;
}

INTERNAL Token next_token(Parser *parser) {
    skip_whitespaces(parser);

    Token token = {};
    token.line = parser->current_line;

    u8 c;
    if (peek_char(parser, &c)) {
        if (c == '[') {
            token.kind    = TOKEN_LEFT_BRACKET;
            token.content = make_token_content(parser, 1);
            parser->current_pos += 1;
        } else if (c == ']') {
            token.kind    = TOKEN_RIGHT_BRACKET;
            token.content = make_token_content(parser, 1);
            parser->current_pos += 1;
        } else if (c == '=') {
            token.kind    = TOKEN_EQUAL;
            token.content = make_token_content(parser, 1);
            parser->current_pos += 1;
        } else if (c == '\n' || c == '\r') {
            u32 const multi_new_line = '\n' + '\r';

            parser->current_pos += 1;

            u8 c2;
            if (peek_char(parser, &c2) && (c + c2) == multi_new_line) {
                parser->current_pos += 1;
            }
            parser->current_line += 1;
            token.kind    = TOKEN_NEW_LINE;
        } else if (c == ';') {
            token = parse_comment(parser);
        } else if (c == '"') {
            token = parse_quoted_string(parser);
        } else {
            token = parse_string(parser);
        }
    } else {
        token.kind = TOKEN_END_OF_INPUT;
        parser->status = PARSE_END;
    }

    return token;
}

INTERNAL Parser init_parser(String source) {
    Parser parser = {};
    parser.source = source;

    parser.current_token = next_token(&parser);

    return parser;
}

INTERNAL void advance_token(Parser *parser) {
    parser->previous_token = parser->current_token;
    parser->current_token  = next_token(parser);
}

INTERNAL b32 match(Parser *parser, TokenKind kind) {
    if (parser->current_token.kind == kind) {
        advance_token(parser);

        return true;
    }

    return false;
}

INTERNAL b32 consume(Parser *parser, TokenKind kind, String message) {
    if (!match(parser, kind)) {
        parser->current_token.kind    = TOKEN_ERROR;
        parser->current_token.content = message;
        parser->status = PARSE_ERROR;

        return false;
    }

    return true;
}

INTERNAL void parse_section(Parser *parser, Configuration *config) {
    ConfigurationSection section = {};

    if (!consume(parser, TOKEN_LEFT_BRACKET, "Expected [ in section declaration.")) return;
    if (!consume(parser, TOKEN_STRING, "Section is missing a name.")) return;

    Token name = parser->previous_token;
    if (!consume(parser, TOKEN_RIGHT_BRACKET, "Missing closing ] in section declaration.")) return;

    // TODO: Check for duplicates.
    section.name = allocate_string(name.content, config->allocator);
    parser->current_section = append(&config->sections, section);

    match(parser, TOKEN_NEW_LINE);
}

INTERNAL void parse_entry(Parser *parser, Configuration *config) {
    if (!consume(parser, TOKEN_STRING, "Expected key.")) return;
    String key_token = parser->previous_token.content;

    if (!consume(parser, TOKEN_EQUAL, "Expected = .")) return;

    String value_token = {};
    if (parser->current_token.kind != TOKEN_NEW_LINE) {
        if (!consume(parser, TOKEN_STRING, "Expected value.")) return;
        value_token = parser->previous_token.content;
    }

    String key     = allocate_string(key_token, config->allocator);
    String value   = allocate_string(value_token, config->allocator);
    String comment = {};
    if (match(parser, TOKEN_COMMENT)) {
        comment = allocate_string(parser->previous_token.content, config->allocator);
    }

    append(&parser->current_section->lines, {key, value, comment});

    match(parser, TOKEN_NEW_LINE);
}

INTERNAL void parse_comment(Parser *parser, Configuration *config) {
    if (!consume(parser, TOKEN_COMMENT, "Expected comment.")) return;
    String comment = allocate_string(parser->previous_token.content, config->allocator);

    append(&parser->current_section->lines, {{}, {}, comment});

    match(parser, TOKEN_NEW_LINE);
}

b32 init(Configuration *config, String file_name, Allocator alloc) {
    PlatformReadResult read_result = platform_read_entire_file(file_name);
    DEFER(destroy(&read_result.content));
    
    if (read_result.error != PLATFORM_READ_OK) return false;
    Parser parser = init_parser(read_result.content);

    // TODO: This can be just a reset.
    destroy(config);
    config->allocator = alloc;
    config->file_name = allocate_string(file_name, alloc);

    ConfigurationSection empty = {}; // NOTE: Ambiguous call otherwise.
    parser.current_section = append(&config->sections, empty);

    while (parser.status != PARSE_END) {
        switch (parser.current_token.kind) {
        case TOKEN_LEFT_BRACKET: {
            parse_section(&parser, config);
        } break;

        case TOKEN_STRING: {
            parse_entry(&parser, config);
        } break;

        case TOKEN_COMMENT: {
            parse_comment(&parser, config);
        } break;

        case TOKEN_NEW_LINE: {
            advance_token(&parser);
            append(&parser.current_section->lines, {{}, {}, {}});
        } break;

        case TOKEN_END_OF_INPUT: {
            parser.status = PARSE_END;
        } break;

        default:
            parser.current_token.kind    = TOKEN_ERROR;
            parser.current_token.content = "Unexpected token.";

            parser.status = PARSE_ERROR;
        }

        // TODO: Better error handling and recovery.
        //       Just because there is an error on one line it should not stop working.
        if (parser.status == PARSE_ERROR) {
            Token error = parser.current_token;
            print("Error while loading configuration.\n%S:%d: %S\n\n", file_name, error.line + 1, error.content);

            advance_token(&parser);
            parser.status = PARSE_OK;
        }
    }

    return true;
}

void destroy(Configuration *config) {
    if (config->allocator.allocate) {
        FOR (config->sections, section) {
            FOR (section->lines, line) {
                destroy(&line->key);
                destroy(&line->value);
                destroy(&line->comment);
            }

            destroy(&section->lines);
            destroy(&section->name);
        }

        destroy(&config->sections);
        destroy(&config->file_name);
    }

    INIT_STRUCT(&config->allocator);
}

ConfigurationSection *find_section(Configuration *config, String section_name) {
    FOR (config->sections, section) {
        if (section->name == section_name) return section;
    }

    return 0;
}

String entry_string(Configuration *config, String section, String key, String def) {
    String result = def;

    ConfigurationSection *sec = find_section(config, section);
    if (sec) {
        FOR (sec->lines, line) {
            if (line->key == key) return line->value;
        }
    }

    return result;
}

s64 entry_s64(Configuration *config, String section, String key, s64 def) {
    s64 result = def;

    ConfigurationSection *sec = find_section(config, section);
    if (sec) {
        FOR (sec->lines, line) {
            if (line->key == key) return to_s64(line->value);
        }
    }

    return result;
}

r32 entry_r32(Configuration *config, String section, String key, r32 def) {
    r32 result = def;

    ConfigurationSection *sec = find_section(config, section);
    if (sec) {
        FOR (sec->lines, line) {
            if (line->key == key) return to_r32(line->value);
        }
    }

    return result;
}
