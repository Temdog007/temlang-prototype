
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum NumberOperator
{
    NumberOperator_Invalid = -1,
    NumberOperator_Add,
    NumberOperator_Subtract,
    NumberOperator_Multiply,
    NumberOperator_Divide,
    NumberOperator_Modulo
} NumberOperator,
  *pNumberOperator;

#define NumberOperatorCount 5
#define NumberOperatorLongestString 8

static const NumberOperator NumberOperatorMembers[] = { NumberOperator_Add,
                                                        NumberOperator_Subtract,
                                                        NumberOperator_Multiply,
                                                        NumberOperator_Divide,
                                                        NumberOperator_Modulo };

static inline NumberOperator
NumberOperatorFromIndex(size_t index)
{
    if (index >= NumberOperatorCount) {
        return NumberOperator_Invalid;
    }
    return NumberOperatorMembers[index];
}
static inline NumberOperator
NumberOperatorFromString(const void* c, const size_t size)
{
    if (size > NumberOperatorLongestString) {
        return NumberOperator_Invalid;
    }
    if (size == 3 && memcmp("Add", c, 3) == 0) {
        return NumberOperator_Add;
    }
    if (size == 8 && memcmp("Subtract", c, 8) == 0) {
        return NumberOperator_Subtract;
    }
    if (size == 8 && memcmp("Multiply", c, 8) == 0) {
        return NumberOperator_Multiply;
    }
    if (size == 6 && memcmp("Divide", c, 6) == 0) {
        return NumberOperator_Divide;
    }
    if (size == 6 && memcmp("Modulo", c, 6) == 0) {
        return NumberOperator_Modulo;
    }
    return NumberOperator_Invalid;
}
static inline NumberOperator
NumberOperatorFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > NumberOperatorLongestString) {
        return NumberOperator_Invalid;
    }
    char c[NumberOperatorLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 3 && memcmp("add", c, 3) == 0) {
        return NumberOperator_Add;
    }
    if (size == 8 && memcmp("subtract", c, 8) == 0) {
        return NumberOperator_Subtract;
    }
    if (size == 8 && memcmp("multiply", c, 8) == 0) {
        return NumberOperator_Multiply;
    }
    if (size == 6 && memcmp("divide", c, 6) == 0) {
        return NumberOperator_Divide;
    }
    if (size == 6 && memcmp("modulo", c, 6) == 0) {
        return NumberOperator_Modulo;
    }
    return NumberOperator_Invalid;
}
static inline const char*
NumberOperatorToString(const NumberOperator e)
{
    if (e == NumberOperator_Add) {
        return "Add";
    }
    if (e == NumberOperator_Subtract) {
        return "Subtract";
    }
    if (e == NumberOperator_Multiply) {
        return "Multiply";
    }
    if (e == NumberOperator_Divide) {
        return "Divide";
    }
    if (e == NumberOperator_Modulo) {
        return "Modulo";
    }
    return "Invalid";
}