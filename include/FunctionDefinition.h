#pragma once

#include "FunctionType.h"
#include "InstructionList.h"
#include "TemLangString.h"

typedef struct FunctionDefinition
{
    FunctionType type;
    InstructionList instructions;
    union
    {
        struct
        {
            TemLangString leftParameter;
            TemLangString rightParameter;
        };
        TemLangStringList captures;
    };
} FunctionDefinition, *pFunctionDefinition;

static inline void
FunctionDefinitionFree(FunctionDefinition* f);

static inline bool
FunctionDefinitionCopy(FunctionDefinition* dest,
                       const FunctionDefinition* src,
                       const Allocator* allocator);

static inline TemLangString
FunctionDefinitionToString(const FunctionDefinition* f,
                           const Allocator* allocator);