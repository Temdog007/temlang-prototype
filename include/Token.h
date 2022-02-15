#pragma once

#include <stdarg.h>

#include "Allocator.h"
#include "BooleanOperator.h"
#include "ComparisonOperator.h"
#include "GetOperator.h"
#include "InstructionStarter.h"
#include "Keyword.h"
#include "Number.h"
#include "NumberOperator.h"
#include "NumberRound.h"
#include "StructMember.h"
#include "TemLangString.h"
#include "TokenType.h"

typedef struct Token Token, *pToken;

typedef struct TokenSource
{
    const char* source;
    size_t lineNumber;
} TokenSource, *pTokenSource;

typedef struct InstructionSource
{
    TemLangString source;
    size_t lineNumber;
} InstructionSource, *pInstructionSource;

static inline InstructionSource
TokenSourceToInstructionSource(TokenSource source, const Allocator* allocator)
{
    InstructionSource s = { 0 };
    s.lineNumber = source.lineNumber;
    s.source = TemLangStringCreate(source.source, allocator);
    return s;
}

static inline void
InstructionSourceFree(InstructionSource* i)
{
    TemLangStringFree(&i->source);
    memset(i, 0, sizeof(InstructionSource));
}

static inline bool
InstructionSourceCopy(InstructionSource* dest,
                      const InstructionSource* src,
                      const Allocator* allocator)
{
    InstructionSourceFree(dest);
    dest->lineNumber = src->lineNumber;
    return TemLangStringCopy(&dest->source, &src->source, allocator);
}

static inline TemLangString
InstructionSourceToString(const InstructionSource* i,
                          const Allocator* allocator)
{
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"source\": \"%s\", \"lineNumber\": %zu }",
                              i->source.buffer,
                              i->lineNumber);
    return s;
}

static inline void
TokenError(TokenSource source, const char* format, ...)
{
    char buffer[1024] = { 0 };
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    TemLangError(
      "Token error: %s(%s:%zu)", buffer, source.source, source.lineNumber);
}

typedef struct Token Token, *pToken;
MAKE_LIST(Token)

typedef struct Token
{
    TokenType type;
    TokenSource source;
    union
    {
        char c;
        InstructionStarter starter;
        Keyword keyword;
        ComparisonOperator comparisonOperator;
        NumberOperator numberOperator;
        BooleanOperator booleanOperator;
        GetOperator getOperator;
        TempStructMember structMember;
        struct
        {
            const char* string;
            size_t length;
        };
        Number number;
        TokenList tokens;
    };
} Token, *pToken;

static inline bool
TokenHasList(const Token* token)
{
    switch (token->type) {
        case TokenType_List:
        case TokenType_Array:
        case TokenType_Expression:
        case TokenType_Scope:
        case TokenType_Struct:
        case TokenType_Match:
            return true;
        default:
            return false;
    }
}

static inline bool
TokenHasString(const Token* token)
{
    switch (token->type) {
        case TokenType_Identifier:
        case TokenType_FunctionCall:
        case TokenType_String:
        case TokenType_Type:
            return true;
        default:
            return false;
    }
}

static inline size_t
nextInstructionStarter(const Token* tokens,
                       const size_t start,
                       const size_t end)
{
    for (size_t i = start; i < end; ++i) {
        if (tokens[i].type == TokenType_InstructionStarter) {
            return i;
        }
    }
    return end;
}

static inline void
TokenFree(Token* token)
{
    switch (token->type) {
        default:
            if (TokenHasList(token)) {
                TokenListFree(&token->tokens);
            }
            break;
    }
    memset(token, 0, sizeof(Token));
}

