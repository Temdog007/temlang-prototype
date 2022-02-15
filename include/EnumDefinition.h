#pragma once

#include "Allocator.h"
#include "TemLangString.h"

typedef struct EnumDefinition
{
    TemLangStringList members;
    bool isFlag;
} EnumDefinition, *pEnumDefinition;

static inline TemLangString
EnumDefinitionToString(const EnumDefinition* e, const Allocator* allocator)
{
    TemLangString n = TemLangStringListToString(&e->members, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"isFlag\": %s, \"members\": %s }",
                              e->isFlag ? "true" : "false",
                              n.buffer);
    TemLangStringFree(&n);
    return s;
}

static inline void
EnumDefinitionFree(EnumDefinition* e)
{
    TemLangStringListFree(&e->members);
}

static inline bool
EnumDefinitionCopy(EnumDefinition* dest,
                   const EnumDefinition* src,
                   const Allocator* allocator)
{
    EnumDefinitionFree(dest);
    dest->isFlag = src->isFlag;
    return TemLangStringListCopy(&dest->members, &src->members, allocator);
}
