
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum CType
{
    CType_Invalid = -1,
    CType_u8,
    CType_u16,
    CType_u32,
    CType_u64,
    CType_i8,
    CType_i16,
    CType_i32,
    CType_i64,
    CType_f32,
    CType_f64,
    CType_bool,
    CType_string,
    CType_ptr
} CType,
  *pCType;

#define CTypeCount 13
#define CTypeLongestString 6

static const CType CTypeMembers[] = { CType_u8,  CType_u16,  CType_u32,
                                      CType_u64, CType_i8,   CType_i16,
                                      CType_i32, CType_i64,  CType_f32,
                                      CType_f64, CType_bool, CType_string,
                                      CType_ptr };

static inline CType
CTypeFromIndex(size_t index)
{
    if (index >= CTypeCount) {
        return CType_Invalid;
    }
    return CTypeMembers[index];
}
static inline CType
CTypeFromString(const void* c, const size_t size)
{
    if (size > CTypeLongestString) {
        return CType_Invalid;
    }
    if (size == 2 && memcmp("u8", c, 2) == 0) {
        return CType_u8;
    }
    if (size == 3 && memcmp("u16", c, 3) == 0) {
        return CType_u16;
    }
    if (size == 3 && memcmp("u32", c, 3) == 0) {
        return CType_u32;
    }
    if (size == 3 && memcmp("u64", c, 3) == 0) {
        return CType_u64;
    }
    if (size == 2 && memcmp("i8", c, 2) == 0) {
        return CType_i8;
    }
    if (size == 3 && memcmp("i16", c, 3) == 0) {
        return CType_i16;
    }
    if (size == 3 && memcmp("i32", c, 3) == 0) {
        return CType_i32;
    }
    if (size == 3 && memcmp("i64", c, 3) == 0) {
        return CType_i64;
    }
    if (size == 3 && memcmp("f32", c, 3) == 0) {
        return CType_f32;
    }
    if (size == 3 && memcmp("f64", c, 3) == 0) {
        return CType_f64;
    }
    if (size == 4 && memcmp("bool", c, 4) == 0) {
        return CType_bool;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return CType_string;
    }
    if (size == 3 && memcmp("ptr", c, 3) == 0) {
        return CType_ptr;
    }
    return CType_Invalid;
}
static inline CType
CTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > CTypeLongestString) {
        return CType_Invalid;
    }
    char c[CTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 2 && memcmp("u8", c, 2) == 0) {
        return CType_u8;
    }
    if (size == 3 && memcmp("u16", c, 3) == 0) {
        return CType_u16;
    }
    if (size == 3 && memcmp("u32", c, 3) == 0) {
        return CType_u32;
    }
    if (size == 3 && memcmp("u64", c, 3) == 0) {
        return CType_u64;
    }
    if (size == 2 && memcmp("i8", c, 2) == 0) {
        return CType_i8;
    }
    if (size == 3 && memcmp("i16", c, 3) == 0) {
        return CType_i16;
    }
    if (size == 3 && memcmp("i32", c, 3) == 0) {
        return CType_i32;
    }
    if (size == 3 && memcmp("i64", c, 3) == 0) {
        return CType_i64;
    }
    if (size == 3 && memcmp("f32", c, 3) == 0) {
        return CType_f32;
    }
    if (size == 3 && memcmp("f64", c, 3) == 0) {
        return CType_f64;
    }
    if (size == 4 && memcmp("bool", c, 4) == 0) {
        return CType_bool;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return CType_string;
    }
    if (size == 3 && memcmp("ptr", c, 3) == 0) {
        return CType_ptr;
    }
    return CType_Invalid;
}
static inline const char*
CTypeToString(const CType e)
{
    if (e == CType_u8) {
        return "u8";
    }
    if (e == CType_u16) {
        return "u16";
    }
    if (e == CType_u32) {
        return "u32";
    }
    if (e == CType_u64) {
        return "u64";
    }
    if (e == CType_i8) {
        return "i8";
    }
    if (e == CType_i16) {
        return "i16";
    }
    if (e == CType_i32) {
        return "i32";
    }
    if (e == CType_i64) {
        return "i64";
    }
    if (e == CType_f32) {
        return "f32";
    }
    if (e == CType_f64) {
        return "f64";
    }
    if (e == CType_bool) {
        return "bool";
    }
    if (e == CType_string) {
        return "string";
    }
    if (e == CType_ptr) {
        return "ptr";
    }
    return "Invalid";
}