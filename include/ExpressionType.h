
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum ExpressionType
{
    ExpressionType_Invalid = -1,
    ExpressionType_Nullary,
    ExpressionType_UnaryValue,
    ExpressionType_UnaryVariable,
    ExpressionType_UnaryScope,
    ExpressionType_UnaryStruct,
    ExpressionType_UnaryList,
    ExpressionType_UnaryMatch,
    ExpressionType_Binary
} ExpressionType,
  *pExpressionType;

#define ExpressionTypeCount 8
#define ExpressionTypeLongestString 13

static const ExpressionType ExpressionTypeMembers[] = {
    ExpressionType_Nullary,       ExpressionType_UnaryValue,
    ExpressionType_UnaryVariable, ExpressionType_UnaryScope,
    ExpressionType_UnaryStruct,   ExpressionType_UnaryList,
    ExpressionType_UnaryMatch,    ExpressionType_Binary
};

static inline ExpressionType
ExpressionTypeFromIndex(size_t index)
{
    if (index >= ExpressionTypeCount) {
        return ExpressionType_Invalid;
    }
    return ExpressionTypeMembers[index];
}
static inline ExpressionType
ExpressionTypeFromString(const void* c, const size_t size)
{
    if (size > ExpressionTypeLongestString) {
        return ExpressionType_Invalid;
    }
    if (size == 7 && memcmp("Nullary", c, 7) == 0) {
        return ExpressionType_Nullary;
    }
    if (size == 10 && memcmp("UnaryValue", c, 10) == 0) {
        return ExpressionType_UnaryValue;
    }
    if (size == 13 && memcmp("UnaryVariable", c, 13) == 0) {
        return ExpressionType_UnaryVariable;
    }
    if (size == 10 && memcmp("UnaryScope", c, 10) == 0) {
        return ExpressionType_UnaryScope;
    }
    if (size == 11 && memcmp("UnaryStruct", c, 11) == 0) {
        return ExpressionType_UnaryStruct;
    }
    if (size == 9 && memcmp("UnaryList", c, 9) == 0) {
        return ExpressionType_UnaryList;
    }
    if (size == 10 && memcmp("UnaryMatch", c, 10) == 0) {
        return ExpressionType_UnaryMatch;
    }
    if (size == 6 && memcmp("Binary", c, 6) == 0) {
        return ExpressionType_Binary;
    }
    return ExpressionType_Invalid;
}
static inline ExpressionType
ExpressionTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > ExpressionTypeLongestString) {
        return ExpressionType_Invalid;
    }
    char c[ExpressionTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 7 && memcmp("nullary", c, 7) == 0) {
        return ExpressionType_Nullary;
    }
    if (size == 10 && memcmp("unaryvalue", c, 10) == 0) {
        return ExpressionType_UnaryValue;
    }
    if (size == 13 && memcmp("unaryvariable", c, 13) == 0) {
        return ExpressionType_UnaryVariable;
    }
    if (size == 10 && memcmp("unaryscope", c, 10) == 0) {
        return ExpressionType_UnaryScope;
    }
    if (size == 11 && memcmp("unarystruct", c, 11) == 0) {
        return ExpressionType_UnaryStruct;
    }
    if (size == 9 && memcmp("unarylist", c, 9) == 0) {
        return ExpressionType_UnaryList;
    }
    if (size == 10 && memcmp("unarymatch", c, 10) == 0) {
        return ExpressionType_UnaryMatch;
    }
    if (size == 6 && memcmp("binary", c, 6) == 0) {
        return ExpressionType_Binary;
    }
    return ExpressionType_Invalid;
}
static inline const char*
ExpressionTypeToString(const ExpressionType e)
{
    if (e == ExpressionType_Nullary) {
        return "Nullary";
    }
    if (e == ExpressionType_UnaryValue) {
        return "UnaryValue";
    }
    if (e == ExpressionType_UnaryVariable) {
        return "UnaryVariable";
    }
    if (e == ExpressionType_UnaryScope) {
        return "UnaryScope";
    }
    if (e == ExpressionType_UnaryStruct) {
        return "UnaryStruct";
    }
    if (e == ExpressionType_UnaryList) {
        return "UnaryList";
    }
    if (e == ExpressionType_UnaryMatch) {
        return "UnaryMatch";
    }
    if (e == ExpressionType_Binary) {
        return "Binary";
    }
    return "Invalid";
}