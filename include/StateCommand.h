
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum StateCommand
{
    StateCommand_Invalid = -1,
    StateCommand_Print
} StateCommand,
  *pStateCommand;

#define StateCommandCount 1
#define StateCommandLongestString 5

static const StateCommand StateCommandMembers[] = { StateCommand_Print };

static inline StateCommand
StateCommandFromIndex(size_t index)
{
    if (index >= StateCommandCount) {
        return StateCommand_Invalid;
    }
    return StateCommandMembers[index];
}
static inline StateCommand
StateCommandFromString(const void* c, const size_t size)
{
    if (size > StateCommandLongestString) {
        return StateCommand_Invalid;
    }
    if (size == 5 && memcmp("Print", c, 5) == 0) {
        return StateCommand_Print;
    }
    return StateCommand_Invalid;
}
static inline StateCommand
StateCommandFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > StateCommandLongestString) {
        return StateCommand_Invalid;
    }
    char c[StateCommandLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 5 && memcmp("print", c, 5) == 0) {
        return StateCommand_Print;
    }
    return StateCommand_Invalid;
}
static inline const char*
StateCommandToString(const StateCommand e)
{
    if (e == StateCommand_Print) {
        return "Print";
    }
    return "Invalid";
}