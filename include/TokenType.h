
#pragma once
#include <ctype.h>
#include <memory.h>
#include <stddef.h>

typedef enum TokenType
{
    TokenType_Invalid = -1,
    TokenType_Keyword,
    TokenType_InstructionStarter,
    TokenType_GetOperator,
    TokenType_NumberOperator,
    TokenType_BooleanOperator,
    TokenType_ComparisonOperator,
    TokenType_String,
    TokenType_Character,
    TokenType_Number,
    TokenType_Type,
    TokenType_Identifier,
    TokenType_FunctionCall,
    TokenType_ListInitialization,
    TokenType_Match,
    TokenType_Iterate,
    TokenType_NumberRound,
    TokenType_Expression,
    TokenType_List,
    TokenType_Array,
    TokenType_Scope,
    TokenType_Struct
} TokenType,
  *pTokenType;

#define TokenTypeCount 21
#define TokenTypeLongestString 18

static const TokenType TokenTypeMembers[] = { TokenType_Keyword,
                                              TokenType_InstructionStarter,
                                              TokenType_GetOperator,
                                              TokenType_NumberOperator,
                                              TokenType_BooleanOperator,
                                              TokenType_ComparisonOperator,
                                              TokenType_String,
                                              TokenType_Character,
                                              TokenType_Number,
                                              TokenType_Type,
                                              TokenType_Identifier,
                                              TokenType_FunctionCall,
                                              TokenType_ListInitialization,
                                              TokenType_Match,
                                              TokenType_Iterate,
                                              TokenType_NumberRound,
                                              TokenType_Expression,
                                              TokenType_List,
                                              TokenType_Array,
                                              TokenType_Scope,
                                              TokenType_Struct };

