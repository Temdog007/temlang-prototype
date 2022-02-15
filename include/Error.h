#pragma once

#include "Atom.h"
#include "InstructionStarter.h"
#include "Token.h"
#include "TokenType.h"
#include "ValueType.h"

#define RED_TEXT "\033[0;31m"
#define GREEN_TEXT "\033[0;32m"
#define YELLOW_TEXT "\033[0;33m"
#define CYAN_TEXT "\033[0;36m"
#define RESET_TEXT "\033[0m"

#define STRING(color, s) color s RESET_TEXT

static inline void
InstructionStarterError(InstructionStarter starter)
{
    TemLangError("Failed to handle instruction: %s",
                 InstructionStarterToString(starter));
}

static inline void
UnexpectedTypeError(const InstructionSource* source,
                    const char* expected,
                    const char* actual)
{
    if (source == NULL) {
        TemLangError("Unexpected value. Expected %s; Got %s", expected, actual);
    } else {
        TemLangError("Unexpected value. Expected %s; Got %s (%s:%zu)",
                     expected,
                     actual,
                     source->source.buffer,
                     source->lineNumber);
    }
}

static inline void
UnexpectedTokenTypeError(const InstructionSource* source,
                         const TokenType expected,
                         const TokenType actual)
{
    UnexpectedTypeError(
      source, TokenTypeToString(expected), TokenTypeToString(actual));
}

static inline void
UnexpectedValueTypeError(const InstructionSource* source,
                         const ValueType expected,
                         const ValueType actual)
{
    UnexpectedTypeError(
      source, ValueTypeToString(expected), ValueTypeToString(actual));
}

static inline void
UnexpectedAtomTypeError(const InstructionSource* source,
                        const AtomType expected,
                        const AtomType actual)
{
    UnexpectedTypeError(
      source, AtomTypeToString(expected), AtomTypeToString(actual));
}

static inline void
UnexpectedEndError(const InstructionSource source)
{
    TemLangError("Unexpected end of tokens (%s:%zu)\n",
                 source.source.buffer,
                 source.lineNumber);
}

static inline void
MemberNotFoundError(const char* t,
                    const TemLangString* enumName,
                    const TemLangString* memberName)
{
    TemLangError("Member '%s' was not in %s '%s'",
                 memberName->buffer,
                 t,
                 enumName->buffer);
}