#pragma once

#include "Allocator.h"
#include "List.h"
#include "Number.h"
#include "Token.h"
#include "TokenType.h"

#include <ctype.h>
#include <stdlib.h>

static inline bool
isIdentifierChar(char c);

static inline bool
isNumber(const Allocator* allocator,
         const char* content,
         const size_t length,
         pNumber number);

static inline size_t
countNewLines(const char* content, const size_t length)
{
    size_t count = 0;
    for (size_t i = 0; i < length; ++i) {
        if (content[i] == '\n') {
            ++count;
        }
    }
    return count;
}

static inline bool
isMemberCall(const char* content, const size_t length)
{
    size_t dots = 0;
    for (size_t i = 0; i < length; ++i) {
        if (content[i] == '.') {
            ++dots;
        }
        if (isIdentifierChar(content[i])) {
            continue;
        }
        return false;
    }
    return dots > 0;
}

static inline TokenList
getMemberCallExpression(const char* content,
                        const size_t length,
                        const TokenSource source,
                        const Allocator* allocator)
{
    TokenList tokens = {
        .buffer = NULL, .size = 0, .used = 0, .allocator = allocator
    };

    size_t i = 0;
    while (i < length) {
        if (content[i] == '.') {
            Token token = { 0 };
            token.source = source;
            token.type = TokenType_GetOperator;
            token.getOperator = GetOperator_Member;
            TokenListAppend(&tokens, &token);
            ++i;
        }
        {
            Token token = { 0 };
            token.source = source;
            token.type =
              tokens.used == 0 ? TokenType_Identifier : TokenType_String;
            const size_t start = i;
            while (i < length && isalnum(content[i])) {
                ++i;
            }
            if (start != i) {
                if (isNumber(
                      allocator, content + start, i - start, &token.number)) {
                    token.type = TokenType_Number;
                } else {
                    token.string = content + start;
                    token.length = i - start;
                }
                TokenListAppend(&tokens, &token);
            }
        }
    }
    return tokens;
}

static inline size_t
continueUntilScope(const char* content,
                   const size_t size,
                   const size_t start,
                   const char enterC,
                   const char exitC)
{
    size_t scope = 0;
    size_t quotes = 0;
    for (size_t i = start; i < size; ++i) {
        switch (content[i]) {
            case '\"':
            case '\'':
                ++quotes;
                break;
            default:
                if (quotes % 2 != 0) {
                    continue;
                }
                if (content[i] == enterC) {
                    ++scope;
                } else if (content[i] == exitC) {
                    if (scope < 2) {
                        return i;
                    } else {
                        --scope;
                    }
                }
                break;
        }
    }
    return size;
}

static inline size_t
continueUntilString(const char* content,
                    const size_t size,
                    const size_t start,
                    const char* enterC,
                    const size_t enterSize,
                    const char* exitC,
                    const size_t exitSize)
{
    size_t scope = 0;
    for (size_t i = start; i < size; ++i) {
        if (memcmp(&content[i], enterC, enterSize) == 0) {
            ++scope;
        } else if (memcmp(&content[i], exitC, exitSize) == 0) {
            switch (scope) {
                case 0:
                    TemLangError("Compiler error! Found end characters '%s' "
                                 "without being in a scope",
                                 exitC);
                    break;
                case 1:
                    return i;
                default:
                    --scope;
                    break;
            }
        }
    }
    return start + size;
}

static inline size_t
continueUntilChar(const char* content,
                  const size_t size,
                  const size_t start,
                  const char target)
{
    for (size_t i = start; i < size; ++i) {
        if (content[i] == target) {
            return i;
        }
    }
    return size;
}

static inline size_t
continueUntilEndStringQuotes(const char* content,
                             const size_t size,
                             const size_t start)
{
    for (size_t i = start; i < size; ++i) {
        switch (content[i]) {
            case '\\':
                ++i;
                break;
            case '\"':
                return i;
            default:
                break;
        }
    }
    return size;
}

static inline size_t
continueWhile(const char* content,
              const size_t size,
              const size_t start,
              bool (*f)(char))
{
    for (size_t i = start; i < size; ++i) {
        if (f(content[i])) {
            continue;
        }
        return i;
    }
    return size;
}

static inline bool
isIdentifierChar(char c)
{
    return c == '_' || c == '.' || isalnum(c);
}

static inline bool
isNumberChar(char c)
{
    return c == '.' || c == ',' || c == '_' || isdigit(c);
}