static inline TokenType
TokenTypeFromIndex(size_t index)
{
    if (index >= TokenTypeCount) {
        return TokenType_Invalid;
    }
    return TokenTypeMembers[index];
}
static inline TokenType
TokenTypeFromString(const void* c, const size_t size)
{
    if (size > TokenTypeLongestString) {
        return TokenType_Invalid;
    }
    if (size == 7 && memcmp("Keyword", c, 7) == 0) {
        return TokenType_Keyword;
    }
    if (size == 18 && memcmp("InstructionStarter", c, 18) == 0) {
        return TokenType_InstructionStarter;
    }
    if (size == 11 && memcmp("GetOperator", c, 11) == 0) {
        return TokenType_GetOperator;
    }
    if (size == 14 && memcmp("NumberOperator", c, 14) == 0) {
        return TokenType_NumberOperator;
    }
    if (size == 15 && memcmp("BooleanOperator", c, 15) == 0) {
        return TokenType_BooleanOperator;
    }
    if (size == 18 && memcmp("ComparisonOperator", c, 18) == 0) {
        return TokenType_ComparisonOperator;
    }
    if (size == 6 && memcmp("String", c, 6) == 0) {
        return TokenType_String;
    }
    if (size == 9 && memcmp("Character", c, 9) == 0) {
        return TokenType_Character;
    }
    if (size == 6 && memcmp("Number", c, 6) == 0) {
        return TokenType_Number;
    }
    if (size == 4 && memcmp("Type", c, 4) == 0) {
        return TokenType_Type;
    }
    if (size == 10 && memcmp("Identifier", c, 10) == 0) {
        return TokenType_Identifier;
    }
    if (size == 12 && memcmp("FunctionCall", c, 12) == 0) {
        return TokenType_FunctionCall;
    }
    if (size == 18 && memcmp("ListInitialization", c, 18) == 0) {
        return TokenType_ListInitialization;
    }
    if (size == 5 && memcmp("Match", c, 5) == 0) {
        return TokenType_Match;
    }
    if (size == 7 && memcmp("Iterate", c, 7) == 0) {
        return TokenType_Iterate;
    }
    if (size == 11 && memcmp("NumberRound", c, 11) == 0) {
        return TokenType_NumberRound;
    }
    if (size == 10 && memcmp("Expression", c, 10) == 0) {
        return TokenType_Expression;
    }
    if (size == 4 && memcmp("List", c, 4) == 0) {
        return TokenType_List;
    }
    if (size == 5 && memcmp("Array", c, 5) == 0) {
        return TokenType_Array;
    }
    if (size == 5 && memcmp("Scope", c, 5) == 0) {
        return TokenType_Scope;
    }
    if (size == 6 && memcmp("Struct", c, 6) == 0) {
        return TokenType_Struct;
    }
    return TokenType_Invalid;
}
static inline TokenType
TokenTypeFromCaseInsensitiveString(const char* original, const size_t size)
{
    if (size > TokenTypeLongestString) {
        return TokenType_Invalid;
    }
    char c[TokenTypeLongestString] = { 0 };
    for (size_t i = 0; i < size; ++i) {
        c[i] = tolower(original[i]);
    }
    if (size == 7 && memcmp("keyword", c, 7) == 0) {
        return TokenType_Keyword;
    }
    if (size == 18 && memcmp("instructionstarter", c, 18) == 0) {
        return TokenType_InstructionStarter;
    }
    if (size == 11 && memcmp("getoperator", c, 11) == 0) {
        return TokenType_GetOperator;
    }
    if (size == 14 && memcmp("numberoperator", c, 14) == 0) {
        return TokenType_NumberOperator;
    }
    if (size == 15 && memcmp("booleanoperator", c, 15) == 0) {
        return TokenType_BooleanOperator;
    }
    if (size == 18 && memcmp("comparisonoperator", c, 18) == 0) {
        return TokenType_ComparisonOperator;
    }
    if (size == 6 && memcmp("string", c, 6) == 0) {
        return TokenType_String;
    }
    if (size == 9 && memcmp("character", c, 9) == 0) {
        return TokenType_Character;
    }
    if (size == 6 && memcmp("number", c, 6) == 0) {
        return TokenType_Number;
    }
    if (size == 4 && memcmp("type", c, 4) == 0) {
        return TokenType_Type;
    }
    if (size == 10 && memcmp("identifier", c, 10) == 0) {
        return TokenType_Identifier;
    }
    if (size == 12 && memcmp("functioncall", c, 12) == 0) {
        return TokenType_FunctionCall;
    }
    if (size == 18 && memcmp("listinitialization", c, 18) == 0) {
        return TokenType_ListInitialization;
    }
    if (size == 5 && memcmp("match", c, 5) == 0) {
        return TokenType_Match;
    }
    if (size == 7 && memcmp("iterate", c, 7) == 0) {
        return TokenType_Iterate;
    }
    if (size == 11 && memcmp("numberround", c, 11) == 0) {
        return TokenType_NumberRound;
    }
    if (size == 10 && memcmp("expression", c, 10) == 0) {
        return TokenType_Expression;
    }
    if (size == 4 && memcmp("list", c, 4) == 0) {
        return TokenType_List;
    }
    if (size == 5 && memcmp("array", c, 5) == 0) {
        return TokenType_Array;
    }
    if (size == 5 && memcmp("scope", c, 5) == 0) {
        return TokenType_Scope;
    }
    if (size == 6 && memcmp("struct", c, 6) == 0) {
        return TokenType_Struct;
    }
    return TokenType_Invalid;
}
static inline const char*
TokenTypeToString(const TokenType e)
{
    if (e == TokenType_Keyword) {
        return "Keyword";
    }
    if (e == TokenType_InstructionStarter) {
        return "InstructionStarter";
    }
    if (e == TokenType_GetOperator) {
        return "GetOperator";
    }
    if (e == TokenType_NumberOperator) {
        return "NumberOperator";
    }
    if (e == TokenType_BooleanOperator) {
        return "BooleanOperator";
    }
    if (e == TokenType_ComparisonOperator) {
        return "ComparisonOperator";
    }
    if (e == TokenType_String) {
        return "String";
    }
    if (e == TokenType_Character) {
        return "Character";
    }
    if (e == TokenType_Number) {
        return "Number";
    }
    if (e == TokenType_Type) {
        return "Type";
    }
    if (e == TokenType_Identifier) {
        return "Identifier";
    }
    if (e == TokenType_FunctionCall) {
        return "FunctionCall";
    }
    if (e == TokenType_ListInitialization) {
        return "ListInitialization";
    }
    if (e == TokenType_Match) {
        return "Match";
    }
    if (e == TokenType_Iterate) {
        return "Iterate";
    }
    if (e == TokenType_NumberRound) {
        return "NumberRound";
    }
    if (e == TokenType_Expression) {
        return "Expression";
    }
    if (e == TokenType_List) {
        return "List";
    }
    if (e == TokenType_Array) {
        return "Array";
    }
    if (e == TokenType_Scope) {
        return "Scope";
    }
    if (e == TokenType_Struct) {
        return "Struct";
    }
    return "Invalid";
}