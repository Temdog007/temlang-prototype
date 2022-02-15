
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum NumberType
{
    NumberType_Invalid = -1,
    NumberType_Signed,
    NumberType_Unsigned,
    NumberType_Float
} NumberType,
  *pNumberType;

#define NumberTypeCount 3
#define NumberTypeLongestString 8

static const NumberType NumberTypeMembers[] = { NumberType_Signed,
                                                NumberType_Unsigned,
                                                NumberType_Float };

static inline NumberType
NumberTypeFromIndex(size_t index)
{
    if (index >= NumberTypeCount) {
        return NumberType_Invalid;
    }
    return NumberTypeMembers[index];
}
static inline NumberType
NumberTypeFromString(const void* c, const size_t size)
{
    if (size > NumberTypeLongestString) {
        return NumberType_Invalid;
    }
    if (size == 6 && memcmp("Signed", c, 6) == 0) {
        return NumberType_Signed;
    }
    if (size == 8 && memcmp("Unsigned", c, 8) == 0) {
        return NumberType_Unsigned;
    }
    if (size == 5 && memcmp("Float", c, 5) == 0) {
        return NumberType_Float;
    }
    return NumberType_Invalid;
}
static inline NumberType
NumberTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > NumberTypeLongestString) {
        return NumberType_Invalid;
    }
    char c[NumberTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 6 && memcmp("signed", c, 6) == 0) {
        return NumberType_Signed;
    }
    if (size == 8 && memcmp("unsigned", c, 8) == 0) {
        return NumberType_Unsigned;
    }
    if (size == 5 && memcmp("float", c, 5) == 0) {
        return NumberType_Float;
    }
    return NumberType_Invalid;
}
static inline const char*
NumberTypeToString(const NumberType e)
{
    if (e == NumberType_Signed) {
        return "Signed";
    }
    if (e == NumberType_Unsigned) {
        return "Unsigned";
    }
    if (e == NumberType_Float) {
        return "Float";
    }
    return "Invalid";
}