#pragma once

#include "Allocator.h"
#include "CType.h"
#include "StructMember.h"
#include "TemLangString.h"
#include "Token.h"

static inline bool
TempStructMemberFromToken(const Token* token, TempStructMember* m)
{
    switch (token->type) {
        case TokenType_Keyword:
            m->quantity = 1UL;
            m->isKeyword = true;
            m->keyword = token->keyword;
            return true;
        case TokenType_List:
        case TokenType_Array: {
            const TokenList* tokens = &token->tokens;
            switch (tokens->used) {
                case 1:
                    return TempStructMemberFromToken(&tokens->buffer[0], m);
                case 2:
                    break;
                default:
                    TemLangError("Expected 2 tokens to be converted to struct "
                                 "member. Got %zu",
                                 tokens->used);
                    return false;
            }
            const Token* numberToken = NULL;
            const Token* typeToken = NULL;
            if (tokens->buffer[0].type == TokenType_Number) {
                numberToken = &tokens->buffer[0];
                typeToken = &tokens->buffer[1];
            } else if (tokens->buffer[1].type == TokenType_Number) {
                typeToken = &tokens->buffer[0];
                numberToken = &tokens->buffer[1];
            } else {
                break;
            }
            if (numberToken->number.type == NumberType_Unsigned &&
                TempStructMemberFromToken(typeToken, m)) {
                if (m->quantity != 1) {
                    TemLangError("Structs cannot have lists of lists");
                    return false;
                }
                m->quantity = numberToken->number.u;
                return true;
            }
        } break;
        default:
            if (TokenHasString(token)) {
                m->quantity = 1UL;
                m->isKeyword = false;
                m->typeName = token->string;
                m->typeNameLength = token->length;
                return true;
            }
            break;
    }
    TemLangError("Token type '%s' cannot be converted to a struct member",
                 TokenTypeToString(token->type));
    return false;
}

typedef struct StructDefinition
{
    StructMemberList members;
    bool isVariant;
    TemLangString destructorTargetName;
    InstructionList deleteInstructions;
} StructDefinition, *pStructDefinition;

static inline void
StructDefinitionFree(StructDefinition* d)
{
    StructMemberListFree(&d->members);
    TemLangStringFree(&d->destructorTargetName);
    InstructionListFree(&d->deleteInstructions);
}

static inline bool
StructDefinitionCopy(StructDefinition* dest,
                     const StructDefinition* src,
                     const Allocator* allocator)
{
    StructDefinitionFree(dest);
    dest->isVariant = src->isVariant;
    return StructMemberListCopy(&dest->members, &src->members, allocator) &&
           TemLangStringCopy(&dest->destructorTargetName,
                             &src->destructorTargetName,
                             allocator) &&
           InstructionListCopy(
             &dest->deleteInstructions, &src->deleteInstructions, allocator);
}

static inline bool
InstructionListIsEmpty(const InstructionList*);

static inline TemLangString
StructDefinitionToString(const StructDefinition* d, const Allocator* allocator)
{
    LIST_TO_STRING(d->members, a, StructMemberToString, allocator);
    TemLangString s = { .allocator = allocator };
    if (InstructionListIsEmpty(&d->deleteInstructions)) {
        TemLangStringAppendFormat(
          s, "{ \"members\": %s, \"isVariant\": false }", a.buffer);
    } else {
        TemLangString s1 =
          InstructionListToString(&d->deleteInstructions, allocator);
        TemLangStringAppendFormat(
          s,
          "{ \"members\": %s, \"deleterTargetName\":   \"%s\", "
          "\"deleteInstructions\": %s, \"isVariant\": false }",
          a.buffer,
          d->destructorTargetName.buffer,
          s1.buffer);
        TemLangStringFree(&s1);
    }
    TemLangStringFree(&a);
    return s;
}

static inline TemLangString
StructDefinitionToTemLang(const TemLangString* name,
                          const StructDefinition* d,
                          const Allocator* allocator)
{
    TemLangStringCreateFormat(s,
                              allocator,
                              "%s %s\n{\n",
                              d->isVariant ? "variant" : "struct",
                              name->buffer);
    for (size_t i = 0; i < d->members.used; ++i) {
        const StructMember* m = &d->members.buffer[i];
        if (m->isKeyword) {
            switch (m->quantity) {
                case 0:
                    TemLangStringAppendFormat(s,
                                              "\t[0 %s] %s\n",
                                              KeywordToString(m->keyword),
                                              m->name.buffer);
                    break;
                case 1:
                    TemLangStringAppendFormat(s,
                                              "\t%s %s\n",
                                              KeywordToString(m->keyword),
                                              m->name.buffer);
                    break;
                default:
                    TemLangStringAppendFormat(s,
                                              "\t[%zu %s] %s\n",
                                              m->quantity,
                                              KeywordToString(m->keyword),
                                              m->name.buffer);
                    break;
            }
        } else {
            switch (m->quantity) {
                case 0:
                    TemLangStringAppendFormat(
                      s, "\t[0 #%s] %s\n", m->typeName.buffer, m->name.buffer);
                    break;
                case 1:
                    TemLangStringAppendFormat(
                      s, "\t#%s %s\n", m->typeName.buffer, m->name.buffer);
                    break;
                default:
                    TemLangStringAppendFormat(s,
                                              "\t[%zu #%s] %s\n",
                                              m->quantity,
                                              m->typeName.buffer,
                                              m->name.buffer);
                    break;
            }
        }
    }
    TemLangStringAppendChars(&s, "}\n");
    return s;
}