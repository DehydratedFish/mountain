#pragma once

#include "memory.h"


inline u64 basic_hash(u32 *value) {
    return *value;
}

inline u64 basic_hash(String *str) {
    u64 h = 0x100;

    for (s64 i = 0; i < str->size; i += 1) {
        h ^= str->data[i];
        h *= 1111111111111111111u;
    }

    return h;
}


// NOTE: I am too lazy to type out these template things every time.
#define TEMPLATE_DEFINITION template<class KeyType, class ValueType, class HashType = u64, HashType(*HashFunc)(KeyType*) = basic_hash>
#define TABLE_TYPE HashTable<KeyType, ValueType, HashType, HashFunc>
#define ENTRY_TYPE typename TABLE_TYPE::Entry

TEMPLATE_DEFINITION
struct HashTable {
    // TODO: Can hashes be something other than integers?
    static HashType const FirstValidHash = 1;

    // TODO: Put this outside the HashTable struct?
    //       Not sure if the template/namespace hassle is worth it otherwise.
    struct Entry {
        HashType  hash;
        KeyType   key;
        ValueType value;
    };

    Allocator allocator;

    Entry *entries;
    s64    used;
    s64    alloc;
    s64    max_load;
    s32    exponent;
    // TODO: Use u64 for the size instead?
};


TEMPLATE_DEFINITION
void init(TABLE_TYPE *table, s32 exponent, Allocator alloc = DefaultAllocator) {
	r32 const max_load = 0.6f;

    // NOTE: Use u64 for the size instead?
	assert(exponent >= 0 && exponent < 63);

    if (table->allocator.allocate != alloc.allocate) {
        destroy(table);
        table->allocator = alloc;
    }

    s64 initial_size = 1LL << exponent;

	table->entries  = ALLOC(alloc, ENTRY_TYPE, initial_size);
	table->exponent = exponent;
    table->used     = 0;
	table->alloc    = initial_size;
	table->max_load = (s64)(initial_size * max_load);
}

TEMPLATE_DEFINITION
void destroy(TABLE_TYPE *table) {
    if (table->allocator.allocate) {
        DEALLOC(table->allocator, table->entries, table->alloc);

        table->entries  = 0;
        table->exponent = 0;
        table->used     = 0;
        table->alloc    = 0;
        table->max_load = 0;
    }
}

inline s64 hash_table_lookup(u64 hash, s32 exponent, s64 index) {
    u64 mask = (1 << exponent) - 1;
    u64 step = (hash >> (64 - exponent)) | 1;

    return (index + step) & mask;
}


TEMPLATE_DEFINITION
void grow(TABLE_TYPE *table) {
    TABLE_TYPE new_table = {};
    init(&new_table, table->exponent + 1, table->allocator);

    for (s64 i = 0; i < table->alloc; i += 1) {
        if (table->entries[i].hash >= table->FirstValidHash) {
            insert(&new_table, table->entries[i].key, table->entries[i].value);
        }
    }

    destroy(table);
    *table = new_table;
}

TEMPLATE_DEFINITION
ValueType *find(TABLE_TYPE *table, KeyType key) {
    if (table->alloc == 0) return 0;

    u64 h = HashFunc(&key);
    if (h < table->FirstValidHash) h = table->FirstValidHash;
    for (s64 i = h;;) {
        i = hash_table_lookup(h, table->exponent, i);

        ENTRY_TYPE *entry = &table->entries[i];
        if (entry->hash == 0) {
            return 0;
        } else if (entry->key == key) {
            return &entry->value;
        }
    }
}


TEMPLATE_DEFINITION
ValueType *insert(TABLE_TYPE *table, KeyType key, ValueType value) {
    s32 const default_exponent = 6;
    if (table->alloc == 0) init(table, default_exponent);

    // NOTE: Technically the table doesn't need to grow if the value is never inserted.
    //       But if the grow() is happening inside the for loop the hash needs to be
    //       recalculated and I think the complexity is not worth it.
    if (table->used + 1 > table->max_load) grow(table);

    u64 h = HashFunc(&key);
    if (h < table->FirstValidHash) h = table->FirstValidHash;
    for (s64 i = h;;) {
        i = hash_table_lookup(h, table->exponent, i);

        ENTRY_TYPE *entry = &table->entries[i];
        if (entry->hash == 0) {
            entry->hash  = h;
            entry->key   = key;
            entry->value = value;

            table->used += 1;

            return &entry->value;
        } else if (entry->key == key) {
            return 0;
        }
    }
}

TEMPLATE_DEFINITION
ValueType *upsert(TABLE_TYPE *table, KeyType key) {
    s32 const default_exponent = 6;
    if (table->alloc == 0) init(table, default_exponent);

    if (table->used + 1 > table->max_load) grow(table);

    u64 h = HashFunc(&key);
    if (h < table->FirstValidHash) h = table->FirstValidHash;
    for (s64 i = h;;) {
        i = hash_table_lookup(h, table->exponent, i);

        ENTRY_TYPE *entry = &table->entries[i];
        if (entry->hash == 0) {
            entry->hash  = h;
            entry->key   = key;
            entry->value = {};

            table->used += 1;

            return &entry->value;
        } else if (entry->key == key) {
            return &entry->value;
        }
    }
}


#undef TEMPLATE_DEFINITION
#undef TABLE_TYPE
#undef ENTRY_TYPE

