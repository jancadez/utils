#ifndef HASHMAP_H
#define HASHMAP_H

// Depends on strview.h
#include "../string_view/strview.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static uint64_t hash_sv(StringView sv) {
    uint64_t hash = 14695981039346656037ull;

    for (size_t i = 0; i < sv.len; i++) {
        hash ^= (unsigned char)sv.ptr[i];
        hash *= 1099511628211ull;
    }

    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdull;
    hash ^= hash >> 33;
    hash *= 0xc4ceb9fe1a85ec53ull;
    hash ^= hash >> 33;
    return hash;
}

enum HashState {
    HM_EMPTY,
    HM_FULL,
    HM_DELETED,
};

#define HASHMAP_INIT_CAP 32
_Static_assert((HASHMAP_INIT_CAP & (HASHMAP_INIT_CAP - 1)) == 0, "HASHMAP_INIT_CAP has to be a power of 2");

#define HASHMAP_DECLARE(Name, ValueType)                                                                               \
    typedef struct {                                                                                                   \
        ValueType value;                                                                                               \
        StringView key;                                                                                                \
        uint64_t hash;                                                                                                 \
        uint8_t state;                                                                                                 \
    } Name##Entry;                                                                                                     \
                                                                                                                       \
    typedef struct {                                                                                                   \
        Name##Entry* entries;                                                                                          \
        size_t len;                                                                                                    \
        size_t cap;                                                                                                    \
        size_t tombs;                                                                                                  \
    } Name##Map;                                                                                                       \
                                                                                                                       \
    Name##Map Name##Map_create() {                                                                                     \
        Name##Map map = {0};                                                                                           \
        map.cap = HASHMAP_INIT_CAP;                                                                                    \
        void* tmp = calloc(HASHMAP_INIT_CAP, sizeof(Name##Entry));                                                     \
        if (!tmp) {                                                                                                    \
            fprintf(stderr, "fatal error: failed to allocate memory\n");                                               \
            exit(EXIT_FAILURE);                                                                                        \
        }                                                                                                              \
                                                                                                                       \
        map.entries = tmp;                                                                                             \
        return map;                                                                                                    \
    }                                                                                                                  \
                                                                                                                       \
    void Name##Map_grow(Name##Map* map);                                                                               \
    bool Name##Map_put(Name##Map* map, StringView key, ValueType value);                                               \
    bool Name##Map_set(Name##Map* map, StringView key, ValueType value);                                               \
    bool Name##Map_get(Name##Map* map, StringView key, ValueType* out);                                                \
    bool Name##Map_has(Name##Map* map, StringView key);                                                                \
                                                                                                                       \
    void Name##Map_grow(Name##Map* map) {                                                                              \
        Name##Entry* entries = map->entries;                                                                           \
        size_t entry_count = map->cap;                                                                                 \
                                                                                                                       \
        map->cap *= 2;                                                                                                 \
        map->len = 0;                                                                                                  \
        void* tmp = calloc(map->cap, sizeof(Name##Entry));                                                             \
        if (!tmp) {                                                                                                    \
            fprintf(stderr, "fatal error: failed to allocate memoryn");                                                \
            exit(EXIT_FAILURE);                                                                                        \
        }                                                                                                              \
        map->entries = tmp;                                                                                            \
                                                                                                                       \
        for (size_t i = 0; i < entry_count; i++) {                                                                     \
            Name##Entry entry = entries[i];                                                                            \
            if (entry.state == HM_FULL) {                                                                              \
                Name##Map_put(map, entry.key, entry.value);                                                            \
            }                                                                                                          \
        }                                                                                                              \
                                                                                                                       \
        free(entries);                                                                                                 \
        map->tombs = 0;                                                                                                \
    }                                                                                                                  \
                                                                                                                       \
    bool Name##Map_put(Name##Map* map, StringView key, ValueType value) {                                              \
        if ((map->len + map->tombs) * 100 >= map->cap * 70) {                                                          \
            Name##Map_grow(map);                                                                                       \
        }                                                                                                              \
                                                                                                                       \
        uint64_t hash = hash_sv(key);                                                                                  \
        size_t mask = (map->cap - 1);                                                                                  \
        size_t i = hash & mask;                                                                                        \
                                                                                                                       \
        Name##Entry* first_free = NULL;                                                                                \
                                                                                                                       \
        for (size_t visited = 0; visited < map->cap; visited++) {                                                      \
            Name##Entry* entry = map->entries + i;                                                                     \
                                                                                                                       \
            if (entry->state == HM_EMPTY) {                                                                            \
                if (first_free) {                                                                                      \
                    entry = first_free;                                                                                \
                }                                                                                                      \
                                                                                                                       \
                entry->key = key;                                                                                      \
                entry->hash = hash;                                                                                    \
                entry->value = value;                                                                                  \
                entry->state = HM_FULL;                                                                                \
                map->len++;                                                                                            \
                return true;                                                                                           \
            } else if (entry->state == HM_DELETED) {                                                                   \
                if (!first_free) {                                                                                     \
                    first_free = entry;                                                                                \
                }                                                                                                      \
            } else if (hash == entry->hash && sv_eq(key, entry->key)) {                                                \
                return false;                                                                                          \
            }                                                                                                          \
                                                                                                                       \
            i = (i + 1) & mask;                                                                                        \
        };                                                                                                             \
                                                                                                                       \
        if (first_free) {                                                                                              \
            first_free->key = key;                                                                                     \
            first_free->hash = hash;                                                                                   \
            first_free->value = value;                                                                                 \
            first_free->state = HM_FULL;                                                                               \
            map->len++;                                                                                                \
            return true;                                                                                               \
        } else {                                                                                                       \
            Name##Map_grow(map);                                                                                       \
            return Name##Map_put(map, key, value);                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    bool Name##Map_set(Name##Map* map, StringView key, ValueType value) {                                              \
        if ((map->len + map->tombs) * 100 >= map->cap * 70) {                                                          \
            Name##Map_grow(map);                                                                                       \
        }                                                                                                              \
                                                                                                                       \
        uint64_t hash = hash_sv(key);                                                                                  \
        size_t mask = (map->cap - 1);                                                                                  \
        size_t i = hash & mask;                                                                                        \
                                                                                                                       \
        Name##Entry* first_free = NULL;                                                                                \
                                                                                                                       \
        for (size_t visited = 0; visited < map->cap; visited++) {                                                      \
            Name##Entry* entry = map->entries + i;                                                                     \
                                                                                                                       \
            if (entry->state == HM_EMPTY) {                                                                            \
                if (first_free) {                                                                                      \
                    entry = first_free;                                                                                \
                }                                                                                                      \
                                                                                                                       \
                entry->key = key;                                                                                      \
                entry->hash = hash;                                                                                    \
                entry->value = value;                                                                                  \
                entry->state = HM_FULL;                                                                                \
                map->len++;                                                                                            \
                return true;                                                                                           \
            } else if (entry->state == HM_DELETED) {                                                                   \
                if (!first_free) {                                                                                     \
                    first_free = entry;                                                                                \
                }                                                                                                      \
            } else if (hash == entry->hash && sv_eq(key, entry->key)) {                                                \
                entry->value = value;                                                                                  \
                return false;                                                                                          \
            }                                                                                                          \
                                                                                                                       \
            i = (i + 1) & mask;                                                                                        \
        };                                                                                                             \
                                                                                                                       \
        if (first_free) {                                                                                              \
            first_free->key = key;                                                                                     \
            first_free->hash = hash;                                                                                   \
            first_free->value = value;                                                                                 \
            first_free->state = HM_FULL;                                                                               \
            map->len++;                                                                                                \
            return true;                                                                                               \
        } else {                                                                                                       \
            Name##Map_grow(map);                                                                                       \
            return Name##Map_set(map, key, value);                                                                     \
        }                                                                                                              \
    }                                                                                                                  \
                                                                                                                       \
    bool Name##Map_get(Name##Map* map, StringView key, ValueType* out) {                                               \
        uint64_t hash = hash_sv(key);                                                                                  \
        size_t mask = (map->cap - 1);                                                                                  \
        size_t i = hash & mask;                                                                                        \
                                                                                                                       \
        for (size_t visited = 0; visited < map->cap; visited++) {                                                      \
            Name##Entry* entry = map->entries + i;                                                                     \
                                                                                                                       \
            if (entry->state == HM_EMPTY) {                                                                            \
                return false;                                                                                          \
            }                                                                                                          \
                                                                                                                       \
            if (entry->state == HM_FULL && hash == entry->hash && sv_eq(key, entry->key)) {                            \
                if (out) {                                                                                             \
                    *out = entry->value;                                                                               \
                }                                                                                                      \
                return true;                                                                                           \
            }                                                                                                          \
                                                                                                                       \
            i = (i + 1) & mask;                                                                                        \
        };                                                                                                             \
                                                                                                                       \
        return false;                                                                                                  \
    }                                                                                                                  \
                                                                                                                       \
    bool Name##Map_has(Name##Map* map, StringView key) { return Name##Map_get(map, key, NULL); }                       \
                                                                                                                       \
    bool Name##Map_del(Name##Map* map, StringView key) {                                                               \
        uint64_t hash = hash_sv(key);                                                                                  \
        size_t mask = (map->cap - 1);                                                                                  \
        size_t i = hash & mask;                                                                                        \
                                                                                                                       \
        for (size_t visited = 0; visited < map->cap; visited++) {                                                      \
            Name##Entry* entry = map->entries + i;                                                                     \
                                                                                                                       \
            if (entry->state == HM_EMPTY) {                                                                            \
                return false;                                                                                          \
            }                                                                                                          \
                                                                                                                       \
            if (entry->state == HM_FULL && hash == entry->hash && sv_eq(key, entry->key)) {                            \
                entry->state = HM_DELETED;                                                                             \
                map->len--;                                                                                            \
                map->tombs++;                                                                                          \
                return true;                                                                                           \
            }                                                                                                          \
                                                                                                                       \
            i = (i + 1) & mask;                                                                                        \
        };                                                                                                             \
                                                                                                                       \
        return false;                                                                                                  \
    }

#endif
