#pragma once

#include "AtomType.h"
#include "EnumDefinition.h"
#include "FunctionDefinition.h"
#include "StructDefinition.h"
#include "Variable.h"

typedef struct Atom
{
    AtomType type;
    TemLangString name;
    bool notCompiled;
    union
    {
        Variable variable;
        Range range;
        EnumDefinition enumDefinition;
        StructDefinition structDefinition;
        FunctionDefinition functionDefinition;
    };
} Atom, *pAtom;

static inline void
AtomFree(Atom* atom)
{
    TemLangStringFree(&atom->name);
    switch (atom->type) {
        case AtomType_Variable:
            VariableFree(&atom->variable);
            break;
        case AtomType_Enum:
            EnumDefinitionFree(&atom->enumDefinition);
            break;
        case AtomType_Struct:
            StructDefinitionFree(&atom->structDefinition);
            break;
        case AtomType_Function:
            FunctionDefinitionFree(&atom->functionDefinition);
            break;
        default:
            break;
    }
    memset(atom, 0, sizeof(Atom));
}

static inline TemLangString
AtomToString(const Atom* atom, const Allocator* allocator)
{
    TemLangString v = { 0 };
    switch (atom->type) {
        case AtomType_Variable:
            v = VariableToString(&atom->variable, allocator);
            break;
        case AtomType_Range:
            v = RangeToString(&atom->range, allocator);
            break;
        case AtomType_Enum:
            v = EnumDefinitionToString(&atom->enumDefinition, allocator);
            break;
        case AtomType_Struct:
            v = StructDefinitionToString(&atom->structDefinition, allocator);
            break;
        case AtomType_Function:
            v =
              FunctionDefinitionToString(&atom->functionDefinition, allocator);
            break;
        default:
            v = TemLangStringCreate("null", allocator);
            break;
    }
    TemLangStringCreateFormat(
      s,
      allocator,
      "{  \"name\": \"%s\", \"compiles\": %s, \"type\": "
      "\"%s\", \"value\": %s }",
      atom->name.buffer,
      atom->notCompiled ? "false" : "true",
      AtomTypeToString(atom->type),
      v.buffer);
    TemLangStringFree(&v);
    return s;
}

static inline bool
AtomCopy(Atom* dest, const Atom* src, const Allocator* allocator)
{
    AtomFree(dest);
    dest->type = src->type;
    dest->notCompiled = src->notCompiled;
    if (!TemLangStringCopy(&dest->name, &src->name, allocator)) {
        return false;
    }
    switch (src->type) {
        case AtomType_Variable:
            return VariableCopy(&dest->variable, &src->variable, allocator);
        case AtomType_Range:
            dest->range = src->range;
            return true;
        case AtomType_Enum:
            return EnumDefinitionCopy(
              &dest->enumDefinition, &src->enumDefinition, allocator);
        case AtomType_Struct:
            return StructDefinitionCopy(
              &dest->structDefinition, &src->structDefinition, allocator);
        case AtomType_Function:
            return FunctionDefinitionCopy(
              &dest->functionDefinition, &src->functionDefinition, allocator);
        default:
            copyFailure(AtomTypeToString(src->type));
            return false;
    }
}

static inline void
AtomExistsError(const Atom* atom)
{
    TemLangError("Atom '%s' already exists as type: '%s'",
                 atom->name.buffer,
                 AtomTypeToString(atom->type));
}

static inline void
AtomNotFoundError(const TemLangString* name)
{
    TemLangError("Atom '%s' does not exist", name->buffer);
}

static inline ComparisonOperator
AtomCompareName(const Atom* atom, const TemLangString* name)
{
    return TemLangStringCompare(&atom->name, name);
}

static inline bool
AtomNameEquals(const Atom* atom, const TemLangString* name)
{
    return AtomCompareName(atom, name) == ComparisonOperator_EqualTo;
}

static inline ComparisonOperator
AtomCompare(const Atom* a, const Atom* b)
{
    return AtomCompareName(a, &b->name);
}