#ifndef STRING_VIEW_HEADER
#define STRING_VIEW_HEADER

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char* ptr;
    size_t len;
} StringView;

#define SV_Fmt "%.*s"
#define SV_Args(s) (int)((s).len), (s).ptr

static inline StringView sv_empty() { return (StringView){NULL, 0}; }

StringView sv_create(const char* cstr);
StringView sv_create_len(const char* cstr, size_t len);
StringView sv_create_end(const char* cstr, const char* end);
char* sv_to_cstr(StringView sv);
void sv_chopr(StringView* sv, size_t n);
void sv_chopl(StringView* sv, size_t n);
StringView sv_chop_by_delim(StringView* sv, char delim);
StringView sv_chop_by_type(StringView* sv, int (*type_func)(int));
StringView sv_chop_while(StringView* sv, int (*type_func)(int));
StringView sv_chop_until(StringView* sv, int (*type_func)(int));
StringView sv_triml(StringView sv);
StringView sv_trimr(StringView sv);
StringView sv_trim(StringView sv);
bool sv_eq(StringView a, StringView b);
bool sv_begins_with(StringView sv, StringView prefix);
bool sv_find(StringView haystack, StringView needle, size_t* pos);
bool sv_find_ch(StringView sv, char ch, size_t* pos);

#ifdef STRING_VIEW_IMPLEMENTATION

StringView sv_create(const char* cstr) { return (StringView){cstr, strlen(cstr)}; }
StringView sv_create_len(const char* cstr, size_t len) { return (StringView){cstr, len}; }
StringView sv_create_end(const char* cstr, const char* end) { return (StringView){cstr, end - cstr}; }
char* sv_to_cstr(StringView sv) {
    char* buf = malloc(sv.len + 1);
    memcpy(buf, sv.ptr, sv.len);
    buf[sv.len] = '\0';
    return buf;
}

void sv_chopr(StringView* sv, size_t n) {
    if (n >= sv->len)
        n = sv->len;
    sv->len -= n;
}

void sv_chopl(StringView* sv, size_t n) {
    if (n >= sv->len)
        n = sv->len;

    sv->len -= n;
    sv->ptr += n;
}

StringView sv_chop_by_delim(StringView* sv, char delim) {
    StringView split = *sv;

    void* res = memchr((char*)sv->ptr, delim, sv->len);
    if (!res) {
        split = *sv;
        *sv = sv_empty();
        return split;
    }

    size_t offset = (char*)res - sv->ptr;

    split.len = offset;

    if (offset < sv->len) {
        sv->ptr = res + 1;
        sv->len -= offset + 1;
    }

    return split;
}

StringView sv_chop_by_type(StringView* sv, int (*type_func)(int)) {
    while (sv->len && type_func((unsigned char)*sv->ptr)) {
        sv->ptr++;
        sv->len--;
    }

    StringView result = *sv;
    size_t i = 0;

    while (i < sv->len && !type_func((unsigned char)*sv->ptr)) {
        sv->ptr++;
        i++;
    }

    result.len = i;

    if (i < sv->len) {
        sv->ptr += 1;
        sv->len -= i + 1;
    } else {
        sv->len -= i;
    }

    return result;
}

StringView sv_chop_while(StringView* sv, int (*type_func)(int)) {
    StringView result = *sv;
    while (sv->len && type_func((unsigned char)sv->ptr[0])) {
        sv->ptr++;
        sv->len--;
    }

    result.len = sv->ptr - result.ptr;

    return result;
}

StringView sv_chop_until(StringView* sv, int (*type_func)(int)) {
    StringView result = *sv;
    while (sv->len && !type_func((unsigned char)sv->ptr[0])) {
        sv->ptr++;
        sv->len--;
    }

    result.len = sv->ptr - result.ptr;

    return result;
}

StringView sv_triml(StringView sv) {
    while (sv.len && isspace((unsigned char)sv.ptr[0])) {
        sv.ptr += 1;
        sv.len -= 1;
    }

    return sv;
}

StringView sv_trimr(StringView sv) {
    while (sv.len && isspace((unsigned char)sv.ptr[sv.len - 1])) {
        sv.len -= 1;
    }

    return sv;
}

StringView sv_trim(StringView sv) { return sv_trimr(sv_triml(sv)); }

bool sv_eq(StringView a, StringView b) { return a.len == b.len && memcmp(a.ptr, b.ptr, a.len) == 0; }

bool sv_begins_with(StringView sv, StringView prefix) {
    return sv.len >= prefix.len && memcmp(sv.ptr, prefix.ptr, prefix.len) == 0;
}

bool sv_find(StringView haystack, StringView needle, size_t* pos) {
    if (!needle.len) {
        if (pos) {
            *pos = 0;
        }

        return true;
    }

    if (needle.len > haystack.len) {
        return false;
    }

    const char* h = haystack.ptr;
    const char* n = needle.ptr;

    const char* end = h + haystack.len - needle.len + 1;

    while ((h = memchr(h, *n, end - h))) {
        if (memcmp(h, n, needle.len) == 0) {
            if (pos) {
                *pos = (size_t)(h - haystack.ptr);
            }

            return true;
        }

        h++;
    }

    return false;
}

bool sv_find_ch(StringView sv, char ch, size_t* pos) {
    void* res = memchr((char*)sv.ptr, ch, sv.len);

    if (!res) {
        return false;
    }

    if (pos) {
        *pos = (char*)res - sv.ptr;
    }

    return res;
}
#endif
#endif