static inline bool
isIdentifier(const char* content, const size_t length)
{
    for (size_t i = 0; i < length; ++i) {
        const char c = content[i];
        if (c == '_' || isalnum(c)) {
            continue;
        }
        return false;
    }
    return true;
}

static inline bool
isNumber(const Allocator* allocator,
         const char* content,
         const size_t length,
         pNumber number)
{
    size_t i = 0;
    TemLangString s = { 0 };
    s.allocator = allocator;
    bool hasDecimal = false;
    bool result = true;
    for (; i < length; ++i) {
        const char c = content[i];
        switch (c) {
            case ',':
            case '_':
                break;
            case '.':
                if (hasDecimal) {
                    result = false;
                    goto end;
                }
                TemLangStringAppendChar(&s, c);
                hasDecimal = true;
                break;
            default:
                if (isdigit(c)) {
                    TemLangStringAppendChar(&s, c);
                    break;
                }
                result = false;
                goto end;
        }
    }
    if (s.buffer == NULL) {
        result = false;
        goto end;
    }
    char* endPtr = NULL;
    if (hasDecimal) {
        number->d = strtod(s.buffer, &endPtr);
        number->type = NumberType_Float;
    } else {
        number->i = (uint64_t)strtoull(s.buffer, &endPtr, 10);
        number->type = NumberType_Unsigned;
    }
    result = s.buffer[s.used] == *endPtr;
end:
    TemLangStringFree(&s);
    return result;
}

