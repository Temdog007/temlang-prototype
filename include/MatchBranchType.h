
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum MatchBranchType
{
    MatchBranchType_Invalid = -1,
    MatchBranchType_None,
    MatchBranchType_Expression,
    MatchBranchType_Instructions
} MatchBranchType,
  *pMatchBranchType;

#define MatchBranchTypeCount 3
#define MatchBranchTypeLongestString 12

static const MatchBranchType MatchBranchTypeMembers[] = {
    MatchBranchType_None,
    MatchBranchType_Expression,
    MatchBranchType_Instructions
};

static inline MatchBranchType
MatchBranchTypeFromIndex(size_t index)
{
    if (index >= MatchBranchTypeCount) {
        return MatchBranchType_Invalid;
    }
    return MatchBranchTypeMembers[index];
}
static inline MatchBranchType
MatchBranchTypeFromString(const void* c, const size_t size)
{
    if (size > MatchBranchTypeLongestString) {
        return MatchBranchType_Invalid;
    }
    if (size == 4 && memcmp("None", c, 4) == 0) {
        return MatchBranchType_None;
    }
    if (size == 10 && memcmp("Expression", c, 10) == 0) {
        return MatchBranchType_Expression;
    }
    if (size == 12 && memcmp("Instructions", c, 12) == 0) {
        return MatchBranchType_Instructions;
    }
    return MatchBranchType_Invalid;
}
static inline MatchBranchType
MatchBranchTypeFromCaseInsensitiveString(const char* original,
                                         const size_t size)
{
    if (size > MatchBranchTypeLongestString) {
        return MatchBranchType_Invalid;
    }
    char c[MatchBranchTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 4 && memcmp("none", c, 4) == 0) {
        return MatchBranchType_None;
    }
    if (size == 10 && memcmp("expression", c, 10) == 0) {
        return MatchBranchType_Expression;
    }
    if (size == 12 && memcmp("instructions", c, 12) == 0) {
        return MatchBranchType_Instructions;
    }
    return MatchBranchType_Invalid;
}
static inline const char*
MatchBranchTypeToString(const MatchBranchType e)
{
    if (e == MatchBranchType_None) {
        return "None";
    }
    if (e == MatchBranchType_Expression) {
        return "Expression";
    }
    if (e == MatchBranchType_Instructions) {
        return "Instructions";
    }
    return "Invalid";
}