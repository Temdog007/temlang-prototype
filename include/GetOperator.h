
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum GetOperator
{
    GetOperator_Invalid = -1,
    GetOperator_Member,
    GetOperator_Type,
    GetOperator_MakeList,
    GetOperator_Length
} GetOperator,
  *pGetOperator;

#define GetOperatorCount 4
#define GetOperatorLongestString 8

static const GetOperator GetOperatorMembers[] = { GetOperator_Member,
                                                  GetOperator_Type,
                                                  GetOperator_MakeList,
                                                  GetOperator_Length };

static inline GetOperator
GetOperatorFromIndex(size_t index)
{
    if (index >= GetOperatorCount) {
        return GetOperator_Invalid;
    }
    return GetOperatorMembers[index];
}
static inline GetOperator
GetOperatorFromString(const void* c, const size_t size)
{
    if (size > GetOperatorLongestString) {
        return GetOperator_Invalid;
    }
    if (size == 6 && memcmp("Member", c, 6) == 0) {
        return GetOperator_Member;
    }
    if (size == 4 && memcmp("Type", c, 4) == 0) {
        return GetOperator_Type;
    }
    if (size == 8 && memcmp("MakeList", c, 8) == 0) {
        return GetOperator_MakeList;
    }
    if (size == 6 && memcmp("Length", c, 6) == 0) {
        return GetOperator_Length;
    }
    return GetOperator_Invalid;
}
static inline GetOperator
GetOperatorFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > GetOperatorLongestString) {
        return GetOperator_Invalid;
    }
    char c[GetOperatorLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 6 && memcmp("member", c, 6) == 0) {
        return GetOperator_Member;
    }
    if (size == 4 && memcmp("type", c, 4) == 0) {
        return GetOperator_Type;
    }
    if (size == 8 && memcmp("makelist", c, 8) == 0) {
        return GetOperator_MakeList;
    }
    if (size == 6 && memcmp("length", c, 6) == 0) {
        return GetOperator_Length;
    }
    return GetOperator_Invalid;
}
static inline const char*
GetOperatorToString(const GetOperator e)
{
    if (e == GetOperator_Member) {
        return "Member";
    }
    if (e == GetOperator_Type) {
        return "Type";
    }
    if (e == GetOperator_MakeList) {
        return "MakeList";
    }
    if (e == GetOperator_Length) {
        return "Length";
    }
    return "Invalid";
}