static inline TokenList
performLex(const Allocator* allocator,
           const char* content,
           size_t size,
           const size_t currentLineNumber,
           const char* source)
{
    TokenList list = { 0 };
    list.buffer = NULL;
    list.used = 0;
    list.allocator = allocator;

    size_t i = 0;
    size_t lineNumber = currentLineNumber;
    while (i < size) {
        const char c = content[i];
        Token token = { 0 };
        token.source.source = source;
        token.source.lineNumber = lineNumber;
        if (isalpha(c)) {
            const size_t start = i;
            const size_t end =
              continueWhile(content, size, start + 1, isIdentifierChar);
            const char* string = content + start;
            const size_t length = end - start;

            token.starter =
              InstructionStarterFromCaseInsensitiveString(string, length);
            if (token.starter != InstructionStarter_Invalid) {
                token.type = TokenType_InstructionStarter;
                goto addToken;
            }

            token.booleanOperator =
              BooleanOperatorFromCaseInsensitiveString(string, length);
            if (token.booleanOperator != BooleanOperator_Invalid) {
                token.type = TokenType_BooleanOperator;
                goto addToken;
            }

            token.numberOperator =
              NumberOperatorFromCaseInsensitiveString(string, length);
            if (token.numberOperator != NumberOperator_Invalid) {
                token.type = TokenType_NumberOperator;
                goto addToken;
            }

            token.comparisonOperator =
              ComparisonOperatorFromCaseInsensitiveString(string, length);
            if (token.comparisonOperator != ComparisonOperator_Invalid) {
                token.type = TokenType_ComparisonOperator;
                goto addToken;
            }

            token.keyword = KeywordFromCaseInsensitiveString(string, length);
            if (token.keyword != Keyword_Invalid) {
                token.type = TokenType_Keyword;
                goto addToken;
            }

            if (isMemberCall(string, length)) {
                token.type = TokenType_Expression;
                token.tokens = getMemberCallExpression(
                  string, length, token.source, allocator);
                goto addToken;
            }

            if (isIdentifier(string, length)) {
                token.type = TokenType_Identifier;
                token.string = string;
                token.length = length;
                goto addToken;
            }
            char buffer[1024] = { 0 };
            memcpy(buffer, string, length);
            TokenError(token.source, "'%s' is not an identifier", buffer);
            break;
        addToken:
            i = end;
            TokenListAppend(&list, &token);
            lineNumber += countNewLines(string, length);
            continue;
        }
        if (c == '.' || isdigit(c)) {
            const size_t start = i;
            const size_t end =
              continueWhile(content, size, start + 1, isNumberChar);
            const char* string = content + start;
            const size_t length = end - start;
            if (isNumber(allocator, string, length, &token.number)) {
                token.type = TokenType_Number;
                i = end;
                TokenListAppend(&list, &token);
                lineNumber += countNewLines(string, length);
                continue;
            }
            char buffer[1024] = { 0 };
            memcpy(buffer, string, length);
            TokenError(token.source, "'%s' is not a number", buffer);
            break;
        }
        if (i < size - 1) {
            if (memcmp("//", &content[i], 2) == 0) {
                i = continueUntilChar(content, size, i, '\n');
                continue;
            }
            if (memcmp("/*", &content[i], 2) == 0) {
                const size_t end =
                  continueUntilString(content, size, i, "/*", 2, "*/", 2);
                lineNumber += countNewLines(content + i, end - i);
                i = end + 2;
                continue;
            }
            if (memcmp("{{", &content[i], 2) == 0) {
                const size_t end =
                  continueUntilString(content, size, i, "{{", 2, "}}", 2);
                if (end != size) {
                    const char* string = content + (i + 2);
                    const size_t length = end - (i + 2);
                    token.tokens =
                      performLex(allocator, string, length, lineNumber, source);
                    lineNumber += countNewLines(string, length);
                    token.type = TokenType_Struct;
                    TokenListAppend(&list, &token);
                    i = end + 2;
                    TokenFree(&token);
                    continue;
                }
            }
            if (memcmp("[|", &content[i], 2) == 0) {
                const size_t end =
                  continueUntilString(content, size, i, "[|", 2, "|]", 2);
                const char* string = content + i + 2;
                const size_t length = end - (i + 2);
                token.tokens =
                  performLex(allocator, string, length, lineNumber, source);
                lineNumber += countNewLines(string, length);
                token.type = TokenType_Array;
                TokenListAppend(&list, &token);
                i = end + 2;
                TokenFree(&token);
                continue;
            }
            if (memcmp("{=", &content[i], 2) == 0) {
                const size_t end =
                  continueUntilString(content, size, i, "{=", 2, "=}", 2);
                const char* string = content + i + 2;
                const size_t length = end - (i + 2);
                token.tokens =
                  performLex(allocator, string, length, lineNumber, source);
                lineNumber += countNewLines(string, length);
                token.type = TokenType_Match;
                TokenListAppend(&list, &token);
                i = end + 2;
                TokenFree(&token);
                continue;
            }
            if (memcmp("[]", &content[i], 2) == 0) {
                const size_t start = content[i + 2] == '#' ? i + 3 : i + 2;
                const size_t end =
                  continueWhile(content, size, start, isIdentifierChar);
                const char* string = content + start;
                const size_t length = end - start;
                Token subToken = {
                    .type = TokenType_List,
                    .tokens =
                      performLex(allocator, string, length, lineNumber, source)
                };
                if (TempStructMemberFromToken(&subToken, &token.structMember)) {
                    token.type = TokenType_ListInitialization;
                    TokenListAppend(&list, &token);
                    goto listInitNextLine;
                }

                TokenError(token.source, "Failed to parse list initialization");

            listInitNextLine:
                TokenFree(&subToken);
                lineNumber += countNewLines(string, length);
                i = end;
                continue;
            }
            if (memcmp("##", &content[i], 2) == 0) {
                token.type = TokenType_GetOperator;
                token.getOperator = GetOperator_Length;
                TokenListAppend(&list, &token);
                i += 2;
                continue;
            }
        }
        switch (c) {
            case '\n':
                ++lineNumber;
                break;
            case '+':
                token.type = TokenType_NumberOperator;
                token.numberOperator = NumberOperator_Add;
                TokenListAppend(&list, &token);
                break;
            case '-':
                token.type = TokenType_NumberOperator;
                token.numberOperator = NumberOperator_Subtract;
                TokenListAppend(&list, &token);
                break;
            case '*':
                token.type = TokenType_NumberOperator;
                token.numberOperator = NumberOperator_Multiply;
                TokenListAppend(&list, &token);
                break;
            case '/':
                token.type = TokenType_NumberOperator;
                token.numberOperator = NumberOperator_Divide;
                TokenListAppend(&list, &token);
                break;
            case '%':
                token.type = TokenType_NumberOperator;
                token.numberOperator = NumberOperator_Modulo;
                TokenListAppend(&list, &token);
                break;
            case '<':
                token.type = TokenType_ComparisonOperator;
                token.comparisonOperator = ComparisonOperator_LessThan;
                TokenListAppend(&list, &token);
                break;
            case '>':
                token.type = TokenType_ComparisonOperator;
                token.comparisonOperator = ComparisonOperator_GreaterThan;
                TokenListAppend(&list, &token);
                break;
            case '=':
                token.type = TokenType_ComparisonOperator;
                token.comparisonOperator = ComparisonOperator_EqualTo;
                TokenListAppend(&list, &token);
                break;
            case '|':
                token.type = TokenType_BooleanOperator;
                token.booleanOperator = BooleanOperator_Or;
                TokenListAppend(&list, &token);
                break;
            case '&':
                token.type = TokenType_BooleanOperator;
                token.booleanOperator = BooleanOperator_And;
                TokenListAppend(&list, &token);
                break;
            case '^':
                token.type = TokenType_BooleanOperator;
                token.booleanOperator = BooleanOperator_Xor;
                TokenListAppend(&list, &token);
                break;
            case '!':
                token.type = TokenType_BooleanOperator;
                token.booleanOperator = BooleanOperator_Not;
                TokenListAppend(&list, &token);
                break;
            case '@':
                token.type = TokenType_GetOperator;
                token.getOperator = GetOperator_Member;
                TokenListAppend(&list, &token);
                break;
            case '#': {
                const size_t start = i + 1;
                const size_t end =
                  continueWhile(content, size, start, isIdentifierChar);
                const size_t length = end - start;
                if (length == 0) {
                    token.type = TokenType_GetOperator;
                    token.getOperator = GetOperator_Type;
                } else {
                    token.type = TokenType_Type;
                    token.string = content + start;
                    token.length = length;
                }
                TokenListAppend(&list, &token);
                i = end;
                lineNumber += countNewLines(content + start, length);
                continue;
            } break;
            case '[': {
                const size_t start = i;
                const size_t end =
                  continueUntilScope(content, size, start, '[', ']');
                const char* string = content + start + 1;
                const size_t length = end - (start + 1);
                token.tokens =
                  performLex(allocator, string, length, lineNumber, source);
                lineNumber += countNewLines(string, length);
                token.type = TokenType_List;
                TokenListAppend(&list, &token);
                i = end + 1;
                TokenFree(&token);
                continue;
            } break;
            case '{': {
                const size_t start = i;
                const size_t end =
                  continueUntilScope(content, size, start, '{', '}');
                const char* string = content + start + 1;
                const size_t length = end - (start + 1);
                token.tokens =
                  performLex(allocator, string, length, lineNumber, source);
                lineNumber += countNewLines(string, length);
                token.type = TokenType_Scope;
                TokenListAppend(&list, &token);
                i = end + 1;
                TokenFree(&token);
                continue;
            } break;
            case '(': {
                const size_t start = i;
                const size_t end =
                  continueUntilScope(content, size, start, '(', ')');
                const char* string = content + start + 1;
                const size_t length = end - (start + 1);
                token.tokens =
                  performLex(allocator, string, length, lineNumber, source);
                lineNumber += countNewLines(string, length);
                token.type = TokenType_Expression;
                TokenListAppend(&list, &token);
                i = end + 1;
                TokenFree(&token);
                continue;
            } break;
            case ':': {
                const size_t start = i + 1;
                const size_t end =
                  continueWhile(content, size, start, isIdentifierChar);
                const char* string = content + start;
                const size_t length = end - start;
                TokenList newList =
                  performLex(allocator, string, length, lineNumber, source);
                if (newList.used == 1 &&
                    newList.buffer[0].type == TokenType_Identifier) {
                    token.type = TokenType_FunctionCall;
                    token.string = newList.buffer[0].string;
                    token.length = newList.buffer[0].length;
                    TokenListAppend(&list, &token);
                } else {
                    TokenError(token.source, "Failed to parse function call");
                }
                TokenListFree(&newList);
                lineNumber += countNewLines(string, length);
                i = end;
                continue;
            } break;
            case '\"': {
                const size_t start = i + 1;
                const size_t end =
                  continueUntilEndStringQuotes(content, size, start);
                token.type = TokenType_String;
                token.string = content + start;
                token.length = end - start;
                lineNumber += countNewLines(token.string, token.length);
                i = end + 1;
                TokenListAppend(&list, &token);
                continue;
            } break;
            case '\'': {
                if (i + 2 >= size) {
                    TokenError(token.source,
                               "Unexpected character end of characters");
                    break;
                }
                if (content[i + 2] != '\'') {
                    TokenError(token.source,
                               "Unexpected character '%c'; Expected ending "
                               "\' character",
                               content[i + 2]);
                    break;
                }
                token.type = TokenType_Character;
                token.c = content[i + 1];
                TokenListAppend(&list, &token);
                i = i + 3;
                continue;
            } break;
            case ',':
            case ';':
                break;
            default:
                if (!isspace(c)) {
                    TokenError(token.source, "Unexpected character '%c'", c);
                }
                break;
        }
        ++i;
    }

    return list;
}