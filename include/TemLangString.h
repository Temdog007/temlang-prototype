#pragma once

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "Allocator.h"
#include "ComparisonOperator.h"
#include "List.h"
#include "Misc.h"

typedef struct TemLangString
{
    char* buffer;
    // Don't use size_t. Use constant size for portability
    uint32_t size;
    uint32_t used;
    const Allocator* allocator;
} TemLangString, *pTemLangString;

static inline void
TemLangStringFree(pTemLangString s)
{
    if (s->buffer == NULL) {
        return;
    }
    s->allocator->free(s->buffer);
    memset(s, 0, sizeof(TemLangString));
}

static inline bool
TemLangStringCopy(pTemLangString dest,
                  const TemLangString* src,
                  const Allocator* allocator)
{
    TemLangStringFree(dest);
    dest->size = src->used + 1;
    dest->buffer = allocator->allocate(dest->size);
    memcpy(dest->buffer, src->buffer, src->used);
    dest->used = src->used;
    dest->buffer[dest->used] = '\0';
    dest->allocator = allocator;
    return true;
}

static inline TemLangString
TemLangStringClone(const TemLangString* s, const Allocator* allocator)
{
    TemLangString n = { 0 };
    TemLangStringCopy(&n, s, allocator);
    return n;
}

static inline void
TemLangStringNullTerminate(pTemLangString s)
{
    s->buffer[s->used] = '\0';
}

static inline TemLangString
TemLangStringCreateFromSize(const char* c,
                            const size_t size,
                            const Allocator* allocator)
{
    if (size == 0) {
        TemLangString s = {
            .allocator = allocator, .buffer = NULL, .used = 0, .size = 0
        };
        return s;
    }
    TemLangString s = { .buffer = allocator->allocate(size),
                        .size = size,
                        .used = size - 1,
                        .allocator = allocator };
    memcpy(s.buffer, c, size);
    TemLangStringNullTerminate(&s);
    return s;
}

static inline TemLangString
TemLangStringCreate(const char* c, const Allocator* allocator)
{
    const size_t size = strlen(c) + 1;
    return TemLangStringCreateFromSize(c, size, allocator);
}

static inline TemLangString
TemLangStringCreateWithSize(const size_t size, const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator,
                        .buffer = allocator->allocate(size),
                        .used = 0,
                        .size = size };
    return s;
}

static inline bool
TemLangStringRellocIfNeeded(pTemLangString s, const size_t addition)
{
    const size_t oldSize = s->size;
    if (s->size == 0) {
        s->size = addition * sizeof(char) + 1;
        goto doAlloc;
    } else if (s->used + addition >= s->size) {
        s->size += addition + 1;
        goto doAlloc;
    }
    return true;
doAlloc : {
    char* data = s->allocator->reallocate(s->buffer, s->size);
    if (data != NULL) {
        memset(data + oldSize, 0, s->size - oldSize);
        s->buffer = data;
        return true;
    }
    return false;
}
}

static inline bool
TemLangStringAppendChar(pTemLangString s, const char c)
{
    if (!TemLangStringRellocIfNeeded(s, 1)) {
        return false;
    }
    s->buffer[s->used] = c;
    ++s->used;
    TemLangStringNullTerminate(s);
    return true;
}

static inline bool
TemLangStringAppendChars(pTemLangString s, const char* c)
{
    while (*c != '\0') {
        if (!TemLangStringRellocIfNeeded(s, 1)) {
            return false;
        }
        s->buffer[s->used] = *c;
        ++s->used;
        ++c;
    }
    TemLangStringNullTerminate(s);
    return true;
}

static inline void
TemLangStringAppendSizedBuffer(pTemLangString s,
                               const char* c,
                               const size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        TemLangStringAppendChar(s, *c);
        ++c;
    }
    TemLangStringNullTerminate(s);
}

static inline bool
TemLangStringInsertChar(pTemLangString s, const char c, const size_t index)
{
    if (index >= s->used) {
        return TemLangStringAppendChar(s, c);
    }

    if (!TemLangStringRellocIfNeeded(s, 1)) {
        return false;
    }
    memmove(s->buffer + index + 1, s->buffer + index, s->used - index);
    s->buffer[index] = c;
    ++s->used;
    TemLangStringNullTerminate(s);
    return true;
}

