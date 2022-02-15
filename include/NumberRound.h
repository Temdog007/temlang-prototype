
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum NumberRound
{
    NumberRound_Invalid = -1,
    NumberRound_Floor,
    NumberRound_Round,
    NumberRound_Ceil
} NumberRound,
  *pNumberRound;

#define NumberRoundCount 3
#define NumberRoundLongestString 5

static const NumberRound NumberRoundMembers[] = { NumberRound_Floor,
                                                  NumberRound_Round,
                                                  NumberRound_Ceil };

static inline NumberRound
NumberRoundFromIndex(size_t index)
{
    if (index >= NumberRoundCount) {
        return NumberRound_Invalid;
    }
    return NumberRoundMembers[index];
}
static inline NumberRound
NumberRoundFromString(const void* c, const size_t size)
{
    if (size > NumberRoundLongestString) {
        return NumberRound_Invalid;
    }
    if (size == 5 && memcmp("Floor", c, 5) == 0) {
        return NumberRound_Floor;
    }
    if (size == 5 && memcmp("Round", c, 5) == 0) {
        return NumberRound_Round;
    }
    if (size == 4 && memcmp("Ceil", c, 4) == 0) {
        return NumberRound_Ceil;
    }
    return NumberRound_Invalid;
}
static inline NumberRound
NumberRoundFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > NumberRoundLongestString) {
        return NumberRound_Invalid;
    }
    char c[NumberRoundLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 5 && memcmp("floor", c, 5) == 0) {
        return NumberRound_Floor;
    }
    if (size == 5 && memcmp("round", c, 5) == 0) {
        return NumberRound_Round;
    }
    if (size == 4 && memcmp("ceil", c, 4) == 0) {
        return NumberRound_Ceil;
    }
    return NumberRound_Invalid;
}
static inline const char*
NumberRoundToString(const NumberRound e)
{
    if (e == NumberRound_Floor) {
        return "Floor";
    }
    if (e == NumberRound_Round) {
        return "Round";
    }
    if (e == NumberRound_Ceil) {
        return "Ceil";
    }
    return "Invalid";
}