static inline bool
TokenCopy(Token* dest, const Token* src, const Allocator* allocator)
{
    TokenFree(dest);
    dest->type = src->type;
    dest->source = src->source;
    switch (src->type) {
        case TokenType_Character:
            dest->c = src->c;
            return true;
        case TokenType_ListInitialization:
            dest->structMember = src->structMember;
            return true;
        case TokenType_InstructionStarter:
            dest->starter = src->starter;
            break;
        case TokenType_Keyword:
            dest->keyword = src->keyword;
            break;
        case TokenType_ComparisonOperator:
            dest->comparisonOperator = src->comparisonOperator;
            break;
        case TokenType_NumberOperator:
            dest->numberOperator = src->numberOperator;
            break;
        case TokenType_BooleanOperator:
            dest->booleanOperator = src->booleanOperator;
            break;
        case TokenType_GetOperator:
            dest->getOperator = src->getOperator;
            break;
        case TokenType_Identifier:
        case TokenType_FunctionCall:
        case TokenType_String:
        case TokenType_Type:
            dest->string = src->string;
            dest->length = src->length;
            break;
        case TokenType_Number:
            dest->number = src->number;
            break;
        default:
            if (TokenHasList(src)) {
                dest->tokens.allocator = allocator;
                return TokenListCopy(&dest->tokens, &src->tokens, allocator);
            }
            copyFailure(TokenTypeToString(src->type));
            return false;
    }
    return true;
}

static inline TemLangString
TokenToString(const Token* token, const Allocator* a)
{
    TemLangStringCreateFormat(s,
                              a,
                              "{ \"type\": \"%s\", \"source\": \"%s\", "
                              "\"lineNumber\": %zu, \"value\": ",
                              TokenTypeToString(token->type),
                              token->source.source,
                              token->source.lineNumber);

    switch (token->type) {
        case TokenType_Character:
            TemLangStringAppendFormat(s, "%c }", token->c);
            break;
        case TokenType_ListInitialization: {
            StructMember m = ToRealStructMember(&token->structMember, a);
            TemLangString b = StructMemberToString(&m, a);
            StructMemberFree(&m);
            TemLangStringAppendFormat(s, "%s }", b.buffer);
            TemLangStringFree(&b);
        } break;
        case TokenType_BooleanOperator: {
            TemLangStringAppendFormat(
              s, "\"%s\" }", BooleanOperatorToString(token->booleanOperator));
        } break;
        case TokenType_NumberOperator: {
            TemLangStringAppendFormat(
              s, "\"%s\" }", NumberOperatorToString(token->numberOperator));
        } break;
        case TokenType_InstructionStarter: {
            TemLangStringAppendFormat(
              s, "\"%s\" }", InstructionStarterToString(token->starter));
        } break;
        case TokenType_ComparisonOperator: {
            TemLangStringAppendFormat(
              s,
              "\"%s\" }",
              ComparisonOperatorToString(token->comparisonOperator));
        } break;
        case TokenType_GetOperator: {
            TemLangStringAppendFormat(
              s, "\"%s\" }", GetOperatorToString(token->getOperator));
        } break;
        case TokenType_Keyword: {
            TemLangStringAppendFormat(
              s, "\"%s\"}", KeywordToString(token->keyword));
        } break;
        case TokenType_Number: {
            TemLangString newS = NumberToString(&token->number, s.allocator);
            TemLangStringAppendChars(&newS, " }");
            TemLangStringAppend(&s, &newS);
            TemLangStringFree(&newS);
        } break;
        case TokenType_Identifier:
        case TokenType_FunctionCall:
        case TokenType_String:
        case TokenType_Type: {
            TemLangString n = TemLangStringCreateFromSize(
              token->string, token->length + 1, s.allocator);
            TemLangStringAppendFormat(s, "\"%s\" }", n.buffer);
            TemLangStringFree(&n);
        } break;
        default:
            if (TokenHasList(token)) {
                LIST_TO_STRING(token->tokens, n, TokenToString, s.allocator);
                TemLangStringAppend(&s, &n);
                TemLangStringFree(&n);
                TemLangStringAppendChars(&s, " }");
                break;
            }
            TemLangStringAppendChars(&s, "null }");
            break;
    }

    return s;
}

#define CHECK_TOKEN(token, desiredTokenType, source, f)                        \
    if (token.type != desiredTokenType) {                                      \
        UnexpectedTokenTypeError(source, desiredTokenType, token.type);        \
        f                                                                      \
    }

#define CHECK_TOKEN_LIST(token, source, f)                                     \
    if (!TokenHasList(&token)) {                                               \
        UnexpectedTokenTypeError(source, TokenType_List, token.type);          \
        f                                                                      \
    }

DEFAULT_MAKE_LIST_FUNCTIONS(Token)