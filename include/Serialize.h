#pragma once

#include "Includes.h"
#include "Number.h"

#include <endian.h>

static inline size_t
uint8_tSerialize(const uint8_t* value, pBytes bytes, const bool littleEndian)
{
    (void)littleEndian;
    if (uint8_tListAppend(bytes, value)) {
        return sizeof(uint8_t);
    } else {
        return 0UL;
    }
}

static inline size_t
uint8_tDeserialize(uint8_t* value,
                   const Bytes* bytes,
                   const size_t offset,
                   const bool littleEndian)
{
    (void)littleEndian;
    if (offset + sizeof(uint8_t) > bytes->used) {
        return 0UL;
    }

    *value = bytes->buffer[offset];
    return sizeof(uint8_t);
}

static inline size_t
uint16_tSerialize(const uint16_t* value, pBytes bytes, const bool littleEndian)
{
    union
    {
        uint16_t value;
        uint8_t bytes[sizeof(uint16_t)];
    } f;
    if (littleEndian) {
        f.value = htole16(*value);
    } else {
        f.value = htobe16(*value);
    }
    for (size_t i = 0; i < sizeof(uint16_t); ++i) {
        if (!uint8_tListAppend(bytes, &f.bytes[i])) {
            return i;
        }
    }
    return sizeof(uint16_t);
}

static inline size_t
uint16_tDeserialize(uint16_t* value,
                    const Bytes* bytes,
                    const size_t offset,
                    const bool littleEndian)
{
    if (offset + sizeof(uint16_t) > bytes->used) {
        return 0UL;
    }

    union
    {
        uint16_t value;
        uint8_t bytes[sizeof(uint16_t)];
    } f;
    memcpy(f.bytes, &bytes->buffer[offset], sizeof(uint16_t));
    if (littleEndian) {
        *value = le16toh(f.value);
    } else {
        *value = be16toh(f.value);
    }
    return sizeof(uint16_t);
}

static inline size_t
uint32_tSerialize(const uint32_t* value, pBytes bytes, const bool littleEndian)
{
    union
    {
        uint32_t value;
        uint8_t bytes[sizeof(uint32_t)];
    } f;
    if (littleEndian) {
        f.value = htole32(*value);
    } else {
        f.value = htobe32(*value);
    }
    for (size_t i = 0; i < sizeof(uint32_t); ++i) {
        if (!uint8_tListAppend(bytes, &f.bytes[i])) {
            return i;
        }
    }
    return sizeof(uint32_t);
}

static inline size_t
uint32_tDeserialize(uint32_t* value,
                    const Bytes* bytes,
                    const size_t offset,
                    const bool littleEndian)
{
    if (offset + sizeof(uint32_t) > bytes->used) {
        return 0UL;
    }

    union
    {
        uint32_t value;
        uint8_t bytes[sizeof(uint32_t)];
    } f;
    memcpy(f.bytes, &bytes->buffer[offset], sizeof(uint32_t));
    if (littleEndian) {
        *value = le32toh(f.value);
    } else {
        *value = be32toh(f.value);
    }
    return sizeof(uint32_t);
}

static inline size_t
uint64_tSerialize(const uint64_t* value, pBytes bytes, const bool littleEndian)
{
    union
    {
        uint64_t value;
        uint8_t bytes[sizeof(uint64_t)];
    } f;
    if (littleEndian) {
        f.value = htole64(*value);
    } else {
        f.value = htobe64(*value);
    }
    for (size_t i = 0; i < sizeof(uint64_t); ++i) {
        if (!uint8_tListAppend(bytes, &f.bytes[i])) {
            return i;
        }
    }
    return sizeof(uint64_t);
}

static inline size_t
uint64_tDeserialize(uint64_t* value,
                    const Bytes* bytes,
                    const size_t offset,
                    const bool littleEndian)
{
    if (offset + sizeof(uint64_t) > bytes->used) {
        return 0UL;
    }

    union
    {
        uint64_t value;
        uint8_t bytes[sizeof(uint64_t)];
    } f;
    memcpy(f.bytes, &bytes->buffer[offset], sizeof(uint64_t));
    if (littleEndian) {
        *value = le64toh(f.value);
    } else {
        *value = be64toh(f.value);
    }
    return sizeof(uint64_t);
}

static inline size_t
int8_tSerialize(const int8_t* value, pBytes bytes, const bool littleEndian)
{
    return uint8_tSerialize((const uint8_t*)value, bytes, littleEndian);
}

static inline size_t
int8_tDeserialize(int8_t* value,
                  const Bytes* bytes,
                  const size_t offset,
                  const bool littleEndian)
{
    return uint8_tDeserialize((uint8_t*)value, bytes, offset, littleEndian);
}

static inline size_t
int16_tSerialize(const int16_t* value, pBytes bytes, const bool littleEndian)
{
    return uint16_tSerialize((const uint16_t*)value, bytes, littleEndian);
}

static inline size_t
int16_tDeserialize(int16_t* value,
                   const Bytes* bytes,
                   const size_t offset,
                   const bool littleEndian)
{
    return uint16_tDeserialize((uint16_t*)value, bytes, offset, littleEndian);
}

static inline size_t
int32_tSerialize(const int32_t* value, pBytes bytes, const bool littleEndian)
{
    return uint32_tSerialize((const uint32_t*)value, bytes, littleEndian);
}

static inline size_t
int32_tDeserialize(int32_t* value,
                   const Bytes* bytes,
                   const size_t offset,
                   const bool littleEndian)
{
    return uint32_tDeserialize((uint32_t*)value, bytes, offset, littleEndian);
}