static inline bool
TemLangStringIsEmpty(const TemLangString* s)
{
    return s->buffer == NULL || s->used == 0;
}

static inline bool
TemLangStringAppend(pTemLangString s, const TemLangString* other)
{
    if (TemLangStringIsEmpty(other)) {
        return true;
    }
    if (!TemLangStringRellocIfNeeded(s, other->used)) {
        return false;
    }
    memcpy(s->buffer + s->used, other->buffer, other->used);
    s->used += other->used;
    TemLangStringNullTerminate(s);
    return true;
}

static inline bool
TemLangStringAppendCount(pTemLangString s,
                         const char* buffer,
                         const size_t size)
{
    if (size == 0) {
        return true;
    }
    if (!TemLangStringRellocIfNeeded(s, size)) {
        return false;
    }
    memcpy(s->buffer + s->used, buffer, size);
    s->used += size;
    TemLangStringNullTerminate(s);
    return true;
}

static inline bool
TemLangStringInsert(pTemLangString s,
                    const TemLangString* other,
                    const size_t index)
{
    if (s->used + other->used - 1 >= s->size) {
        return TemLangStringAppend(s, other);
    }
    char* newBuffer = s->allocator->allocate(s->used + other->used + 1);
    memcpy(newBuffer, s->buffer, index);
    memcpy(newBuffer + index, other->buffer, other->used);
    memcpy(newBuffer + index + other->used, s->buffer + index, s->used - index);
    s->allocator->free(s->buffer);

    s->buffer = newBuffer;
    s->size = s->used + other->used + 1;
    s->used = s->used + other->used;
    TemLangStringNullTerminate(s);
    return true;
}

static inline char
TemLangStringPop(pTemLangString s)
{
    if (s->used == 0) {
        return '\0';
    }
    const char c = s->buffer[s->used - 1];
    --s->used;
    TemLangStringNullTerminate(s);
    return c;
}

static inline bool
TemLangStringRemove(pTemLangString s, const size_t index)
{
    if (index >= s->used) {
        return false;
    }

    memcpy(s->buffer + index, s->buffer + index + 1, s->used - index);
    --s->used;
    TemLangStringNullTerminate(s);
    return true;
}

static inline void
TemLangStringRemoveChunk(pTemLangString s,
                         const size_t start,
                         const size_t length)
{
    if (start + length >= s->used) {
        s->used = start;
        TemLangStringNullTerminate(s);
        return;
    }

    memmove(s->buffer + start,
            s->buffer + start + length,
            s->used - (start + length));
    s->used -= length;
    TemLangStringNullTerminate(s);
}

static inline void
TemLangStringRemoveNewLines(pTemLangString s)
{
    size_t i = 0;
    while (i < s->used) {
        switch (s->buffer[i]) {
            case '\n':
                TemLangStringRemove(s, i);
                TemLangStringInsertChar(s, 'n', i);
                TemLangStringInsertChar(s, '\\', i);
                i += 2;
                break;
            default:
                ++i;
                break;
        }
    }
}

static inline void
TemLangStringRemoveBackslahses(pTemLangString s)
{
    size_t i = 0;
    while (i < s->used) {
        switch (s->buffer[i]) {
            case '\\':
                TemLangStringRemove(s, i);
                ++i;
                break;
            default:
                ++i;
                break;
        }
    }
}

static inline void
TemLangStringRemoveChar(pTemLangString s, const char c)
{
    size_t i = 0;
    while (i < s->used) {
        if (s->buffer[i] == c) {
            TemLangStringRemove(s, i);
        } else {
            ++i;
        }
    }
}

