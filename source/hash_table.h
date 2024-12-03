#pragma once

#include "memory.h"


inline u64 basic_string_hash(String str) {
    u64 h = 0x100;

    for (s64 i = 0; i < str.size; i += 1) {
        h ^= str[i];
        h *= 1111111111111111111u;
    }

    return h;
}

template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
struct HashTable {
    static s64 const FirstValidHash = 1;

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

template<class ValueType>
using StringTable = HashTable<String, ValueType, u64, basic_string_hash>;


template<class ValueType>
struct HashTableFindResult {
    ValueType *found;
    s64        index;
};


template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
void init(HashTable<KeyType, ValueType, HashType, HashFunc> *table, s32 exponent, Allocator alloc = DefaultAllocator) {
    typedef HashTable<KeyType, ValueType, HashType, HashFunc> Table;
	typedef typename Table::Entry Entry;

	r32 const max_load = 0.6f;

    // NOTE: Use u64 for the size instead?
	assert(exponent >= 0 && exponent < 63);

    if (table->allocator.allocate != alloc.allocate) {
        destroy(table);
        table->allocator = alloc;
    }

    s64 initial_size = 1LL << exponent;

	table->entries  = ALLOC(alloc, Entry, initial_size);
	table->exponent = exponent;
    table->used     = 0;
	table->alloc    = initial_size;
	table->max_load = (s64)(initial_size * max_load);
}

template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
void destroy(HashTable<KeyType, ValueType, HashType, HashFunc> *table) {
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


template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
void grow(HashTable<KeyType, ValueType, HashType, HashFunc> *table) {
    typedef HashTable<KeyType, ValueType, HashType, HashFunc> Table;

    Table new_table = {};
    init(&new_table, table->exponent + 1, table->allocator);

    for (s64 i = 0; i < table->alloc; i += 1) {
        if (table->entries[i].hash >= table->FirstValidHash) {
            insert(&new_table, table->entries[i].key, table->entries[i].value);
        }
    }

    destroy(table);
    *table = new_table;
}

template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
HashTableFindResult<ValueType> find(HashTable<KeyType, ValueType, HashType, HashFunc> *table, KeyType key) {
    typedef HashTable<KeyType, ValueType, HashType, HashFunc> Table;
	typedef typename Table::Entry Entry;

    // NOTE: Can't just return nothing on an empty map, because the resulting index would
    //       point to unallocated memory.
    s32 const default_exponent = 6;
    if (table->alloc == 0) init(table, default_exponent);

    u64 h = HashFunc(key);
    if (h < table->FirstValidHash) h = table->FirstValidHash;
    for (s64 i = h;;) {
        i = hash_table_lookup(h, table->exponent, i);

        Entry *entry = &table->entries[i];
        if (entry->hash == 0) {
            // TODO: Growing table while finding. Could lead to reallocation
            //       although you are only interrested in knowing if a key exists.
            //       But I think this is worth it as it makes inserting easier.
            if (table->used == table->max_load) {
                grow(table);
                return find(table, key);
            }
            return {0, i};
        } else if (entry->key == key) {
            return {&entry->value, i};
        }
    }
}



// TODO: KeyType is passed by value. This could be problematic if it is a big struct.
//       But usually keys are integers or strings. Who would shove a std::iostream in here?
//       Also for now the slot gets overidden. Should the original value be kept?
template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
ValueType *insert(HashTable<KeyType, ValueType, HashType, HashFunc> *table, KeyType key, ValueType value) {
    auto result = find(table, key);
    return insert_at(table, result.index, key, value);
}

// NOTE: Never use an index that is not returned from a previous call to find.
template<class KeyType, class ValueType, class HashType, HashType(*HashFunc)(KeyType key)>
ValueType *insert_at(HashTable<KeyType, ValueType, HashType, HashFunc> *table, s64 index, KeyType key, ValueType value) {
    assert(index >= 0 && index < table->alloc);

    HashType hash = HashFunc(key);
    if (hash < table->FirstValidHash) hash = table->FirstValidHash;

    auto *entry = &table->entries[index];
    entry->hash  = hash;
    entry->key   = key;
    entry->value = value;

    table->used += 1;

    return &entry->value;
}

