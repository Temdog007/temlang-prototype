
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum FunctionType
{
    FunctionType_Invalid = -1,
    FunctionType_Nullary,
    FunctionType_Unary,
    FunctionType_Binary,
    FunctionType_Procedure
} FunctionType,
  *pFunctionType;

#define FunctionTypeCount 4
#define FunctionTypeLongestString 9

static const FunctionType FunctionTypeMembers[] = { FunctionType_Nullary,
                                                    FunctionType_Unary,
                                                    FunctionType_Binary,
                                                    FunctionType_Procedure };

static inline FunctionType
FunctionTypeFromIndex(size_t index)
{
    if (index >= FunctionTypeCount) {
        return FunctionType_Invalid;
    }
    return FunctionTypeMembers[index];
}
static inline FunctionType
FunctionTypeFromString(const void* c, const size_t size)
{
    if (size > FunctionTypeLongestString) {
        return FunctionType_Invalid;
    }
    if (size == 7 && memcmp("Nullary", c, 7) == 0) {
        return FunctionType_Nullary;
    }
    if (size == 5 && memcmp("Unary", c, 5) == 0) {
        return FunctionType_Unary;
    }
    if (size == 6 && memcmp("Binary", c, 6) == 0) {
        return FunctionType_Binary;
    }
    if (size == 9 && memcmp("Procedure", c, 9) == 0) {
        return FunctionType_Procedure;
    }
    return FunctionType_Invalid;
}
static inline FunctionType
FunctionTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > FunctionTypeLongestString) {
        return FunctionType_Invalid;
    }
    char c[FunctionTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 7 && memcmp("nullary", c, 7) == 0) {
        return FunctionType_Nullary;
    }
    if (size == 5 && memcmp("unary", c, 5) == 0) {
        return FunctionType_Unary;
    }
    if (size == 6 && memcmp("binary", c, 6) == 0) {
        return FunctionType_Binary;
    }
    if (size == 9 && memcmp("procedure", c, 9) == 0) {
        return FunctionType_Procedure;
    }
    return FunctionType_Invalid;
}
static inline const char*
FunctionTypeToString(const FunctionType e)
{
    if (e == FunctionType_Nullary) {
        return "Nullary";
    }
    if (e == FunctionType_Unary) {
        return "Unary";
    }
    if (e == FunctionType_Binary) {
        return "Binary";
    }
    if (e == FunctionType_Procedure) {
        return "Procedure";
    }
    return "Invalid";
}