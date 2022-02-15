
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum BooleanOperator
{
    BooleanOperator_Invalid = -1,
    BooleanOperator_And,
    BooleanOperator_Or,
    BooleanOperator_Xor,
    BooleanOperator_Not
} BooleanOperator,
  *pBooleanOperator;

#define BooleanOperatorCount 4
#define BooleanOperatorLongestString 3

static const BooleanOperator BooleanOperatorMembers[] = { BooleanOperator_And,
                                                          BooleanOperator_Or,
                                                          BooleanOperator_Xor,
                                                          BooleanOperator_Not };

static inline BooleanOperator
BooleanOperatorFromIndex(size_t index)
{
    if (index >= BooleanOperatorCount) {
        return BooleanOperator_Invalid;
    }
    return BooleanOperatorMembers[index];
}
static inline BooleanOperator
BooleanOperatorFromString(const void* c, const size_t size)
{
    if (size > BooleanOperatorLongestString) {
        return BooleanOperator_Invalid;
    }
    if (size == 3 && memcmp("And", c, 3) == 0) {
        return BooleanOperator_And;
    }
    if (size == 2 && memcmp("Or", c, 2) == 0) {
        return BooleanOperator_Or;
    }
    if (size == 3 && memcmp("Xor", c, 3) == 0) {
        return BooleanOperator_Xor;
    }
    if (size == 3 && memcmp("Not", c, 3) == 0) {
        return BooleanOperator_Not;
    }
    return BooleanOperator_Invalid;
}
static inline BooleanOperator
BooleanOperatorFromCaseInsensitiveString(const char* original,
                                         const size_t size)
{
    if (size > BooleanOperatorLongestString) {
        return BooleanOperator_Invalid;
    }
    char c[BooleanOperatorLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 3 && memcmp("and", c, 3) == 0) {
        return BooleanOperator_And;
    }
    if (size == 2 && memcmp("or", c, 2) == 0) {
        return BooleanOperator_Or;
    }
    if (size == 3 && memcmp("xor", c, 3) == 0) {
        return BooleanOperator_Xor;
    }
    if (size == 3 && memcmp("not", c, 3) == 0) {
        return BooleanOperator_Not;
    }
    return BooleanOperator_Invalid;
}
static inline const char*
BooleanOperatorToString(const BooleanOperator e)
{
    if (e == BooleanOperator_And) {
        return "And";
    }
    if (e == BooleanOperator_Or) {
        return "Or";
    }
    if (e == BooleanOperator_Xor) {
        return "Xor";
    }
    if (e == BooleanOperator_Not) {
        return "Not";
    }
    return "Invalid";
}