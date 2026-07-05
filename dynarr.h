#ifndef DYNARR_HEADER
#define DYNARR_HEADER

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define DYNARR_INITIAL_CAP 8

typedef struct ArrayHeader ArrayHeader;
struct ArrayHeader {
    size_t len;
    size_t cap;
};

#define da_header_sz(ptr) (_Alignof(*ptr) > sizeof(ArrayHeader) ? _Alignof(*ptr) : sizeof(ArrayHeader))
#define da_header(ptr) ((ArrayHeader*)((char*)ptr - da_header_sz(ptr)))

#define da_create(ptr) da_create_sz(ptr, DYNARR_INITIAL_CAP)
#define da_create_sz(ptr, initial_cap)                                                                                 \
    do {                                                                                                               \
        ArrayHeader* buf = malloc(da_header_sz(ptr) + initial_cap * sizeof(*ptr)); \
        buf->cap = initial_cap;                                                                                        \
        buf->len = 0;                                                                                                  \
        ptr = (void*)(buf + 1);                                                                                        \
    } while (0)

#define da_append(ptr, val)                                                                                            \
    do {                                                                                                               \
        ArrayHeader* header = da_header(ptr);                                                                          \
        if (header->len >= header->cap) {                                                                              \
            size_t new_cap = header->cap * 2;                                                                          \
            void* tmp = realloc(header, da_header_sz(ptr) + new_cap * sizeof(*ptr)); \
            if (!tmp) {                                                                                                \
                fprintf(stderr, "fatal: out of memory\n");                                                             \
                exit(EXIT_FAILURE);                                                                                    \
            }                                                                                                          \
            header = tmp;                                                                                              \
            header->cap = new_cap;                                                                                     \
        }                                                                                                              \
                                                                                                                       \
        ptr = (void*)(header + 1);                                                                                     \
        ptr[header->len++] = val;                                                                                      \
    } while (0)

#define da_free(ptr)                                                                                                 \
    do {                                                                                                               \
        free(da_header(ptr));                                                                                          \
        ptr = NULL                                                                                                     \
    } while (0)

#define da_len(ptr) (da_header(ptr)->len)
#define da_cap(ptr) (da_header(ptr)->cap)

#endif
