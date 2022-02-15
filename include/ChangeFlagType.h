
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum ChangeFlagType
{
    ChangeFlagType_Invalid = -1,
    ChangeFlagType_Add,
    ChangeFlagType_Remove,
    ChangeFlagType_Toggle
} ChangeFlagType,
  *pChangeFlagType;

#define ChangeFlagTypeCount 3
#define ChangeFlagTypeLongestString 6

static const ChangeFlagType ChangeFlagTypeMembers[] = { ChangeFlagType_Add,
                                                        ChangeFlagType_Remove,
                                                        ChangeFlagType_Toggle };

static inline ChangeFlagType
ChangeFlagTypeFromIndex(size_t index)
{
    if (index >= ChangeFlagTypeCount) {
        return ChangeFlagType_Invalid;
    }
    return ChangeFlagTypeMembers[index];
}
static inline ChangeFlagType
ChangeFlagTypeFromString(const void* c, const size_t size)
{
    if (size > ChangeFlagTypeLongestString) {
        return ChangeFlagType_Invalid;
    }
    if (size == 3 && memcmp("Add", c, 3) == 0) {
        return ChangeFlagType_Add;
    }
    if (size == 6 && memcmp("Remove", c, 6) == 0) {
        return ChangeFlagType_Remove;
    }
    if (size == 6 && memcmp("Toggle", c, 6) == 0) {
        return ChangeFlagType_Toggle;
    }
    return ChangeFlagType_Invalid;
}
static inline ChangeFlagType
ChangeFlagTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > ChangeFlagTypeLongestString) {
        return ChangeFlagType_Invalid;
    }
    char c[ChangeFlagTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 3 && memcmp("add", c, 3) == 0) {
        return ChangeFlagType_Add;
    }
    if (size == 6 && memcmp("remove", c, 6) == 0) {
        return ChangeFlagType_Remove;
    }
    if (size == 6 && memcmp("toggle", c, 6) == 0) {
        return ChangeFlagType_Toggle;
    }
    return ChangeFlagType_Invalid;
}
static inline const char*
ChangeFlagTypeToString(const ChangeFlagType e)
{
    if (e == ChangeFlagType_Add) {
        return "Add";
    }
    if (e == ChangeFlagType_Remove) {
        return "Remove";
    }
    if (e == ChangeFlagType_Toggle) {
        return "Toggle";
    }
    return "Invalid";
}