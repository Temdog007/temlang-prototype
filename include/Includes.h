#pragma once

#include <List.h>
#include <TemLangString.h>
#include <math.h>
#include <memory.h>
#include <stdbool.h>
#include <stdint.h>

#define PRIMITIVE_MAKE_LIST_FUNCTIONS(T)                                       \
    MAKE_COPY_AND_FREE(T);                                                     \
    MAKE_LIST(T);                                                              \
    DEFAULT_MAKE_LIST_FUNCTIONS(T)

typedef bool boolean;
PRIMITIVE_MAKE_LIST_FUNCTIONS(boolean);
PRIMITIVE_MAKE_LIST_FUNCTIONS(int8_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(int16_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(int32_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(int64_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(uint8_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(uint16_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(uint32_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(uint64_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(size_t);
PRIMITIVE_MAKE_LIST_FUNCTIONS(float);
PRIMITIVE_MAKE_LIST_FUNCTIONS(double);

typedef void* NullValue;
PRIMITIVE_MAKE_LIST_FUNCTIONS(NullValue);

typedef uint8_tList Bytes;
typedef uint8_tList* pBytes;
extern const Allocator* currentAllocator;

typedef const char* CString;
#define STR_EQUALS(a, b, len, f)                                               \
    if (len == sizeof(b) - 1 && memcmp(a, b, len) == 0) {                      \
        f                                                                      \
    }

#include "IncludesCGLM.h"