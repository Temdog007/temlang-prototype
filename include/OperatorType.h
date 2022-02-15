
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum OperatorType
{
    OperatorType_Invalid = -1,
    OperatorType_Get,
    OperatorType_Number,
    OperatorType_Comparison,
    OperatorType_Function,
    OperatorType_Boolean
} OperatorType,
  *pOperatorType;

#define OperatorTypeCount 5
#define OperatorTypeLongestString 10

static const OperatorType OperatorTypeMembers[] = { OperatorType_Get,
                                                    OperatorType_Number,
                                                    OperatorType_Comparison,
                                                    OperatorType_Function,
                                                    OperatorType_Boolean };

static inline OperatorType
OperatorTypeFromIndex(size_t index)
{
    if (index >= OperatorTypeCount) {
        return OperatorType_Invalid;
    }
    return OperatorTypeMembers[index];
}
static inline OperatorType
OperatorTypeFromString(const void* c, const size_t size)
{
    if (size > OperatorTypeLongestString) {
        return OperatorType_Invalid;
    }
    if (size == 3 && memcmp("Get", c, 3) == 0) {
        return OperatorType_Get;
    }
    if (size == 6 && memcmp("Number", c, 6) == 0) {
        return OperatorType_Number;
    }
    if (size == 10 && memcmp("Comparison", c, 10) == 0) {
        return OperatorType_Comparison;
    }
    if (size == 8 && memcmp("Function", c, 8) == 0) {
        return OperatorType_Function;
    }
    if (size == 7 && memcmp("Boolean", c, 7) == 0) {
        return OperatorType_Boolean;
    }
    return OperatorType_Invalid;
}
static inline OperatorType
OperatorTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > OperatorTypeLongestString) {
        return OperatorType_Invalid;
    }
    char c[OperatorTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 3 && memcmp("get", c, 3) == 0) {
        return OperatorType_Get;
    }
    if (size == 6 && memcmp("number", c, 6) == 0) {
        return OperatorType_Number;
    }
    if (size == 10 && memcmp("comparison", c, 10) == 0) {
        return OperatorType_Comparison;
    }
    if (size == 8 && memcmp("function", c, 8) == 0) {
        return OperatorType_Function;
    }
    if (size == 7 && memcmp("boolean", c, 7) == 0) {
        return OperatorType_Boolean;
    }
    return OperatorType_Invalid;
}
static inline const char*
OperatorTypeToString(const OperatorType e)
{
    if (e == OperatorType_Get) {
        return "Get";
    }
    if (e == OperatorType_Number) {
        return "Number";
    }
    if (e == OperatorType_Comparison) {
        return "Comparison";
    }
    if (e == OperatorType_Function) {
        return "Function";
    }
    if (e == OperatorType_Boolean) {
        return "Boolean";
    }
    return "Invalid";
}