#define TemLangStringCreateFormat(newS, newAllocator, format, ...)             \
                                                                               \
    TemLangString newS = { 0 };                                                \
    newS.allocator = newAllocator;                                             \
    newS.buffer = newS.allocator->allocate(1024);                              \
    newS.size = 1024;                                                          \
    do {                                                                       \
        const int offset =                                                     \
          snprintf(newS.buffer, newS.size, format, ##__VA_ARGS__);             \
        if ((size_t)offset < newS.size) {                                      \
            newS.used = offset;                                                \
            break;                                                             \
        }                                                                      \
        const size_t oldSize = newS.size;                                      \
        newS.size *= 2;                                                        \
        char* data = newS.allocator->reallocate(newS.buffer, newS.size);       \
        if (data != NULL) {                                                    \
            memset(data + oldSize, 0, newS.size - oldSize);                    \
            newS.buffer = data;                                                \
        }                                                                      \
    } while (true);

#define TemLangStringAppendFormat(s, format, ...)                              \
    {                                                                          \
        TemLangStringCreateFormat(newS, s.allocator, format, ##__VA_ARGS__);   \
        TemLangStringAppend(&s, &newS);                                        \
        TemLangStringFree(&newS);                                              \
    }

static inline ComparisonOperator
TemLangStringEquals(const TemLangString* a, const char* c)
{
    if (strlen(c) != a->used) {
        return false;
    }
    return memcmp(a->buffer, c, a->used) == 0;
}

static inline ComparisonOperator
TemLangStringCompare(const TemLangString* a, const TemLangString* b)
{
    if (a->buffer == b->buffer) {
        return ComparisonOperator_EqualTo;
    }
    if (a->buffer == NULL) {
        return ComparisonOperator_LessThan;
    }
    if (b->buffer == NULL) {
        return ComparisonOperator_GreaterThan;
    }

    const int i = memcmp(a->buffer, b->buffer, MIN(a->used, b->used));
    if (i > 0) {
        return ComparisonOperator_GreaterThan;
    } else if (i < 0) {
        return ComparisonOperator_LessThan;
    } else if (a->used > b->used) {
        return ComparisonOperator_GreaterThan;
    } else if (a->used < b->used) {
        return ComparisonOperator_LessThan;
    }
    return ComparisonOperator_EqualTo;
}

static inline bool
TemLangStringStartsWith(const TemLangString* s, const char* c)
{
    for (size_t i = 0; i < s->used && *c != '\0'; ++i, ++c) {
        if (s->buffer[i] != *c) {
            return false;
        }
    }
    return true;
}

static inline TemLangString
TemLangStringCombine(const TemLangString* a,
                     const TemLangString* b,
                     const Allocator* allocator)
{
    TemLangString s = { 0 };
    if (!TemLangStringCopy(&s, a, allocator)) {
        return s;
    }
    TemLangStringAppend(&s, b);
    return s;
}

MAKE_LIST(TemLangString);
DEFAULT_MAKE_LIST_FUNCTIONS(TemLangString);

static inline TemLangString
TemLangStringListToString(const TemLangStringList* list,
                          const Allocator* allocator)
{
    TemLangString s = TemLangStringCreate("[ ", allocator);
    for (size_t i = 0; i < list->used; ++i) {
        TemLangStringCreateFormat(
          temporaryString, allocator, "\"%s\"", list->buffer[i].buffer);
        TemLangStringAppend(&s, &temporaryString);
        TemLangStringFree(&temporaryString);
        if (i != list->used - 1) {
            TemLangStringAppendChars(&s, ", ");
        }
    }
    TemLangStringAppendChars(&s, " ]");
    return s;
}

static inline bool
TemLangStringsAreEqual(const TemLangString* a, const TemLangString* b)
{
    return TemLangStringCompare(a, b) == ComparisonOperator_EqualTo;
}

static inline bool
TemLangStringContainsSized(const TemLangString* a,
                           const char* c,
                           const size_t size)
{
    if (a->used == size) {
        return TemLangStringEquals(a, c);
    }
    for (size_t i = 0; i + size < a->used; ++i) {
        if (memcpy(&a->buffer[i], c, size) == 0) {
            return true;
        }
    }
    return false;
}

static inline bool
TemLangStringContains(const TemLangString* a, const char* c)
{
    return TemLangStringContainsSized(a, c, strlen(c));
}
