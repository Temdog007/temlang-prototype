
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum ComparisonOperator
{
    ComparisonOperator_Invalid = -1,
    ComparisonOperator_LessThan,
    ComparisonOperator_EqualTo,
    ComparisonOperator_GreaterThan
} ComparisonOperator,
  *pComparisonOperator;

#define ComparisonOperatorCount 3
#define ComparisonOperatorLongestString 11

static const ComparisonOperator ComparisonOperatorMembers[] = {
    ComparisonOperator_LessThan,
    ComparisonOperator_EqualTo,
    ComparisonOperator_GreaterThan
};

static inline ComparisonOperator
ComparisonOperatorFromIndex(size_t index)
{
    if (index >= ComparisonOperatorCount) {
        return ComparisonOperator_Invalid;
    }
    return ComparisonOperatorMembers[index];
}
static inline ComparisonOperator
ComparisonOperatorFromString(const void* c, const size_t size)
{
    if (size > ComparisonOperatorLongestString) {
        return ComparisonOperator_Invalid;
    }
    if (size == 8 && memcmp("LessThan", c, 8) == 0) {
        return ComparisonOperator_LessThan;
    }
    if (size == 7 && memcmp("EqualTo", c, 7) == 0) {
        return ComparisonOperator_EqualTo;
    }
    if (size == 11 && memcmp("GreaterThan", c, 11) == 0) {
        return ComparisonOperator_GreaterThan;
    }
    return ComparisonOperator_Invalid;
}
static inline ComparisonOperator
ComparisonOperatorFromCaseInsensitiveString(const char* original,
                                            const size_t size)
{
    if (size > ComparisonOperatorLongestString) {
        return ComparisonOperator_Invalid;
    }
    char c[ComparisonOperatorLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 8 && memcmp("lessthan", c, 8) == 0) {
        return ComparisonOperator_LessThan;
    }
    if (size == 7 && memcmp("equalto", c, 7) == 0) {
        return ComparisonOperator_EqualTo;
    }
    if (size == 11 && memcmp("greaterthan", c, 11) == 0) {
        return ComparisonOperator_GreaterThan;
    }
    return ComparisonOperator_Invalid;
}
static inline const char*
ComparisonOperatorToString(const ComparisonOperator e)
{
    if (e == ComparisonOperator_LessThan) {
        return "LessThan";
    }
    if (e == ComparisonOperator_EqualTo) {
        return "EqualTo";
    }
    if (e == ComparisonOperator_GreaterThan) {
        return "GreaterThan";
    }
    return "Invalid";
}