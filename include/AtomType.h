
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum AtomType
{
    AtomType_Invalid = -1,
    AtomType_Variable,
    AtomType_Function,
    AtomType_Enum,
    AtomType_Range,
    AtomType_Struct
} AtomType,
  *pAtomType;

#define AtomTypeCount 5
#define AtomTypeLongestString 8

static const AtomType AtomTypeMembers[] = { AtomType_Variable,
                                            AtomType_Function,
                                            AtomType_Enum,
                                            AtomType_Range,
                                            AtomType_Struct };

static inline AtomType
AtomTypeFromIndex(size_t index)
{
    if (index >= AtomTypeCount) {
        return AtomType_Invalid;
    }
    return AtomTypeMembers[index];
}
static inline AtomType
AtomTypeFromString(const void* c, const size_t size)
{
    if (size > AtomTypeLongestString) {
        return AtomType_Invalid;
    }
    if (size == 8 && memcmp("Variable", c, 8) == 0) {
        return AtomType_Variable;
    }
    if (size == 8 && memcmp("Function", c, 8) == 0) {
        return AtomType_Function;
    }
    if (size == 4 && memcmp("Enum", c, 4) == 0) {
        return AtomType_Enum;
    }
    if (size == 5 && memcmp("Range", c, 5) == 0) {
        return AtomType_Range;
    }
    if (size == 6 && memcmp("Struct", c, 6) == 0) {
        return AtomType_Struct;
    }
    return AtomType_Invalid;
}
static inline AtomType
AtomTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > AtomTypeLongestString) {
        return AtomType_Invalid;
    }
    char c[AtomTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 8 && memcmp("variable", c, 8) == 0) {
        return AtomType_Variable;
    }
    if (size == 8 && memcmp("function", c, 8) == 0) {
        return AtomType_Function;
    }
    if (size == 4 && memcmp("enum", c, 4) == 0) {
        return AtomType_Enum;
    }
    if (size == 5 && memcmp("range", c, 5) == 0) {
        return AtomType_Range;
    }
    if (size == 6 && memcmp("struct", c, 6) == 0) {
        return AtomType_Struct;
    }
    return AtomType_Invalid;
}
static inline const char*
AtomTypeToString(const AtomType e)
{
    if (e == AtomType_Variable) {
        return "Variable";
    }
    if (e == AtomType_Function) {
        return "Function";
    }
    if (e == AtomType_Enum) {
        return "Enum";
    }
    if (e == AtomType_Range) {
        return "Range";
    }
    if (e == AtomType_Struct) {
        return "Struct";
    }
    return "Invalid";
}