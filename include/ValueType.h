
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum ValueType
{
    ValueType_Invalid = -1,
    ValueType_Null,
    ValueType_Number,
    ValueType_Boolean,
    ValueType_Type,
    ValueType_String,
    ValueType_Flag,
    ValueType_Enum,
    ValueType_Struct,
    ValueType_Variant,
    ValueType_List,
    ValueType_External,
    ValueType_Data
} ValueType,
  *pValueType;

#define ValueTypeCount 12
#define ValueTypeLongestString 8

static const ValueType ValueTypeMembers[] = {
    ValueType_Null,    ValueType_Number, ValueType_Boolean,  ValueType_Type,
    ValueType_String,  ValueType_Flag,   ValueType_Enum,     ValueType_Struct,
    ValueType_Variant, ValueType_List,   ValueType_External, ValueType_Data
};

static inline ValueType
ValueTypeFromIndex(size_t index)
{
    if (index >= ValueTypeCount) {
        return ValueType_Invalid;
    }
    return ValueTypeMembers[index];
}
static inline ValueType
ValueTypeFromString(const void* c, const size_t size)
{
    if (size > ValueTypeLongestString) {
        return ValueType_Invalid;
    }
    if (size == 4 && memcmp("Null", c, 4) == 0) {
        return ValueType_Null;
    }
    if (size == 6 && memcmp("Number", c, 6) == 0) {
        return ValueType_Number;
    }
    if (size == 7 && memcmp("Boolean", c, 7) == 0) {
        return ValueType_Boolean;
    }
    if (size == 4 && memcmp("Type", c, 4) == 0) {
        return ValueType_Type;
    }
    if (size == 6 && memcmp("String", c, 6) == 0) {
        return ValueType_String;
    }
    if (size == 4 && memcmp("Flag", c, 4) == 0) {
        return ValueType_Flag;
    }
    if (size == 4 && memcmp("Enum", c, 4) == 0) {
        return ValueType_Enum;
    }
    if (size == 6 && memcmp("Struct", c, 6) == 0) {
        return ValueType_Struct;
    }
    if (size == 7 && memcmp("Variant", c, 7) == 0) {
        return ValueType_Variant;
    }
    if (size == 4 && memcmp("List", c, 4) == 0) {
        return ValueType_List;
    }
    if (size == 8 && memcmp("External", c, 8) == 0) {
        return ValueType_External;
    }
    if (size == 4 && memcmp("Data", c, 4) == 0) {
        return ValueType_Data;
    }
    return ValueType_Invalid;
}
static inline ValueType
ValueTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > ValueTypeLongestString) {
        return ValueType_Invalid;
    }
    char c[ValueTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 4 && memcmp("null", c, 4) == 0) {
        return ValueType_Null;
    }
    if (size == 6 && memcmp("number", c, 6) == 0) {
        return ValueType_Number;
    }
    if (size == 7 && memcmp("boolean", c, 7) == 0) {
        return ValueType_Boolean;
    }
    if (size == 4 && memcmp("type", c, 4) == 0) {
        return ValueType_Type;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return ValueType_String;
    }
    if (size == 4 && memcmp("flag", c, 4) == 0) {
        return ValueType_Flag;
    }
    if (size == 4 && memcmp("enum", c, 4) == 0) {
        return ValueType_Enum;
    }
    if (size == 6 && memcmp("struct", c, 6) == 0) {
        return ValueType_Struct;
    }
    if (size == 7 && memcmp("variant", c, 7) == 0) {
        return ValueType_Variant;
    }
    if (size == 4 && memcmp("list", c, 4) == 0) {
        return ValueType_List;
    }
    if (size == 8 && memcmp("external", c, 8) == 0) {
        return ValueType_External;
    }
    if (size == 4 && memcmp("data", c, 4) == 0) {
        return ValueType_Data;
    }
    return ValueType_Invalid;
}
static inline const char*
ValueTypeToString(const ValueType e)
{
    if (e == ValueType_Null) {
        return "Null";
    }
    if (e == ValueType_Number) {
        return "Number";
    }
    if (e == ValueType_Boolean) {
        return "Boolean";
    }
    if (e == ValueType_Type) {
        return "Type";
    }
    if (e == ValueType_String) {
        return "String";
    }
    if (e == ValueType_Flag) {
        return "Flag";
    }
    if (e == ValueType_Enum) {
        return "Enum";
    }
    if (e == ValueType_Struct) {
        return "Struct";
    }
    if (e == ValueType_Variant) {
        return "Variant";
    }
    if (e == ValueType_List) {
        return "List";
    }
    if (e == ValueType_External) {
        return "External";
    }
    if (e == ValueType_Data) {
        return "Data";
    }
    return "Invalid";
}