static inline size_t
floatSerialize(const float* value, pBytes bytes, const bool littleEndian)
{
    return uint32_tSerialize((const uint32_t*)value, bytes, littleEndian);
}

static inline size_t
floatDeserialize(float* value,
                 const Bytes* bytes,
                 const size_t offset,
                 const bool littleEndian)
{
    return uint32_tDeserialize((uint32_t*)value, bytes, offset, littleEndian);
}

static inline size_t
int64_tSerialize(const int64_t* value, pBytes bytes, const bool littleEndian)
{
    return uint64_tSerialize((const uint64_t*)value, bytes, littleEndian);
}

static inline size_t
int64_tDeserialize(int64_t* value,
                   const Bytes* bytes,
                   const size_t offset,
                   const bool littleEndian)
{
    return uint64_tDeserialize((uint64_t*)value, bytes, offset, littleEndian);
}

static inline size_t
doubleSerialize(const double* value, pBytes bytes, const bool littleEndian)
{
    return uint64_tSerialize((const uint64_t*)value, bytes, littleEndian);
}

static inline size_t
doubleDeserialize(double* value,
                  const Bytes* bytes,
                  const size_t offset,
                  const bool littleEndian)
{
    return uint64_tDeserialize((uint64_t*)value, bytes, offset, littleEndian);
}

static inline size_t
TemLangStringSerialize(const TemLangString* string,
                       pBytes bytes,
                       const bool littleEndian)
{
    const uint32_t s = uint32_tSerialize(&string->used, bytes, littleEndian);
    uint32_t i = 0UL;
    for (; i < string->used; ++i) {
        const uint8_t c = *(uint8_t*)&string->buffer[i];
        if (!uint8_tListAppend(bytes, &c)) {
            break;
        }
    }
    return s + i;
}

static inline size_t
TemLangStringDeserialize(TemLangString* string,
                         const Bytes* bytes,
                         const size_t offset,
                         const bool littleEndian)
{
    if (string->allocator == NULL) {
        if (bytes->allocator == NULL) {
            return 0UL;
        }
        memset(string, 0, sizeof(*string));
        string->allocator = bytes->allocator;
    }
    uint32_t count = 0U;
    const uint32_t s = uint32_tDeserialize(&count, bytes, offset, littleEndian);
    uint32_t i = 0UL;
    for (; i < count && offset + i + s < bytes->used; ++i) {
        const char c = *(char*)&bytes->buffer[offset + i + s];
        if (!TemLangStringAppendChar(string, c)) {
            break;
        }
    }
    return s + i;
}

static inline size_t
booleanSerialize(const boolean* b, pBytes bytes, const bool littleEndian)
{
    return uint8_tSerialize((const uint8_t*)b, bytes, littleEndian);
}

static inline size_t
booleanDeserialize(boolean* b,
                   const Bytes* bytes,
                   const size_t offset,
                   const bool littleEndian)
{
    return uint8_tDeserialize((uint8_t*)b, bytes, offset, littleEndian);
}

static inline size_t
NullValueSerialize(const NullValue* n, pBytes bytes, const bool e)
{
    (void)n;
    (void)bytes;
    (void)e;
    return 0UL;
}

static inline size_t
NullValueDeserialize(NullValue* n,
                     const Bytes* bytes,
                     const size_t offset,
                     const bool e)
{
    (void)n;
    (void)bytes;
    (void)e;
    (void)offset;
    return 0UL;
}

#define SIGNED_NUMBER_TO_STRING(T)                                             \
    static inline TemLangString T##ToString(const T value, const Allocator* a) \
    {                                                                          \
        Number n = { .type = NumberType_Signed, .i = value };                  \
        return NumberToString(&n, a);                                          \
    }                                                                          \
    static inline T T##FromString(const TemLangString* s)                      \
    {                                                                          \
        char* e = NULL;                                                        \
        return (T)strtoll(s->buffer, &e, 10);                                  \
    }

SIGNED_NUMBER_TO_STRING(int8_t);
SIGNED_NUMBER_TO_STRING(int16_t);
SIGNED_NUMBER_TO_STRING(int32_t);
SIGNED_NUMBER_TO_STRING(int64_t);

#define UNSIGNED_NUMBER_TO_STRING(T)                                           \
    static inline TemLangString T##ToString(const T value, const Allocator* a) \
    {                                                                          \
        Number n = { .type = NumberType_Unsigned, .u = value };                \
        return NumberToString(&n, a);                                          \
    }                                                                          \
    static inline T T##FromString(const TemLangString* s)                      \
    {                                                                          \
        char* e = NULL;                                                        \
        return (T)strtoul(s->buffer, &e, 10);                                  \
    }

SIGNED_NUMBER_TO_STRING(uint8_t);
SIGNED_NUMBER_TO_STRING(uint16_t);
SIGNED_NUMBER_TO_STRING(uint32_t);
SIGNED_NUMBER_TO_STRING(uint64_t);

static inline TemLangString
doubleToString(const double value, const Allocator* a)
{
    Number n = { .type = NumberType_Float, .d = value };
    return NumberToString(&n, a);
}

static inline double
doubleFromString(const TemLangString* s)
{
    char* e = NULL;
    return strtod(s->buffer, &e);
}

static inline TemLangString
floatToString(const float value, const Allocator* a)
{
    Number n = { .type = NumberType_Float, .d = value };
    return NumberToString(&n, a);
}

static inline float
floatFromString(const TemLangString* s)
{
    char* e = NULL;
    return strtof(s->buffer, &e);
}