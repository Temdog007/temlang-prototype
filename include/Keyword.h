
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum Keyword
{
    Keyword_Invalid = -1,
    Keyword_Null,
    Keyword_True,
    Keyword_False,
    Keyword_external,
    Keyword_bool,
    Keyword_string,
    Keyword_i8,
    Keyword_i16,
    Keyword_i32,
    Keyword_i64,
    Keyword_u8,
    Keyword_u16,
    Keyword_u32,
    Keyword_u64,
    Keyword_f32,
    Keyword_f64
} Keyword,
  *pKeyword;

#define KeywordCount 16
#define KeywordLongestString 8

static const Keyword KeywordMembers[] = {
    Keyword_Null, Keyword_True,   Keyword_False, Keyword_external,
    Keyword_bool, Keyword_string, Keyword_i8,    Keyword_i16,
    Keyword_i32,  Keyword_i64,    Keyword_u8,    Keyword_u16,
    Keyword_u32,  Keyword_u64,    Keyword_f32,   Keyword_f64
};

static inline Keyword
KeywordFromIndex(size_t index)
{
    if (index >= KeywordCount) {
        return Keyword_Invalid;
    }
    return KeywordMembers[index];
}
static inline Keyword
KeywordFromString(const void* c, const size_t size)
{
    if (size > KeywordLongestString) {
        return Keyword_Invalid;
    }
    if (size == 4 && memcmp("Null", c, 4) == 0) {
        return Keyword_Null;
    }
    if (size == 4 && memcmp("True", c, 4) == 0) {
        return Keyword_True;
    }
    if (size == 5 && memcmp("False", c, 5) == 0) {
        return Keyword_False;
    }
    if (size == 8 && memcmp("external", c, 8) == 0) {
        return Keyword_external;
    }
    if (size == 4 && memcmp("bool", c, 4) == 0) {
        return Keyword_bool;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return Keyword_string;
    }
    if (size == 2 && memcmp("i8", c, 2) == 0) {
        return Keyword_i8;
    }
    if (size == 3 && memcmp("i16", c, 3) == 0) {
        return Keyword_i16;
    }
    if (size == 3 && memcmp("i32", c, 3) == 0) {
        return Keyword_i32;
    }
    if (size == 3 && memcmp("i64", c, 3) == 0) {
        return Keyword_i64;
    }
    if (size == 2 && memcmp("u8", c, 2) == 0) {
        return Keyword_u8;
    }
    if (size == 3 && memcmp("u16", c, 3) == 0) {
        return Keyword_u16;
    }
    if (size == 3 && memcmp("u32", c, 3) == 0) {
        return Keyword_u32;
    }
    if (size == 3 && memcmp("u64", c, 3) == 0) {
        return Keyword_u64;
    }
    if (size == 3 && memcmp("f32", c, 3) == 0) {
        return Keyword_f32;
    }
    if (size == 3 && memcmp("f64", c, 3) == 0) {
        return Keyword_f64;
    }
    return Keyword_Invalid;
}
static inline Keyword
KeywordFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > KeywordLongestString) {
        return Keyword_Invalid;
    }
    char c[KeywordLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 4 && memcmp("null", c, 4) == 0) {
        return Keyword_Null;
    }
    if (size == 4 && memcmp("true", c, 4) == 0) {
        return Keyword_True;
    }
    if (size == 5 && memcmp("false", c, 5) == 0) {
        return Keyword_False;
    }
    if (size == 8 && memcmp("external", c, 8) == 0) {
        return Keyword_external;
    }
    if (size == 4 && memcmp("bool", c, 4) == 0) {
        return Keyword_bool;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return Keyword_string;
    }
    if (size == 2 && memcmp("i8", c, 2) == 0) {
        return Keyword_i8;
    }
    if (size == 3 && memcmp("i16", c, 3) == 0) {
        return Keyword_i16;
    }
    if (size == 3 && memcmp("i32", c, 3) == 0) {
        return Keyword_i32;
    }
    if (size == 3 && memcmp("i64", c, 3) == 0) {
        return Keyword_i64;
    }
    if (size == 2 && memcmp("u8", c, 2) == 0) {
        return Keyword_u8;
    }
    if (size == 3 && memcmp("u16", c, 3) == 0) {
        return Keyword_u16;
    }
    if (size == 3 && memcmp("u32", c, 3) == 0) {
        return Keyword_u32;
    }
    if (size == 3 && memcmp("u64", c, 3) == 0) {
        return Keyword_u64;
    }
    if (size == 3 && memcmp("f32", c, 3) == 0) {
        return Keyword_f32;
    }
    if (size == 3 && memcmp("f64", c, 3) == 0) {
        return Keyword_f64;
    }
    return Keyword_Invalid;
}
static inline const char*
KeywordToString(const Keyword e)
{
    if (e == Keyword_Null) {
        return "Null";
    }
    if (e == Keyword_True) {
        return "True";
    }
    if (e == Keyword_False) {
        return "False";
    }
    if (e == Keyword_external) {
        return "external";
    }
    if (e == Keyword_bool) {
        return "bool";
    }
    if (e == Keyword_string) {
        return "string";
    }
    if (e == Keyword_i8) {
        return "i8";
    }
    if (e == Keyword_i16) {
        return "i16";
    }
    if (e == Keyword_i32) {
        return "i32";
    }
    if (e == Keyword_i64) {
        return "i64";
    }
    if (e == Keyword_u8) {
        return "u8";
    }
    if (e == Keyword_u16) {
        return "u16";
    }
    if (e == Keyword_u32) {
        return "u32";
    }
    if (e == Keyword_u64) {
        return "u64";
    }
    if (e == Keyword_f32) {
        return "f32";
    }
    if (e == Keyword_f64) {
        return "f64";
    }
    return "Invalid";
}