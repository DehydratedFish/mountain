#pragma once

#include "list.h"


// NOTE: These can't be sorted or stored in a hash table as this would destroy ordering
//       and the file can't be saved in the layout before the read.

struct ConfigurationLine {
    String key;
    String value;
    String comment;
};

struct ConfigurationSection {
    String name;
    List<ConfigurationLine> lines;
};

struct Configuration {
    Allocator allocator;

    String file_name;
    List<ConfigurationSection> sections;
};


b32  init(Configuration *config, String file_name, Allocator alloc = DefaultAllocator);
void destroy(Configuration *config);

String entry_string(Configuration *config, String section, String key, String def = {});
s64    entry_s64   (Configuration *config, String section, String key, s64 def = 0);
r32    entry_r32   (Configuration *config, String section, String key, r32 def = 0.0f);

