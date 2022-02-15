
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum ListModifyType
{
    ListModifyType_Invalid = -1,
    ListModifyType_Append,
    ListModifyType_Insert,
    ListModifyType_Remove,
    ListModifyType_SwapRemove,
    ListModifyType_Pop,
    ListModifyType_Empty
} ListModifyType,
  *pListModifyType;

#define ListModifyTypeCount 6
#define ListModifyTypeLongestString 10

static const ListModifyType ListModifyTypeMembers[] = {
    ListModifyType_Append,     ListModifyType_Insert, ListModifyType_Remove,
    ListModifyType_SwapRemove, ListModifyType_Pop,    ListModifyType_Empty
};

static inline ListModifyType
ListModifyTypeFromIndex(size_t index)
{
    if (index >= ListModifyTypeCount) {
        return ListModifyType_Invalid;
    }
    return ListModifyTypeMembers[index];
}
static inline ListModifyType
ListModifyTypeFromString(const void* c, const size_t size)
{
    if (size > ListModifyTypeLongestString) {
        return ListModifyType_Invalid;
    }
    if (size == 6 && memcmp("Append", c, 6) == 0) {
        return ListModifyType_Append;
    }
    if (size == 6 && memcmp("Insert", c, 6) == 0) {
        return ListModifyType_Insert;
    }
    if (size == 6 && memcmp("Remove", c, 6) == 0) {
        return ListModifyType_Remove;
    }
    if (size == 10 && memcmp("SwapRemove", c, 10) == 0) {
        return ListModifyType_SwapRemove;
    }
    if (size == 3 && memcmp("Pop", c, 3) == 0) {
        return ListModifyType_Pop;
    }
    if (size == 5 && memcmp("Empty", c, 5) == 0) {
        return ListModifyType_Empty;
    }
    return ListModifyType_Invalid;
}
static inline ListModifyType
ListModifyTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > ListModifyTypeLongestString) {
        return ListModifyType_Invalid;
    }
    char c[ListModifyTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 6 && memcmp("append", c, 6) == 0) {
        return ListModifyType_Append;
    }
    if (size == 6 && memcmp("insert", c, 6) == 0) {
        return ListModifyType_Insert;
    }
    if (size == 6 && memcmp("remove", c, 6) == 0) {
        return ListModifyType_Remove;
    }
    if (size == 10 && memcmp("swapremove", c, 10) == 0) {
        return ListModifyType_SwapRemove;
    }
    if (size == 3 && memcmp("pop", c, 3) == 0) {
        return ListModifyType_Pop;
    }
    if (size == 5 && memcmp("empty", c, 5) == 0) {
        return ListModifyType_Empty;
    }
    return ListModifyType_Invalid;
}
static inline const char*
ListModifyTypeToString(const ListModifyType e)
{
    if (e == ListModifyType_Append) {
        return "Append";
    }
    if (e == ListModifyType_Insert) {
        return "Insert";
    }
    if (e == ListModifyType_Remove) {
        return "Remove";
    }
    if (e == ListModifyType_SwapRemove) {
        return "SwapRemove";
    }
    if (e == ListModifyType_Pop) {
        return "Pop";
    }
    if (e == ListModifyType_Empty) {
        return "Empty";
    }
    return "Invalid";
}