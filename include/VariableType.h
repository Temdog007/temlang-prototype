
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum VariableType
{
    VariableType_Invalid = -1,
    VariableType_Immutable,
    VariableType_Mutable,
    VariableType_Constant
} VariableType,
  *pVariableType;

#define VariableTypeCount 3
#define VariableTypeLongestString 9

static const VariableType VariableTypeMembers[] = { VariableType_Immutable,
                                                    VariableType_Mutable,
                                                    VariableType_Constant };

static inline VariableType
VariableTypeFromIndex(size_t index)
{
    if (index >= VariableTypeCount) {
        return VariableType_Invalid;
    }
    return VariableTypeMembers[index];
}
static inline VariableType
VariableTypeFromString(const void* c, const size_t size)
{
    if (size > VariableTypeLongestString) {
        return VariableType_Invalid;
    }
    if (size == 9 && memcmp("Immutable", c, 9) == 0) {
        return VariableType_Immutable;
    }
    if (size == 7 && memcmp("Mutable", c, 7) == 0) {
        return VariableType_Mutable;
    }
    if (size == 8 && memcmp("Constant", c, 8) == 0) {
        return VariableType_Constant;
    }
    return VariableType_Invalid;
}
static inline VariableType
VariableTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > VariableTypeLongestString) {
        return VariableType_Invalid;
    }
    char c[VariableTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 9 && memcmp("immutable", c, 9) == 0) {
        return VariableType_Immutable;
    }
    if (size == 7 && memcmp("mutable", c, 7) == 0) {
        return VariableType_Mutable;
    }
    if (size == 8 && memcmp("constant", c, 8) == 0) {
        return VariableType_Constant;
    }
    return VariableType_Invalid;
}
static inline const char*
VariableTypeToString(const VariableType e)
{
    if (e == VariableType_Immutable) {
        return "Immutable";
    }
    if (e == VariableType_Mutable) {
        return "Mutable";
    }
    if (e == VariableType_Constant) {
        return "Constant";
    }
    return "Invalid";
}