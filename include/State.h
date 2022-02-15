#pragma once

#include "Atom.h"
#include "Error.h"
#include "Expression.h"
#include "Instruction.h"
#include "Lexer.h"
#include "ProcessTokensArgs.h"
#include "Variable.h"

#include <errno.h>

MAKE_LIST(Atom)
DEFAULT_MAKE_LIST_FUNCTIONS(Atom)

typedef struct State State, *pState;

extern int
REPL_print(const char*, ...);

static inline NamedValueList
StateToVariableList(const State* state, const Allocator* allocator);

static inline bool
StructMemberToFakeValue(const StructMember* m,
                        const State* state,
                        const Allocator* allocator,
                        pNamedValue nv);

typedef struct State
{
    AtomList atoms;
    const State* parent;
} State, *pState;

static inline void
StateFree(State* state)
{
    AtomListFree(&state->atoms);
}

static inline bool
StateCopy(State* dest, const State* src, const Allocator* allocator)
{
    StateFree(dest);
    dest->parent = src->parent;
    if (AtomListIsEmpty(&src->atoms)) {
        dest->atoms.allocator = allocator;
        return true;
    }
    return AtomListCopy(&dest->atoms, &src->atoms, allocator);
}

static inline TemLangString
StateToString(const State* state, const Allocator* allocator)
{
    TemLangString value = TemLangStringCreate("[", allocator);
    for (size_t i = 0; i < state->atoms.used; ++i) {
        TemLangString t = AtomToString(&state->atoms.buffer[i], allocator);
        TemLangStringAppend(&value, &t);
        TemLangStringFree(&t);
        if (i != state->atoms.used - 1) {
            TemLangStringAppendChar(&value, ',');
        }
    }
    TemLangStringAppendChar(&value, ']');
    TemLangString parent = state->parent == NULL
                             ? TemLangStringCreate("null", allocator)
                             : StateToString(state->parent, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"value\": %s, \"parent\": %s }",
                              value.buffer,
                              parent.buffer);
    TemLangStringFree(&value);
    TemLangStringFree(&parent);
    return s;
}

typedef struct StateFindArgs
{
    bool log;
    bool searchParent;
} StateFindArgs, *pStateFindArgs;

static inline Atom*
StateFindAnyAtom(State* state,
                 const TemLangString* name,
                 const StateFindArgs args)
{
    for (size_t i = 0; i < state->atoms.used; ++i) {
        Atom* atom = &state->atoms.buffer[i];
        if (TemLangStringCompare(&atom->name, name) ==
            ComparisonOperator_EqualTo) {
            return atom;
        }
    }
    if (args.log) {
        AtomNotFoundError(name);
    }
    return NULL;
}

static inline const Atom*
StateFindAnyAtomConst(const State* state,
                      const TemLangString* name,
                      const StateFindArgs args)
{
    for (size_t i = 0; i < state->atoms.used; ++i) {
        const Atom* atom = &state->atoms.buffer[i];
        if (TemLangStringCompare(&atom->name, name) ==
            ComparisonOperator_EqualTo) {
            return atom;
        }
    }
    if (args.searchParent && state->parent != NULL) {
        return StateFindAnyAtomConst(state->parent, name, args);
    }
    if (args.log) {
        AtomNotFoundError(name);
    }
    return NULL;
}

static inline Atom*
StateFindAtom(State* state,
              const TemLangString* name,
              const AtomType atomType,
              const StateFindArgs args)
{
    Atom* atom = StateFindAnyAtom(state, name, args);
    if (atom != NULL) {
        if (atom->type != atomType) {
            if (args.log) {
                UnexpectedAtomTypeError(NULL, atomType, atom->type);
            }
            return NULL;
        }
    }
    return atom;
}

static inline const Atom*
StateFindAtomConst(const State* state,
                   const TemLangString* name,
                   const AtomType atomType,
                   const StateFindArgs args)
{
    const Atom* atom = StateFindAnyAtomConst(state, name, args);
    if (atom != NULL) {
        if (atom->type != atomType) {
            if (args.log) {
                UnexpectedAtomTypeError(NULL, atomType, atom->type);
            }
            return NULL;
        }
    }
    return atom;
}

static inline void
InstructionError(const Instruction* i)
{
    if (i->type == InstructionType_Error) {
        TemLangError("Manual error occured (%s:%zu)",
                     i->source.source.buffer,
                     i->source.lineNumber);
    } else {
        TemLangError("Instruction '%s' could not be executed (%s:%zu)",
                     InstructionTypeToString(i->type),
                     i->source.source.buffer,
                     i->source.lineNumber);
    }
}

static inline bool
StateAddVariable(State* state,
                 const TemLangString* name,
                 const Variable* variable)
{
    if (TemLangStringIsEmpty(name)) {
        TemLangError("Emtpy strings cannot be used for variable names");
        return false;
    }
    {
        const StateFindArgs args = { .log = false, .searchParent = false };
        const Atom* atom =
          StateFindAtomConst(state, name, AtomType_Variable, args);
        if (atom != NULL) {
            AtomExistsError(atom);
            return false;
        }
    }

    const Atom atom = { .type = AtomType_Variable,
                        .name = *name,
                        .variable = *variable };
    return AtomListAppend(&state->atoms, &atom);
}

static inline bool
StateAddValueWithMutability(State* state,
                            const TemLangString* name,
                            const Value* value,
                            const VariableType type)
{
    Variable v = { .type = type, .value = *value };
    return StateAddVariable(state, name, &v);
}

static inline bool
StateAddValue(State* state, const TemLangString* name, const Value* value)
{
    return StateAddValueWithMutability(
      state, name, value, VariableType_Immutable);
}

#define CHECK_ATOM_EXISTS(name, doSearchParent)                                \
    {                                                                          \
        const StateFindArgs args = { .log = false,                             \
                                     .searchParent = doSearchParent };         \
        const Atom* atom = StateFindAnyAtomConst(state, &name, args);          \
        if (atom != NULL) {                                                    \
            AtomExistsError(atom);                                             \
            result = false;                                                    \
            break;                                                             \
        }                                                                      \
    }

static inline bool
HandleListModifyInstruction(const ListModifyInstruction* i,
                            State* state,
                            const Allocator* allocator);

static inline bool
CaptureVariables(State* dest,
                 const State* src,
                 const TemLangStringList* captures,
                 const InstructionSource source);

static inline bool
UpdateCapturedVariables(State* dest,
                        const State* src,
                        const TemLangStringList* captures,
                        const Allocator* allocator);

static inline bool
StateProcessInstruction(State* state,
                        const Instruction* instruction,
                        const Allocator* allocator,
                        pValue value)
{
    if (!InstructionTypeCanBeExecuted(instruction->type)) {
        return true;
    }

    bool result = true;
    switch (instruction->type) {
        case InstructionType_NoCompile: {
            for (size_t i = 0; result && i < instruction->instructions.used;
                 ++i) {
                ValueFree(value);
                const Instruction* tempI = &instruction->instructions.buffer[i];
                result =
                  StateProcessInstruction(state, tempI, allocator, value);
                if (!result) {
                    continue;
                }
                if (!InstructionCreatesAtom(tempI->type)) {
                    continue;
                }
                const TemLangString* name = InstructionCreationName(tempI);
                if (name == NULL) {
                    TemLangError("Compiler error! Failed to get instruction "
                                 "creation name.");
                    result = false;
                    continue;
                }
                const StateFindArgs args = { .log = true,
                                             .searchParent = false };
                Atom* atom = StateFindAnyAtom(state, name, args);
                if (atom == NULL) {
                    TemLangError(
                      "Compiler error! Failed to find created atom '%s'",
                      name->buffer);
                    result = false;
                    continue;
                }
                atom->notCompiled = true;
            }
            ValueFree(value);
        } break;
        case InstructionType_Return:
            if (instruction->expression.type == ExpressionType_UnaryVariable &&
                !TemLangStringStartsWith(&instruction->expression.identifier,
                                         "r_")) {
                TemLangError("Returning variables must start with 'r_'");
                result = false;
                break;
            }
            result = EvaluateExpression(
              &instruction->expression, state, value, allocator);
            break;
        case InstructionType_IfReturn: {
            result = EvaluateExpression(
              &instruction->ifCondition, state, value, allocator);
            if (!result) {
                break;
            }
            if (value->type != ValueType_Boolean) {
                TemLangError("If condition must be a boolean");
                result = false;
                break;
            }
            if (value->b) {
                result = EvaluateExpression(
                  &instruction->ifResult, state, value, allocator);
            } else {
                value->type = ValueType_Null;
            }
        } break;
        case InstructionType_Verify: {
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom =
              StateFindAnyAtomConst(state, &instruction->verifyName, args);
            if (atom == NULL) {
                TemLangError("Verify failed for atom '%s'",
                             instruction->verifyName.buffer);
                return false;
            }
            REPL_print("Atom '%s' is a '%s'",
                       instruction->verifyName.buffer,
                       AtomTypeToString(atom->type));
            return true;
        } break;
        case InstructionType_CreateVariable: {
            CHECK_ATOM_EXISTS(instruction->createVariable.name, false);

            Variable variable = { .type = instruction->createVariable.type };
            result = EvaluateExpression(&instruction->createVariable.value,
                                        state,
                                        &variable.value,
                                        allocator);
            if (!result) {
                goto createVariableCleanup;
            }
            result = StateAddVariable(
              state, &instruction->createVariable.name, &variable);
        createVariableCleanup:
            VariableFree(&variable);
        } break;
        case InstructionType_UpdateVariable: {
            Value* target = EvaluateExpressionToReference(
              &instruction->updateVariable.target, state, allocator);
            if (target == NULL) {
                result = false;
                break;
            }
            Value newValue = { 0 };
            result = EvaluateExpression(
              &instruction->updateVariable.value, state, &newValue, allocator);
            if (!result) {
                goto updateVariableCleanup;
            }
            result = ValueTransition(target, &newValue, allocator);
            if (!result) {
                TemLangString from = ValueToString(target, allocator);
                TemLangString to = ValueToString(&newValue, allocator);
                TemLangError("Cannot transition value '%s' to '%s'",
                             from.buffer,
                             to.buffer);
                TemLangStringFree(&from);
                TemLangStringFree(&to);
            }
        updateVariableCleanup:
            ValueFree(&newValue);
        } break;
        case InstructionType_DefineRange: {
            CHECK_ATOM_EXISTS(instruction->defineRange.name, true)

            Value value = { 0 };
            Atom atom = { .type = AtomType_Range };
            if (!TemLangStringCopy(
                  &atom.name, &instruction->defineRange.name, allocator)) {
                result = false;
                goto defineRangeCleanup;
            }

            if (!EvaluateExpression(
                  &instruction->defineRange.min, state, &value, allocator)) {
                result = false;
                goto defineRangeCleanup;
            }
            if (value.type != ValueType_Number) {
                result = false;
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Number, value.type);
                goto defineRangeCleanup;
            }
            atom.range.min = value.rangedNumber.number;
            ValueFree(&value);

            if (!EvaluateExpression(
                  &instruction->defineRange.max, state, &value, allocator)) {
                result = false;
                goto defineRangeCleanup;
            }
            if (value.type != ValueType_Number) {
                result = false;
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Number, value.type);
                goto defineRangeCleanup;
            }
            atom.range.max = value.rangedNumber.number;

            result = AtomListAppend(&state->atoms, &atom);
        defineRangeCleanup:
            ValueFree(&value);
            AtomFree(&atom);
        } break;
        case InstructionType_DefineEnum: {
            CHECK_ATOM_EXISTS(instruction->defineEnum.name, true);

            const size_t length =
              instruction->defineEnum.definition.members.used;
            Atom atom = { .type = AtomType_Enum };
            Atom lengthAtom = { .type = AtomType_Variable,
                                .variable = {
                                  .type = VariableType_Constant,
                                  .value = {
                                    .type = ValueType_Number,
                                    .rangedNumber = {
                                      .hasRange = false,
                                      .number = NumberFromUInt(length) } } } };
            atom.enumDefinition.isFlag = false;
            lengthAtom.name.allocator = allocator;
            TemLangStringAppendFormat(lengthAtom.name,
                                      "%s_Length",
                                      instruction->defineEnum.name.buffer);
            CHECK_ATOM_EXISTS(lengthAtom.name, true);
            result = TemLangStringCopy(
                       &atom.name, &instruction->defineEnum.name, allocator) &&
                     EnumDefinitionCopy(&atom.enumDefinition,
                                        &instruction->defineEnum.definition,
                                        allocator) &&
                     AtomListAppend(&state->atoms, &atom) &&
                     AtomListAppend(&state->atoms, &lengthAtom);
            AtomFree(&atom);
            AtomFree(&lengthAtom);
        } break;
        case InstructionType_ChangeFlag: {
            Value* target = EvaluateExpressionToReference(
              &instruction->changeFlag.target, state, allocator);
            if (target == NULL) {
                result = false;
                break;
            }

            if (target->type != ValueType_Flag) {
                if (target->type == ValueType_Boolean &&
                    instruction->changeFlag.flag == ChangeFlagType_Toggle) {
                    target->b = !target->b;
                    break;
                }
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Flag, target->type);
                result = false;
                break;
            }

            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &target->flagValue.name, AtomType_Enum, args);
            if (atom == NULL) {
                result = false;
                break;
            }

            if (!atom->enumDefinition.isFlag) {
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Flag, ValueType_Enum);
                result = false;
                break;
            }

            if (!TemLangStringListFindIf(
                  &atom->enumDefinition.members,
                  (TemLangStringListFindFunc)TemLangStringsAreEqual,
                  &instruction->changeFlag.member,
                  NULL,
                  NULL)) {
                TemLangError("Member '%s' not found in flag",
                             instruction->changeFlag.member.buffer);
                result = false;
                break;
            }

            const bool hasMember = TemLangStringListFindIf(
              &target->flagValue.members,
              (TemLangStringListFindFunc)TemLangStringsAreEqual,
              &instruction->changeFlag.member,
              NULL,
              NULL);
            enum
            {
                DoInsert,
                DoRemoval,
                DoNothing
            } action = DoNothing;
            switch (instruction->changeFlag.flag) {
                case ChangeFlagType_Add:
                    action = hasMember ? DoNothing : DoInsert;
                    break;
                case ChangeFlagType_Remove:
                    action = hasMember ? DoRemoval : DoNothing;
                    break;
                default:
                    action = hasMember ? DoRemoval : DoInsert;
                    break;
            }
            switch (action) {
                case DoInsert:
                    result =
                      TemLangStringListAppend(&target->flagValue.members,
                                              &instruction->changeFlag.member);
                    break;
                case DoRemoval:
                    TemLangStringListRemoveIf(
                      &target->flagValue.members,
                      (TemLangStringListFindFunc)TemLangStringsAreEqual,
                      &instruction->changeFlag.member,
                      allocator);
                    break;
                case DoNothing:
                default:
                    break;
            }
        } break;
        case InstructionType_SetAllFlag: {
            Value* target = EvaluateExpressionToReference(
              &instruction->setAllFlag.target, state, allocator);
            if (target == NULL) {
                result = false;
                break;
            }

            if (target->type != ValueType_Flag) {
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Flag, target->type);
                result = false;
                break;
            }

            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &target->flagValue.name, AtomType_Enum, args);
            if (atom == NULL) {
                result = false;
                break;
            }

            if (!atom->enumDefinition.isFlag) {
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Flag, ValueType_Enum);
                result = false;
                break;
            }

            if (instruction->setAllFlag.clear) {
                TemLangStringListFree(&target->flagValue.members);
            } else {
                result = TemLangStringListCopy(&target->flagValue.members,
                                               &atom->enumDefinition.members,
                                               allocator);
            }
        } break;
        case InstructionType_DefineStruct: {
            CHECK_ATOM_EXISTS(instruction->defineStruct.name, true);

            Atom atom = { .type = AtomType_Struct };
            result = TemLangStringCopy(&atom.name,
                                       &instruction->defineStruct.name,
                                       allocator) &&
                     StructDefinitionCopy(&atom.structDefinition,
                                          &instruction->defineStruct.definition,
                                          allocator) &&
                     AtomListAppend(&state->atoms, &atom);
            AtomFree(&atom);
        } break;
        case InstructionType_DefineFunction: {
            CHECK_ATOM_EXISTS(instruction->functionName, true);

            switch (instruction->functionDefinition.type) {
                case FunctionType_Unary:
                    if (!TemLangStringStartsWith(
                          &instruction->functionDefinition.leftParameter,
                          "p_")) {
                        goto parameterError;
                    }
                    break;
                case FunctionType_Binary:
                    if (!TemLangStringStartsWith(
                          &instruction->functionDefinition.leftParameter,
                          "p_") &&
                        !TemLangStringStartsWith(
                          &instruction->functionDefinition.rightParameter,
                          "p_")) {
                        goto parameterError;
                    }
                    break;
                default:
                    break;
            }

            Atom atom = { .type = AtomType_Function };
            result = TemLangStringCopy(
                       &atom.name, &instruction->functionName, allocator) &&
                     FunctionDefinitionCopy(&atom.functionDefinition,
                                            &instruction->functionDefinition,
                                            allocator) &&
                     AtomListAppend(&state->atoms, &atom);
            AtomFree(&atom);
            break;
        parameterError:
            TemLangError("Parameters must start with a 'p_'");
            result = false;
        } break;
        case InstructionType_Run: {
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &instruction->procedureName, AtomType_Function, args);
            if (atom == NULL) {
                result = false;
                break;
            }
            if (atom->functionDefinition.type != FunctionType_Procedure) {
                TemLangError(
                  "Run instruction is only valid for procedures. Got '%s'",
                  FunctionTypeToString(atom->functionDefinition.type));
                result = false;
                break;
            }
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            result = CaptureVariables(&temp,
                                      state,
                                      &atom->functionDefinition.captures,
                                      instruction->source);
            if (!result) {
                goto runStateFree;
            }
            const InstructionList* instructions =
              &atom->functionDefinition.instructions;
            ValueFree(value);
            for (size_t i = 0; result && value->type == ValueType_Null &&
                               i < instructions->used;
                 ++i) {
                result = StateProcessInstruction(
                  &temp, &instructions->buffer[i], allocator, value);
                if (!result) {
                    InstructionError(&instructions->buffer[i]);
                }
            }
            if (!result) {
                goto runStateFree;
            }
            result = UpdateCapturedVariables(
              state, &temp, &atom->functionDefinition.captures, allocator);
        runStateFree:
            StateFree(&temp);
        } break;
        case InstructionType_ListModify: {
            result = HandleListModifyInstruction(
              &instruction->listModify, state, allocator);
        } break;
        case InstructionType_While:
        case InstructionType_Until: {
            const CaptureInstruction* w = &instruction->captureInstruction;
            const bool isWhile = instruction->type == InstructionType_While;
            do {
                ValueFree(value);
                result =
                  EvaluateExpression(&w->target, state, value, allocator);
                if (!result) {
                    break;
                }
                switch (value->type) {
                    case ValueType_Null:
                        if (isWhile) {
                            goto endWhileLoop;
                        }
                        break;
                    case ValueType_Boolean:
                        if (isWhile != value->b) {
                            goto endWhileLoop;
                        }
                        break;
                    default:
                        if (!isWhile) {
                            goto endWhileLoop;
                        }
                        break;
                }
                State temp = { 0 };
                temp.parent = state;
                temp.atoms.allocator = allocator;
                if (!CaptureVariables(
                      &temp, state, &w->captures, instruction->source)) {
                    result = false;
                    goto stateFree;
                }
                ValueFree(value);
                for (size_t i = 0;
                     value->type == ValueType_Null && i < w->instructions.used;
                     ++i) {
                    result = StateProcessInstruction(
                      &temp, &w->instructions.buffer[i], allocator, value);
                    if (!result) {
                        goto stateFree;
                    }
                }
                result = UpdateCapturedVariables(
                  state, &temp, &w->captures, allocator);
            stateFree:
                ValueFree(value);
                StateFree(&temp);
            } while (result);
        endWhileLoop:
            ValueFree(value);
            break;
        } break;
        case InstructionType_Iterate: {
            const CaptureInstruction* c = &instruction->captureInstruction;
            result = EvaluateExpression(&c->target, state, value, allocator);
            if (!result) {
                break;
            }
            const Value falseValue = { .type = ValueType_Boolean, .b = false };
            Value tempValue = { 0 };
            switch (value->type) {
                case ValueType_Number: {
                    if (!value->rangedNumber.hasRange) {
                        TemLangError("Only ranged numbers can be iterated on");
                        result = false;
                        break;
                    }
                    const Range range = value->rangedNumber.range;
                    const int64_t start =
                      NumberToInt(&value->rangedNumber.number);
                    const int64_t end = NumberToInt(&range.max) + 1;
                    bool continueLoop = true;
                    for (int64_t i = start; continueLoop && result && i < end;
                         ++i) {
                        State temp = { 0 };
                        temp.atoms.allocator = allocator;
                        temp.parent = state;
                        result = CaptureVariables(
                          &temp, state, &c->captures, instruction->source);
                        if (!result) {
                            goto numberIterateStateFree;
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("item", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = VariableType_Immutable;
                            atom.variable.value.type = ValueType_Number;
                            atom.variable.value.rangedNumber.hasRange = true;
                            const Number n = { .type = NumberType_Signed,
                                               .i = i };
                            atom.variable.value.rangedNumber.number = n;
                            atom.variable.value.rangedNumber.range = range;
                            result = AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        if (!result) {
                            goto numberIterateStateFree;
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("index", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = VariableType_Immutable;
                            atom.variable.value.type = ValueType_Number;
                            atom.variable.value.rangedNumber.hasRange = true;
                            const Number n = { .type = NumberType_Signed,
                                               .i = i - start };
                            atom.variable.value.rangedNumber.number = n;
                            atom.variable.value.rangedNumber.range = range;
                            result = AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        if (!result) {
                            goto numberIterateStateFree;
                        }
                        for (size_t j = 0;
                             continueLoop && result && j < c->instructions.used;
                             ++j) {
                            ValueFree(&tempValue);
                            result = StateProcessInstruction(
                              &temp,
                              &c->instructions.buffer[j],
                              allocator,
                              &tempValue);
                            continueLoop =
                              !ValuesMatch(&tempValue, &falseValue);
                        }
                        if (result) {
                            result = UpdateCapturedVariables(
                              state, &temp, &c->captures, allocator);
                        }
                    numberIterateStateFree:
                        StateFree(&temp);
                    }
                } break;
                case ValueType_List: {
                    const ValueList* values = &value->list.values;
                    const Range range = { .min = { .type = NumberType_Unsigned,
                                                   .u = 0UL },
                                          .max = { .type = NumberType_Unsigned,
                                                   .u = values->used - 1 } };
                    bool continueLoop = true;
                    for (size_t i = 0;
                         continueLoop && result &&
                         tempValue.type == ValueType_Null && i < values->used;
                         ++i) {
                        State temp = { 0 };
                        temp.atoms.allocator = allocator;
                        temp.parent = state;
                        result = CaptureVariables(
                          &temp, state, &c->captures, instruction->source);
                        if (!result) {
                            goto iterateStateFree;
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("index", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = false;
                            atom.variable.value.type = ValueType_Number;
                            atom.variable.value.rangedNumber.hasRange = true;
                            const Number n = { .type = NumberType_Unsigned,
                                               .u = i };
                            atom.variable.value.rangedNumber.number = n;
                            atom.variable.value.rangedNumber.range = range;
                            result = AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        if (!result) {
                            goto iterateStateFree;
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("item", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = VariableType_Immutable;
                            result = ValueCopy(&atom.variable.value,
                                               &values->buffer[i],
                                               allocator) &&
                                     AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        if (!result) {
                            goto iterateStateFree;
                        }
                        for (size_t j = 0;
                             continueLoop && result && j < c->instructions.used;
                             ++j) {
                            ValueFree(&tempValue);
                            result = StateProcessInstruction(
                              &temp,
                              &c->instructions.buffer[j],
                              allocator,
                              &tempValue);
                            continueLoop =
                              !ValuesMatch(&tempValue, &falseValue);
                        }
                        if (result) {
                            result = UpdateCapturedVariables(
                              state, &temp, &c->captures, allocator);
                        }
                    iterateStateFree:
                        StateFree(&temp);
                    }
                } break;
                case ValueType_Enum: {
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* enumAtom = StateFindAtomConst(
                      state, &value->enumValue.name, AtomType_Enum, args);
                    if (enumAtom == NULL) {
                        result = false;
                        break;
                    }
                    const Range range = {
                        .min = { .type = NumberType_Unsigned, .u = 0UL },
                        .max = { .type = NumberType_Unsigned,
                                 .u =
                                   enumAtom->enumDefinition.members.used - 1 }
                    };
                    bool continueLoop = true;
                    for (size_t i = 0;
                         continueLoop && result &&
                         i < enumAtom->enumDefinition.members.used;
                         ++i) {
                        State temp = { 0 };
                        temp.atoms.allocator = allocator;
                        temp.parent = state;
                        result = CaptureVariables(
                          &temp, state, &c->captures, instruction->source);
                        if (!result) {
                            goto iterateEnumStateFree;
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("index", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = false;
                            atom.variable.value.type = ValueType_Number;
                            atom.variable.value.rangedNumber.hasRange = true;
                            const Number n = { .type = NumberType_Unsigned,
                                               .u = i };
                            atom.variable.value.rangedNumber.number = n;
                            atom.variable.value.rangedNumber.range = range;
                            result = AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        {
                            Atom atom = { 0 };
                            atom.name = TemLangStringCreate("item", allocator);
                            atom.type = AtomType_Variable;
                            atom.variable.type = VariableType_Immutable;
                            atom.variable.value.type = ValueType_Enum;
                            result =
                              TemLangStringCopy(
                                &atom.variable.value.enumValue.name,
                                &enumAtom->name,
                                allocator) &&
                              TemLangStringCopy(
                                &atom.variable.value.enumValue.value,
                                &enumAtom->enumDefinition.members.buffer[i],
                                allocator) &&
                              AtomListAppend(&temp.atoms, &atom);
                            AtomFree(&atom);
                        }
                        if (!result) {
                            goto iterateEnumStateFree;
                        }
                        for (size_t j = 0;
                             continueLoop && result && j < c->instructions.used;
                             ++j) {
                            ValueFree(&tempValue);
                            result = StateProcessInstruction(
                              &temp,
                              &c->instructions.buffer[j],
                              allocator,
                              &tempValue);
                            continueLoop =
                              !ValuesMatch(&tempValue, &falseValue);
                        }
                        if (result) {
                            result = UpdateCapturedVariables(
                              state, &temp, &c->captures, allocator);
                        }
                    iterateEnumStateFree:
                        StateFree(&temp);
                    }
                } break;
                default:
                    TemLangError("Cannot iterate on value of type '%s'",
                                 ValueTypeToString(value->type));
                    result = false;
                    break;
            }
            ValueFree(&tempValue);
            ValueFree(value);
        } break;
        case InstructionType_Match: {
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            result = CaptureVariables(&temp,
                                      state,
                                      &instruction->matchInstruction.captures,
                                      instruction->source);
            if (!result) {
                goto matchCleanup;
            }
            result =
              EvaluateMatchExpression(&instruction->matchInstruction.expression,
                                      &temp,
                                      value,
                                      allocator);
            if (!result) {
                goto matchCleanup;
            }
            result = UpdateCapturedVariables(
              state, &temp, &instruction->matchInstruction.captures, allocator);
        matchCleanup:
            StateFree(&temp);
        } break;
        case InstructionType_Print:
        case InstructionType_Error: {
            for (size_t i = 0; result && i < instruction->printExpressions.used;
                 ++i) {
                ValueFree(value);
                result =
                  EvaluateExpression(&instruction->printExpressions.buffer[i],
                                     state,
                                     value,
                                     allocator);
                if (result) {
                    TemLangString s = ValueToString(value, allocator);
                    if (instruction->type == InstructionType_Print) {
                        REPL_print("%s\n", s.buffer);
                    } else {
                        TemLangError("%s", s.buffer);
                        result = false;
                    }
                    TemLangStringFree(&s);
                }
            }
            ValueFree(value);
        } break;
        case InstructionType_ConvertContainer: {
            Value value = { 0 };
            result = EvaluateExpression(
              &instruction->fromContainer, state, &value, allocator);
            if (!result) {
                break;
            }
            if (value.type == ValueType_List) {
                value.list.isArray = instruction->toArray;
                result =
                  StateAddValue(state, &instruction->toContainer, &value);
            } else {
                TemLangError("ConvertContainer instruction expects a list or "
                             "array to convert. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
            }
            ValueFree(&value);
        } break;
        case InstructionType_InlineVariable: {
            NamedValue nv = { 0 };
            result = StructMemberToFakeValue(
              &instruction->definitionType, state, allocator, &nv);
            if (!result) {
                goto inlineVariableCleanup;
            }
            if (nv.value.type != ValueType_Type) {
                TemLangError("Inlined variable must be given a valid type");
                result = false;
                goto inlineVariableCleanup;
            }
            result =
              StateAddValueWithMutability(state,
                                          &instruction->definitionName,
                                          nv.value.fakeValue,
                                          instruction->definitionVariableType);
        inlineVariableCleanup:
            NamedValueFree(&nv);
        } break;
        case InstructionType_Inline: {
            result = EvaluateExpression(
              &instruction->expression, state, value, allocator);
            if (!result) {
                break;
            }
            if (value->type != ValueType_String) {
                TemLangError("Inline requires string. Got '%s'",
                             ValueTypeToString(value->type));
                result = false;
                goto mixinCleanup;
            }

            char buffer[1024] = { 0 };
            snprintf(buffer,
                     sizeof(buffer),
                     "<Inlined instructions at (%s:%zu)>",
                     instruction->source.source.buffer,
                     instruction->source.lineNumber);
            TokenList list = performLex(
              allocator, value->string.buffer, value->string.used, 1, buffer);
            InstructionList instructions =
              TokensToInstructions(&list, allocator);
            if (instructions.used == 0) {
                TemLangError("Failed to parse any instructions from '%s'",
                             value->string.buffer);
                result = false;
                goto mixinCleanup;
            }
            ValueFree(value);
            for (size_t i = 0; i < instructions.used; ++i) {
                ValueFree(value);
                result = StateProcessInstruction(
                  state, &instructions.buffer[i], allocator, value);
                if (!result) {
                    InstructionError(&instructions.buffer[i]);
                    break;
                }
                if (value->type != ValueType_Null) {
                    TemLangError("Inline instructions cannot return a value");
                    InstructionError(&instructions.buffer[i]);
                    break;
                }
            }
        mixinCleanup:
            ValueFree(value);
            InstructionListFree(&instructions);
            TokenListFree(&list);
        } break;
        case InstructionType_InlineText:
        case InstructionType_InlineData: {
            Value value = { 0 };
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            result = EvaluateExpression(
              &instruction->dataFile, state, &value, allocator);
            if (!result) {
                goto inlineDataCleanup;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined data must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                goto inlineDataCleanup;
            }
            if (mapFile(
                  value.string.buffer, &fd, &ptr, &size, MapFileType_Read)) {
                Value newValue = { 0 };
                newValue.type = ValueType_Data;
                newValue.string = TemLangStringCreate("", allocator);
                result = TemLangStringAppendCount(&newValue.string, ptr, size);
                if (!instruction->dataIsBinary) {
                    TemLangStringRemoveNewLines(&newValue.string);
                }
                if (!result) {
                    goto inlineDataCleanup;
                }
                result =
                  StateAddValue(state, &instruction->dataName, &newValue);
                ValueFree(&newValue);
            } else {
                TemLangError("Failed to open file '%s': %s",
                             value.string.buffer,
                             strerror(errno));
                result = false;
            }
        inlineDataCleanup:
            ValueFree(&value);
            unmapFile(fd, ptr, size);
        } break;
        case InstructionType_InlineFile: {
            Value value = { 0 };
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            result = EvaluateExpression(
              &instruction->expression, state, &value, allocator);
            if (!result) {
                goto inlineFileCleanup;
            }
            if (value.type != ValueType_String) {
                TemLangError("Inlined file must be a string. Got '%s'",
                             ValueTypeToString(value.type));
                result = false;
                goto inlineFileCleanup;
            }
            if (mapFile(
                  value.string.buffer, &fd, &ptr, &size, MapFileType_Read)) {
                TokenList tokens =
                  performLex(allocator, ptr, size, 0UL, value.string.buffer);
                InstructionList instructions =
                  TokensToInstructions(&tokens, allocator);
                Value tempValue = { 0 };
                for (size_t i = 0; i < instructions.used; ++i) {
                    ValueFree(&tempValue);
                    result = StateProcessInstruction(
                      state, &instructions.buffer[i], allocator, &tempValue);
                    if (!result) {
                        InstructionError(&instructions.buffer[i]);
                        break;
                    }
                    if (tempValue.type != ValueType_Null) {
                        TemLangError(
                          "Inline instructions cannot return a value");
                        InstructionError(&instructions.buffer[i]);
                        break;
                    }
                }
                ValueFree(&tempValue);
                InstructionListFree(&instructions);
                TokenListFree(&tokens);
            } else {
                TemLangError("Failed to open file '%s': %s",
                             value.string.buffer,
                             strerror(errno));
                result = false;
            }
        inlineFileCleanup:
            ValueFree(&value);
            unmapFile(fd, ptr, size);
        } break;
        case InstructionType_Format: {
            result = EvaluateExpression(
              &instruction->formatArgs, state, value, allocator);
            if (!result) {
                break;
            }
            if (value->type != ValueType_Struct) {
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Struct, value->type);
                result = false;
                break;
            }
            Value output = { .type = ValueType_String,
                             .string = { .allocator = allocator } };
            size_t i = 0;
            const TemLangString* content = &instruction->formatString;
            while (result && i < content->used) {
                const char c = content->buffer[i];
                ++i;
                switch (c) {
                    case '\\':
                        if (i < content->used) {
                            result = TemLangStringAppendChar(
                              &output.string, content->buffer[i]);
                        } else {
                            TemLangError(
                              "Warning: Format strings should not end with a "
                              "'\\' character");
                        }
                        ++i;
                        break;
                    case '%': {
                        const size_t end = continueWhile(
                          content->buffer, content->used, i, isIdentifierChar);
                        const size_t length = end - i;
                        TemLangString t = TemLangStringCreateFromSize(
                          content->buffer + i, length + 1, allocator);
                        const NamedValue* nv = NULL;
                        result = NamedValueListFindIf(
                          value->structValues,
                          (NamedValueListFindFunc)NamedValueNameEqualsString,
                          &t,
                          &nv,
                          NULL);
                        if (result) {
                            TemLangStringFree(&t);
                            t = ValueToSimpleString(&nv->value, allocator);
                            result = TemLangStringAppend(&output.string, &t);
                            i = end;
                        } else {
                            TemLangError(
                              "Cannot find member '%s' in arguments struct "
                              "for format string",
                              t.buffer);
                        }
                        TemLangStringFree(&t);
                    } break;
                    default:
                        result = TemLangStringAppendChar(&output.string, c);
                        break;
                }
            }
            ValueFree(value);
            if (result) {
                result =
                  StateAddValue(state, &instruction->formatName, &output);
            }
            ValueFree(&output);
        } break;
        case InstructionType_NumberRound: {
            result = EvaluateExpression(
              &instruction->numberRoundTarget, state, value, allocator);
            if (!result) {
                goto numberRoundEnd;
            }
            if (value->type != ValueType_Number) {
                UnexpectedValueTypeError(
                  &instruction->source, ValueType_Number, value->type);
                result = false;
                goto numberRoundEnd;
            }
            if (value->rangedNumber.number.type != NumberType_Float) {
                TemLangError(
                  "'%s' instruction requires a floating point number\n",
                  NumberRoundToString(instruction->numberRound));
                result = false;
                goto numberRoundEnd;
            }
            if (!StructMemberToRange(&instruction->numberRoundMember,
                                     state,
                                     &value->rangedNumber.range)) {
                TemLangError(
                  "Struct member must be a number. Got '%s'",
                  instruction->numberRoundMember.isKeyword
                    ? KeywordToString(instruction->numberRoundMember.keyword)
                    : instruction->numberRoundMember.name.buffer);
                result = false;
                goto numberRoundEnd;
            }
            double d;
            switch (instruction->numberRound) {
                case NumberRound_Floor:
                    d = floor(value->rangedNumber.number.d);
                    break;
                case NumberRound_Ceil:
                    d = ceil(value->rangedNumber.number.d);
                    break;
                case NumberRound_Round:
                    d = round(value->rangedNumber.number.d);
                    break;
                default:
                    result = false;
                    goto numberRoundEnd;
            }
            switch (instruction->numberRoundMember.keyword) {
                case Keyword_i8:
                case Keyword_i16:
                case Keyword_i32:
                case Keyword_i64:
                    value->rangedNumber.number.type = NumberType_Signed;
                    value->rangedNumber.number.i = (int64_t)d;
                    break;
                case Keyword_u8:
                case Keyword_u16:
                case Keyword_u32:
                case Keyword_u64:
                    value->rangedNumber.number.type = NumberType_Unsigned;
                    value->rangedNumber.number.u = (uint64_t)d;
                    break;
                case Keyword_f32:
                case Keyword_f64:
                    value->rangedNumber.number.type = NumberType_Float;
                    value->rangedNumber.number.d = (uint64_t)d;
                    break;
                default:
                    result = false;
                    goto numberRoundEnd;
            }
            value->rangedNumber.hasRange = true;
            if (!numberInRange(&value->rangedNumber.number,
                               &value->rangedNumber.range)) {
                numberNotInRangeError(&value->rangedNumber.number,
                                      &value->rangedNumber.range,
                                      allocator);
                result = false;
                goto numberRoundEnd;
            }
            result = StateAddValue(state, &instruction->numberRoundName, value);
        numberRoundEnd:
            ValueFree(value);
        } break;
        default:
            result = false;
            break;
    }
    return result;
}

static inline bool
HandleListModifyInstruction(const ListModifyInstruction* i,
                            State* state,
                            const Allocator* allocator)
{
    Value* value = EvaluateExpressionToReference(&i->list, state, allocator);
    if (value == NULL) {
        return false;
    }
    Value newValue = { 0 };
    bool result = true;
    switch (value->type) {
        case ValueType_String: {
            char c;
            switch (i->type) {
                case ListModifyType_Append: {
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator)) {
                        switch (newValue.type) {
                            case ValueType_String:
                                result = TemLangStringAppend(&value->string,
                                                             &newValue.string);
                                break;
                            case ValueType_Number:
                                result =
                                  ValueToChar(&newValue, &c) &&
                                  TemLangStringAppendChar(&value->string, c);
                                break;
                            case ValueType_List:
                                for (size_t i = 0;
                                     result && i < newValue.list.values.used;
                                     ++i) {
                                    result =
                                      ValueToChar(
                                        &newValue.list.values.buffer[i], &c) &&
                                      TemLangStringAppendChar(&value->string,
                                                              c);
                                }
                                break;
                            default:
                                TemLangError(
                                  "Cannot add value to string because it "
                                  "is not a character.");
                                result = false;
                        }
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_Insert: {
                    uint64_t index = 0;
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator) &&
                        ValueToIndex(&newValue, &index)) {
                        if (EvaluateExpression(
                              &i->newValue[1], state, &newValue, allocator) &&
                            ValueToChar(&newValue, &c)) {
                            result =
                              TemLangStringInsertChar(&value->string, c, index);
                        } else {
                            TemLangError(
                              "Cannot add value to string because it "
                              "is not a character.");
                            result = false;
                        }
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_Remove: {
                    uint64_t index = 0;
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator) &&
                        ValueToIndex(&newValue, &index)) {
                        if (index >= value->string.used) {
                            TemLangError(
                              "Index out of range. Size: %zu; Index=%" PRIu64,
                              value->string.used,
                              index);
                            result = false;
                        }
                        result = TemLangStringRemove(&value->string, index);
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_SwapRemove:
                    TemLangError("Swap remove is invalid for strings");
                    result = false;
                    break;
                case ListModifyType_Pop:
                    TemLangStringPop(&value->string);
                    break;
                case ListModifyType_Empty: {
                    const Allocator* a = value->string.allocator;
                    TemLangStringFree(&value->string);
                    value->string.allocator = a;
                } break;
                default:
                    result = false;
                    break;
            }
        } break;
        case ValueType_List: {
            if (value->list.isArray) {
                TemLangError("Cannot add/remove values from arrays");
                result = false;
                break;
            }
            switch (i->type) {
                case ListModifyType_Append: {
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator)) {
                        if (ValueCanTransition(
                              value->list.exampleValue, &newValue, allocator)) {
                            if (value->list.exampleValue->type ==
                                ValueType_Number) {
                                newValue.rangedNumber.hasRange = true;
                                newValue.rangedNumber.range =
                                  value->list.exampleValue->rangedNumber.range;
                            }
                            result =
                              ValueListAppend(&value->list.values, &newValue);
                        } else {
                            TemLangError(
                              "Cannot add value to list because of "
                              "a type mismatch. List has '%s' but "
                              "tried to add '%s'",
                              ValueTypeToString(value->list.exampleValue->type),
                              ValueTypeToString(newValue.type));
                            result = false;
                        }
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_Insert: {
                    uint64_t index = 0;
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator) &&
                        ValueToIndex(&newValue, &index)) {
                        if (EvaluateExpression(
                              &i->newValue[1], state, &newValue, allocator) &&
                            ValueCanTransition(
                              value->list.exampleValue, &newValue, allocator)) {
                            if (value->list.exampleValue->type ==
                                ValueType_Number) {
                                newValue.rangedNumber.hasRange = true;
                                newValue.rangedNumber.range =
                                  value->list.exampleValue->rangedNumber.range;
                            }
                            result = ValueListInsert(
                              &value->list.values, index, &newValue);
                        } else {
                            TemLangError(
                              "Cannot add value to list because of "
                              "a type mismatch. List has '%s' but "
                              "tried to add '%s'",
                              ValueTypeToString(value->list.exampleValue->type),
                              ValueTypeToString(newValue.type));
                            result = false;
                        }
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_Remove: {
                    uint64_t index = 0;
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator) &&
                        ValueToIndex(&newValue, &index)) {
                        if (index >= value->list.values.used) {
                            TemLangError(
                              "Index out of range. Size: %zu; Index=%" PRIu64,
                              value->list.values.used,
                              index);
                            result = false;
                        }
                        result = ValueListRemove(
                          &value->list.values, index, allocator);
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_SwapRemove: {
                    uint64_t index = 0;
                    if (EvaluateExpression(
                          &i->newValue[0], state, &newValue, allocator) &&
                        ValueToIndex(&newValue, &index)) {
                        if (index >= value->list.values.used) {
                            TemLangError(
                              "Index out of range. Size: %zu; Index=%" PRIu64,
                              value->list.values.used,
                              index);
                            result = false;
                        }
                        result =
                          ValueListSwapRemove(&value->list.values, index);
                    } else {
                        result = false;
                    }
                } break;
                case ListModifyType_Pop:
                    ValueListPop(&value->list.values);
                    break;
                case ListModifyType_Empty: {
                    const Allocator* a = value->list.values.allocator;
                    ValueListFree(&value->list.values);
                    value->list.values.allocator = a;
                } break;
                default:
                    result = false;
                    break;
            }
        } break;
        default:
            break;
    }
    ValueFree(&newValue);
    return result;
}

static inline Value
StateProcessTokens(State* state,
                   const TokenList* list,
                   const Allocator* allocator,
                   struct ProcessTokensArgs args)
{
    if (args.printTokens) {
        for (size_t i = 0; i < list->used; ++i) {
            TemLangString s = TokenToString(&list->buffer[i], allocator);
            REPL_print("#%zu\n%s\n", i, s.buffer);
            TemLangStringFree(&s);
        }
    }

    Value v = { 0 };
    InstructionList instructions = TokensToInstructions(list, allocator);
    for (size_t i = 0; v.type == ValueType_Null && i < instructions.used; ++i) {
        if (args.printInstructions) {
            TemLangString s =
              InstructionToString(&instructions.buffer[i], allocator);
            REPL_print("#%zu\n%s\n", i, s.buffer);
            TemLangStringFree(&s);
        }
        if (!StateProcessInstruction(
              state, &instructions.buffer[i], allocator, &v)) {
            InstructionError(&instructions.buffer[i]);
            break;
        }
    }
    InstructionListFree(&instructions);
    return v;
}

static inline bool
StructMemberToRange(const StructMember* m, const State* state, pRange range)
{
    if (m->isKeyword) {
        return KeywordToRange(m->keyword, range);
    }
    StateFindArgs args = { .log = true, .searchParent = true };
    const Atom* atom =
      StateFindAtomConst(state, &m->name, AtomType_Range, args);
    if (atom == NULL) {
        return false;
    }
    *range = atom->range;
    return true;
}

static inline bool
KeywordToRange(const Keyword keyword, pRange range)
{
    switch (keyword) {
        case Keyword_i8:
            range->min = NumberFromInt(INT8_MIN);
            range->max = NumberFromInt(INT8_MAX);
            return true;
        case Keyword_i16:
            range->min = NumberFromInt(INT16_MIN);
            range->max = NumberFromInt(INT16_MAX);
            return true;
        case Keyword_i32:
            range->min = NumberFromInt(INT32_MIN);
            range->max = NumberFromInt(INT32_MAX);
            return true;
        case Keyword_i64:
            range->min = NumberFromInt(INT64_MIN);
            range->max = NumberFromInt(INT64_MAX);
            return true;
        case Keyword_u8:
            range->min = NumberFromUInt(0U);
            range->max = NumberFromUInt(UINT8_MAX);
            return true;
        case Keyword_u16:
            range->min = NumberFromUInt(0U);
            range->max = NumberFromUInt(UINT16_MAX);
            return true;
        case Keyword_u32:
            range->min = NumberFromUInt(0U);
            range->max = NumberFromUInt(UINT32_MAX);
            return true;
        case Keyword_u64:
            range->min = NumberFromUInt(0U);
            range->max = NumberFromUInt(UINT64_MAX);
            return true;
        case Keyword_f32:
            range->min = NumberFromDouble(-FLT_MAX);
            range->max = NumberFromDouble(FLT_MAX);
            return true;
        case Keyword_f64:
            range->min = NumberFromDouble(-DBL_MAX);
            range->max = NumberFromDouble(DBL_MAX);
            return true;
        default:
            return false;
    }
}

#define ExpressionEvaluate EvaluateExpression

static inline bool
EvaluateExpression(const Expression* e,
                   const State* state,
                   pValue value,
                   const Allocator* allocator)
{
    ValueFree(value);
    bool result = false;
    switch (e->type) {
        case ExpressionType_Nullary:
            value->type = ValueType_Null;
            result = true;
            break;
        case ExpressionType_UnaryValue:
            result = ValueCopy(value, &e->value, allocator);
            break;
        case ExpressionType_UnaryVariable: {
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &e->identifier, AtomType_Variable, args);
            if (atom == NULL) {
                result = false;
                break;
            }
            result = ValueCopy(value, &atom->variable.value, allocator);
        } break;
        case ExpressionType_UnaryScope: {
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            result = true;
            for (size_t i = 0;
                 value->type == ValueType_Null && i < e->instructions.used;
                 ++i) {
                result = StateProcessInstruction(
                  &temp, &e->instructions.buffer[i], allocator, value);
                if (!result) {
                    InstructionError(&e->instructions.buffer[i]);
                    break;
                }
            }
            StateFree(&temp);
        } break;
        case ExpressionType_UnaryStruct: {
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            result = true;
            Value unused = { 0 };
            for (size_t i = 0; i < e->instructions.used; ++i) {
                ValueFree(&unused);
                result = StateProcessInstruction(
                  &temp, &e->instructions.buffer[i], allocator, &unused);
                if (!result) {
                    InstructionError(&e->instructions.buffer[i]);
                    break;
                }
                if (unused.type != ValueType_Null) {
                    TemLangError("Unary structs cannot return a value");
                    InstructionError(&e->instructions.buffer[i]);
                    break;
                }
            }
            ValueFree(&unused);
            if (result) {
                value->type = ValueType_Struct;
                value->structValues =
                  allocator->allocate(sizeof(NamedValueList));
                *value->structValues = StateToVariableList(&temp, allocator);
                value->structValuesAllocator = allocator;
            }
            StateFree(&temp);
        } break;
        case ExpressionType_UnaryMatch: {
            State temp = { 0 };
            temp.parent = state;
            temp.atoms.allocator = allocator;
            result = EvaluateMatchExpression(
              e->matchExpression, &temp, value, allocator);
            StateFree(&temp);
        } break;
        case ExpressionType_UnaryList:
            if (e->expressions.used == 0) {
                TemLangError(
                  "Cannot make empty list or array with list expression");
                result = false;
                break;
            }
            value->type = ValueType_List;
            value->list.isArray = e->isArray;
            value->list.allocator = allocator;
            value->list.values.allocator = allocator;
            result = true;
            for (size_t i = 0; result && i < e->expressions.used; ++i) {
                Value temp = { 0 };
                result =
                  EvaluateExpression(
                    &e->expressions.buffer[i], state, &temp, allocator) &&
                  ValueListAppend(&value->list.values, &temp);
            }
            if (!result) {
                break;
            }

            result = ValueListValueIsValid(&value->list, allocator);
            if (!result) {
                break;
            }

            if (value->list.values.buffer[0].type == ValueType_Number) {
                const Range range =
                  value->list.values.buffer[0].rangedNumber.range;
                for (size_t i = 1; i < value->list.values.used; ++i) {
                    value->list.values.buffer[i].rangedNumber.range = range;
                    value->list.values.buffer[i].rangedNumber.hasRange = true;
                }
            }

            value->list.exampleValue = allocator->allocate(sizeof(Value));
            result = ValueCopy(value->list.exampleValue,
                               &value->list.values.buffer[0],
                               allocator);

            break;
        case ExpressionType_Binary:
            result = EvaluateBinaryExpression(
              e->left, &e->op, e->right, state, allocator, value);
            break;
        default:
            break;
    }
    if (!result) {
        EvaluateExpressionError(e);
    }
    return result;
}

Value*
EvaluateExpressionToReference(const Expression* e,
                              State* state,
                              const Allocator* allocator)
{
    switch (e->type) {
        case ExpressionType_UnaryVariable: {
            const StateFindArgs args = { .log = true, .searchParent = false };
            Atom* atom =
              StateFindAtom(state, &e->identifier, AtomType_Variable, args);
            if (atom == NULL) {
                return NULL;
            }
            if (atom->variable.type == VariableType_Mutable) {
                return &atom->variable.value;
            } else {
                TemLangError(
                  "Cannot get reference to variable because it is not mutable");
                return NULL;
            }
        } break;
        case ExpressionType_Binary: {
            switch (e->op.type) {
                case OperatorType_Get: {
                    Value* result = NULL;
                    Value right = { 0 };
                    Value* left =
                      EvaluateExpressionToReference(e->left, state, allocator);
                    if (left == NULL) {
                        goto cleanupValue;
                    }
                    if (!ValueHasMembers(left)) {
                        TemLangError(
                          "Can't use get operator on value type '%s'",
                          ValueTypeToString(left->type));
                        goto cleanupValue;
                    }
                    if (!EvaluateExpression(
                          e->right, state, &right, allocator)) {
                        goto cleanupValue;
                    }
                    result = ValueIndex(state, left, &right);
                cleanupValue:
                    ValueFree(&right);
                    return result;
                } break;
                default:
                    break;
            }
        } break;
        default:
            break;
    }
    TemLangString s = ExpressionToString(e, allocator);
    TemLangError("Expression '%s' is not a reference to an existing variable",
                 s.buffer);
    TemLangStringFree(&s);
    return NULL;
}

static inline bool
EvaluateGetExpression(const Value* left,
                      const Value* right,
                      const State* state,
                      const Allocator* allocator,
                      const GetOperator op,
                      pValue value)
{
    switch (op) {
        case GetOperator_Type: {
            const Value* stringValue = NULL;
            const Value* nullValue = NULL;
            if (!ValueTypesTryMatch(left,
                                    ValueType_String,
                                    right,
                                    ValueType_Null,
                                    &stringValue,
                                    &nullValue)) {
                break;
            }
            const StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom =
              StateFindAnyAtomConst(state, &stringValue->string, args);
            if (atom == NULL) {
                break;
            }
            value->type = ValueType_Type;
            value->fakeValue = allocator->allocate(sizeof(Value));
            value->fakeValueAllocator = allocator;
            Value* fakeValue = value->fakeValue;
            switch (atom->type) {
                case AtomType_Range: {
                    fakeValue->type = ValueType_Number;
                    fakeValue->rangedNumber.number = atom->range.min;
                    fakeValue->rangedNumber.range = atom->range;
                    fakeValue->rangedNumber.hasRange = true;
                    return true;
                } break;
                case AtomType_Variable:
                    return ValueCopy(
                      fakeValue, &atom->variable.value, allocator);
                case AtomType_Enum: {
                    if (atom->enumDefinition.isFlag) {
                        fakeValue->type = ValueType_Flag;
                        return TemLangStringCopy(
                          &fakeValue->flagValue.name, &atom->name, allocator);
                    } else {
                        fakeValue->type = ValueType_Enum;
                        return TemLangStringCopy(
                                 &fakeValue->enumValue.value,
                                 &atom->enumDefinition.members.buffer[0],
                                 allocator) &&
                               TemLangStringCopy(&fakeValue->enumValue.name,
                                                 &atom->name,
                                                 allocator);
                    }
                } break;
                case AtomType_Struct: {
                    if (atom->structDefinition.isVariant) {
                        fakeValue->type = ValueType_Variant;
                        fakeValue->variantValue.allocator = allocator;
                        fakeValue->variantValue.value =
                          allocator->allocate(sizeof(Value));
                        {
                            NamedValue nv = { 0 };
                            if (!StructMemberToFakeValue(
                                  &atom->structDefinition.members.buffer[0],
                                  state,
                                  allocator,
                                  &nv)) {
                                return false;
                            }
                            if (nv.value.type == ValueType_Type) {
                                ValueCopy(fakeValue->variantValue.value,
                                          nv.value.fakeValue,
                                          allocator);
                            } else {
                                ValueCopy(fakeValue->variantValue.value,
                                          &nv.value,
                                          allocator);
                            }
                            NamedValueFree(&nv);
                        }
                        return TemLangStringCopy(&fakeValue->variantValue.name,
                                                 &atom->name,
                                                 allocator) &&
                               TemLangStringCopy(
                                 &fakeValue->variantValue.memberName,
                                 &atom->structDefinition.members.buffer[0].name,
                                 allocator);
                    } else {
                        fakeValue->type = ValueType_Struct;
                        fakeValue->structValuesAllocator = allocator;
                        fakeValue->structValues =
                          allocator->allocate(sizeof(NamedValueList));
                        fakeValue->structValues->allocator = allocator;
                        for (size_t i = 0;
                             i < atom->structDefinition.members.used;
                             ++i) {
                            const StructMember* m =
                              &atom->structDefinition.members.buffer[i];
                            NamedValue nv = { 0 };
                            bool result =
                              StructMemberToFakeValue(m, state, allocator, &nv);
                            if (result) {
                                // Ensure structs of struct give an actual
                                // struct and not a fake value
                                NamedValue temp = { .name = nv.name };
                                if (nv.value.type == ValueType_Type) {
                                    temp.value = *nv.value.fakeValue;
                                } else {
                                    temp.value = nv.value;
                                }
                                result = NamedValueListAppend(
                                  fakeValue->structValues, &temp);
                            }
                            NamedValueFree(&nv);
                            if (!result) {
                                return false;
                            }
                        }
                    }
                    return true;
                } break;
                default:
                    break;
            }
        } break;
        case GetOperator_Member: {
            const Value* result = ValueconstIndex(state, left, right);
            if (result == NULL) {
                break;
            }
            return ValueCopy(value, result, allocator);
        } break;
        case GetOperator_MakeList: {
            const Value* target = getUnaryValue(left, right);
            if (target == NULL) {
                break;
            }
            value->type = ValueType_List;
            value->list.allocator = allocator;
            value->list.isArray = false;
            value->list.exampleValue = allocator->allocate(sizeof(Value));
            ValueListFree(&value->list.values);
            return ValueCopy(
              value->list.exampleValue, target->fakeValue, allocator);
        } break;
        case GetOperator_Length: {
            const Value* target = getUnaryValue(left, right);
            if (target == NULL) {
                break;
            }
            switch (target->type) {
                case ValueType_List: {
                    value->type = ValueType_Number;
                    value->rangedNumber.hasRange = false;
                    value->rangedNumber.number =
                      NumberFromUInt(target->list.values.used);
                    return true;
                } break;
                case ValueType_Enum: {
                    value->type = ValueType_Number;
                    value->rangedNumber.hasRange = false;
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      state, &target->enumValue.name, AtomType_Enum, args);
                    if (atom == NULL) {
                        break;
                    }
                    value->rangedNumber.number =
                      NumberFromUInt(atom->enumDefinition.members.used);
                    return true;
                } break;
                default:
                    TemLangError(
                      "Cannot used length operator on value type '%s'",
                      ValueTypeToString(target->type));
                    break;
            }
        } break;
        default:
            TemLangError("Failed to evaluate get operator '%s'",
                         GetOperatorToString(op));
            break;
    }
    return false;
}

static inline bool
EvaluateFunctionExpression(const Value* left,
                           const TemLangString* name,
                           const Value* right,
                           const State* state,
                           const Allocator* allocator,
                           pValue value)
{
    const StateFindArgs args = { .log = true, .searchParent = true };
    const Atom* atom = StateFindAtomConst(state, name, AtomType_Function, args);
    if (atom == NULL) {
        return false;
    }
    bool result = true;
    State temp = { 0 };
    temp.parent = state;
    temp.atoms.allocator = allocator;
    const InstructionList* instructions =
      &atom->functionDefinition.instructions;
    switch (atom->functionDefinition.type) {
        case FunctionType_Unary: {
            const Value* target = getUnaryValue(left, right);
            if (target == NULL) {
                TemLangError("Expected one null value and one non-null value. "
                             "Got '%s' and '%s'",
                             ValueTypeToString(left->type),
                             ValueTypeToString(right->type));
                result = false;
                break;
            }
            Atom newAtom = { 0 };
            newAtom.type = AtomType_Variable;
            newAtom.variable.type = VariableType_Immutable;
            if (!TemLangStringCopy(&newAtom.name,
                                   &atom->functionDefinition.leftParameter,
                                   allocator) ||
                !ValueCopy(&newAtom.variable.value, target, allocator)) {
                result = false;
            }
            result = AtomListAppend(&temp.atoms, &newAtom);
            AtomFree(&newAtom);
        } break;
        case FunctionType_Binary: {
            Atom leftAtom = { 0 };
            Atom rightAtom = { 0 };
            rightAtom.type = leftAtom.type = AtomType_Variable;
            rightAtom.variable.type = leftAtom.variable.type =
              VariableType_Immutable;
            result = TemLangStringCopy(&leftAtom.name,
                                       &atom->functionDefinition.leftParameter,
                                       allocator) &&
                     ValueCopy(&leftAtom.variable.value, left, allocator) &&
                     TemLangStringCopy(&rightAtom.name,
                                       &atom->functionDefinition.rightParameter,
                                       allocator) &&
                     ValueCopy(&rightAtom.variable.value, right, allocator) &&
                     AtomListAppend(&temp.atoms, &leftAtom) &&
                     AtomListAppend(&temp.atoms, &rightAtom);
            AtomFree(&leftAtom);
            AtomFree(&rightAtom);
        } break;
        case FunctionType_Procedure:
            TemLangError("Cannot call procedures like functions");
            result = false;
            break;
        case FunctionType_Nullary:
        default:
            if (left->type != ValueType_Null || right->type != ValueType_Null) {
                TemLangError("Expected 2 NULL values for nullary function. "
                             "Got '%s' or '%s'",
                             ValueTypeToString(left->type),
                             ValueTypeToString(right->type));
                result = false;
            }
            break;
    }
    for (size_t i = 0;
         result && value->type == ValueType_Null && i < instructions->used;
         ++i) {
        result = StateProcessInstruction(
          &temp, &instructions->buffer[i], allocator, value);
        if (!result) {
            InstructionError(&instructions->buffer[i]);
            break;
        }
    }
    StateFree(&temp);
    return result;
}

static inline bool
StructsMatch(const NamedValueList* list,
             const NamedValueList* types,
             const Allocator* allocator)
{
    if (list->used != types->used) {
        TemLangError("Member count doesn't match when matching struct. "
                     "Expected %zu. Got %zu.",
                     types->used,
                     list->used);
        return false;
    }
    const NamedValue* nv = NULL;
    for (size_t i = 0; i < types->used; ++i) {
        nv = NULL;
        NamedValueListFindIf(list,
                             (NamedValueListFindFunc)NamedValueNameEquals,
                             &types->buffer[i],
                             &nv,
                             NULL);
        if (nv == NULL) {
            TemLangError("Missing member '%s' in struct",
                         types->buffer[i].name.buffer);
            return false;
        }
        const Value* targetValue = NULL;
        if (nv->value.type == ValueType_Type) {
            targetValue = types->buffer[i].value.fakeValue;
        } else {
            targetValue = &nv->value;
        }
        if (!ValueCanTransition(&nv->value, targetValue, allocator)) {
            if (nv->value.type == targetValue->type) {
                TemLangError("Type mismatch for member '%s' in struct",
                             types->buffer[i].name.buffer);
            } else {
                TemLangError("Type mismatch for member '%s' in struct. Got "
                             "'%s'; Expected '%s'",
                             types->buffer[i].name.buffer,
                             ValueTypeToString(nv->value.type),
                             ValueTypeToString(targetValue->type));
            }
            return false;
        }
    }
    return true;
}

static inline bool
EvaluateAddExpression(const Value* left,
                      const Value* right,
                      const State* state,
                      const Allocator* allocator,
                      pValue value)
{
    if (ValueTypesMatch(left, ValueType_String, right, ValueType_String)) {
        value->type = ValueType_String;
        value->string =
          TemLangStringCombine(&left->string, &right->string, allocator);
        return true;
    }
    {
        const Value* numberValue = NULL;
        const Value* enumValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Number,
                               right,
                               ValueType_Enum,
                               &numberValue,
                               &enumValue)) {
            StateFindArgs args = { .log = true, .searchParent = true };
            const Atom* atom = StateFindAtomConst(
              state, &enumValue->enumValue.name, AtomType_Enum, args);
            if (atom == NULL) {
                return false;
            }
            size_t index;
            if (!TemLangStringListFindIf(
                  &atom->enumDefinition.members,
                  (TemLangStringListFindFunc)TemLangStringsAreEqual,
                  &enumValue->enumValue.value,
                  NULL,
                  &index)) {
                MemberNotFoundError(
                  "Enum", &atom->name, &enumValue->enumValue.value);
                TemLangString members = TemLangStringListToString(
                  &atom->enumDefinition.members, allocator);
                TemLangError("Members %s", members.buffer);
                TemLangStringFree(&members);
                return false;
            }
            const int64_t n = NumberToInt(&numberValue->rangedNumber.number);

            value->type = ValueType_Enum;
            index = (n + index) % atom->enumDefinition.members.used;
            return TemLangStringCopy(
                     &value->enumValue.value,
                     &atom->enumDefinition.members.buffer[index],
                     allocator) &&
                   TemLangStringCopy(
                     &value->enumValue.name, &atom->name, allocator);
        }
    }
    {
        const Value* numberValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Number,
                               right,
                               ValueType_Type,
                               &numberValue,
                               &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            const Allocator* allocator = typeValue->fakeValueAllocator;
            switch (fakeValue->type) {
                case ValueType_Number: {
                    if (!numberInRange(&numberValue->rangedNumber.number,
                                       &fakeValue->rangedNumber.range)) {
                        numberNotInRangeError(&numberValue->rangedNumber.number,
                                              &fakeValue->rangedNumber.range,
                                              allocator);
                        return false;
                    }
                    value->type = ValueType_Number;
                    value->rangedNumber.number =
                      numberValue->rangedNumber.number;
                    value->rangedNumber.range = fakeValue->rangedNumber.range;
                    value->rangedNumber.hasRange = true;
                    return true;
                } break;
                case ValueType_String: {
                    value->type = ValueType_String;
                    value->string = NumberToString(
                      &numberValue->rangedNumber.number, allocator);
                    return true;
                } break;
                case ValueType_Enum: {
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      state, &fakeValue->enumValue.name, AtomType_Enum, args);
                    if (atom == NULL) {
                        return false;
                    }
                    const size_t index =
                      NumberToUInt(&numberValue->rangedNumber.number);
                    if (index >= atom->enumDefinition.members.used) {
                        TemLangError("Enum '%s' only has %zu members but tried "
                                     "to get index %zu",
                                     fakeValue->enumValue.name.buffer,
                                     atom->enumDefinition.members.used,
                                     index);
                        return false;
                    }
                    value->type = ValueType_Enum;
                    return TemLangStringCopy(&value->enumValue.name,
                                             &fakeValue->enumValue.name,
                                             allocator) &&
                           TemLangStringCopy(
                             &value->enumValue.value,
                             &atom->enumDefinition.members.buffer[index],
                             allocator);
                } break;
                default:
                    break;
            }
        }
    }
    {
        const Value* stringValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_String,
                               right,
                               ValueType_Type,
                               &stringValue,
                               &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            const Allocator* allocator = typeValue->fakeValueAllocator;
            switch (fakeValue->type) {
                case ValueType_Enum:
                case ValueType_Flag: {
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      state, &fakeValue->enumValue.name, AtomType_Enum, args);
                    if (atom == NULL) {
                        break;
                    }
                    if (TemLangStringIsEmpty(&stringValue->string) &&
                        fakeValue->type == ValueType_Flag) {
                        value->type = ValueType_Flag;
                        value->flagValue.members.allocator = allocator;
                        value->flagValue.members.size = 0UL;
                        value->flagValue.members.used = 0UL;
                        value->flagValue.members.buffer = NULL;
                        return true;
                    }
                    if (!TemLangStringListFindIf(
                          &atom->enumDefinition.members,
                          (TemLangStringListFindFunc)TemLangStringsAreEqual,
                          &stringValue->string,
                          NULL,
                          NULL)) {
                        MemberNotFoundError(
                          fakeValue->type == ValueType_Enum ? "Enum" : "Flag",
                          &fakeValue->enumValue.name,
                          &stringValue->string);
                        break;
                    }
                    if (fakeValue->type == ValueType_Enum) {
                        value->type = ValueType_Enum;
                        return TemLangStringCopy(&value->enumValue.name,
                                                 &fakeValue->enumValue.name,
                                                 allocator) &&
                               TemLangStringCopy(&value->enumValue.value,
                                                 &stringValue->string,
                                                 allocator);
                    } else {
                        value->type = ValueType_Flag;
                        value->flagValue.members.allocator = allocator;
                        return TemLangStringListAppend(
                                 &value->flagValue.members,
                                 &stringValue->string) &&
                               TemLangStringCopy(&value->flagValue.name,
                                                 &fakeValue->flagValue.name,
                                                 allocator);
                    }
                } break;
                case ValueType_Number: {
                    if (isNumber(allocator,
                                 stringValue->string.buffer,
                                 stringValue->string.used,
                                 &value->rangedNumber.number)) {
                        value->type = ValueType_Number;
                        if (!numberInRange(&value->rangedNumber.number,
                                           &fakeValue->rangedNumber.range)) {
                            numberNotInRangeError(
                              &value->rangedNumber.number,
                              &fakeValue->rangedNumber.range,
                              allocator);
                            break;
                        }
                        value->rangedNumber.hasRange = true;
                        value->rangedNumber.range =
                          fakeValue->rangedNumber.range;
                        return true;
                    }
                    TemLangError("'%s' cannot be converted to a number",
                                 stringValue->string.buffer);
                } break;
                default:
                    break;
            }
        }
    }
    {
        const Value* structValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Struct,
                               right,
                               ValueType_Type,
                               &structValue,
                               &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            const Allocator* allocator = typeValue->fakeValueAllocator;
            switch (fakeValue->type) {
                case ValueType_Struct:
                    if (StructsMatch(structValue->structValues,
                                     fakeValue->structValues,
                                     allocator)) {
                        return ValueCopy(value, structValue, allocator);
                    }
                    break;
                case ValueType_Variant: {
                    if (structValue->structValues->used != 1) {
                        TemLangError("Structs converted to variants should "
                                     "only have 1 member. Got %zu",
                                     structValue->structValues->used);
                        break;
                    }
                    const NamedValue* nv =
                      &structValue->structValues->buffer[0];
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom =
                      StateFindAtomConst(state,
                                         &fakeValue->variantValue.name,
                                         AtomType_Struct,
                                         args);
                    if (atom == NULL) {
                        break;
                    }
                    if (!atom->structDefinition.isVariant) {
                        UnexpectedValueTypeError(
                          NULL, ValueType_Variant, ValueType_Struct);
                        break;
                    }
                    const StructMember* m = NULL;
                    StructMemberListFindIf(
                      &atom->structDefinition.members,
                      (StructMemberListFindFunc)StructMemberNameEquals,
                      &nv->name,
                      &m,
                      NULL);
                    if (m == NULL) {
                        TemLangError("Failed to find member '%s' in varaint",
                                     nv->name.buffer);
                        break;
                    }
                    NamedValue temp = { 0 };
                    if (!StructMemberToFakeValue(m, state, allocator, &temp)) {
                        return false;
                    }
                    const Value* fakeValue = temp.value.type == ValueType_Type
                                               ? temp.value.fakeValue
                                               : &temp.value;
                    bool result =
                      ValueCanTransition(&nv->value, fakeValue, allocator);
                    if (result) {
                        value->type = ValueType_Variant;
                        value->variantValue.allocator = allocator;
                        pValue varValue = allocator->allocate(sizeof(Value));
                        value->variantValue.value = varValue;
                        result =
                          TemLangStringCopy(&value->variantValue.name,
                                            &atom->name,
                                            allocator) &&
                          TemLangStringCopy(&value->variantValue.memberName,
                                            &m->name,
                                            allocator) &&
                          ValueCopy(varValue, &nv->value, allocator);
                        if (result) {
                            if (varValue->type == ValueType_Number) {
                                const Range range =
                                  fakeValue->rangedNumber.range;
                                varValue->rangedNumber.range = range;
                                varValue->rangedNumber.hasRange = true;
                            }
                        }
                    } else {
                        TemLangString ts = StructMemberToString(
                          m, typeValue->fakeValueAllocator);
                        TemLangError(
                          "Member '%s' is not the correct type. Expected '%s'",
                          nv->name.buffer,
                          ts.buffer);
                        TemLangStringFree(&ts);
                        result = false;
                    }
                    NamedValueFree(&temp);
                    return result;
                } break;
                default:
                    break;
            }
        }
    }
    {
        const Value* variantValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Variant,
                               right,
                               ValueType_Type,
                               &variantValue,
                               &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            switch (fakeValue->type) {
                case ValueType_String: {
                    value->type = ValueType_String;
                    return TemLangStringCopy(
                      &value->string,
                      &variantValue->variantValue.memberName,
                      allocator);
                } break;
                default:
                    break;
            }
        }
    }
    {
        const Value* enumValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Enum,
                               right,
                               ValueType_Type,
                               &enumValue,
                               &typeValue)) {
            const Value* fakeValue = typeValue->fakeValue;
            switch (fakeValue->type) {
                case ValueType_String: {
                    value->type = ValueType_String;
                    return TemLangStringCopy(
                      &value->string, &enumValue->enumValue.value, allocator);
                } break;
                case ValueType_Number: {
                    const StateFindArgs args = { .log = true,
                                                 .searchParent = true };
                    const Atom* atom = StateFindAtomConst(
                      state, &enumValue->enumValue.name, AtomType_Enum, args);
                    if (atom == NULL) {
                        return false;
                    }
                    size_t index = 0;
                    TemLangStringListFindIf(
                      &atom->enumDefinition.members,
                      (TemLangStringListFindFunc)TemLangStringsAreEqual,
                      &enumValue->enumValue.value,
                      NULL,
                      &index);
                    const Number number = { .type = NumberType_Unsigned,
                                            .u = index };
                    if (!numberInRange(&number,
                                       &fakeValue->rangedNumber.range)) {
                        numberNotInRangeError(
                          &number, &fakeValue->rangedNumber.range, allocator);
                        return false;
                    }
                    value->type = ValueType_Number;
                    value->rangedNumber.number = number;
                    value->rangedNumber.range = fakeValue->rangedNumber.range;
                    value->rangedNumber.hasRange = true;
                    return true;
                } break;
                default:
                    break;
            }
        }
    }
    {
        const Value* nullValue = NULL;
        const Value* typeValue = NULL;
        if (ValueTypesTryMatch(left,
                               ValueType_Null,
                               right,
                               ValueType_Type,
                               &nullValue,
                               &typeValue)) {
            switch (typeValue->fakeValue->type) {
                case ValueType_Flag:
                    value->type = ValueType_Flag;
                    memset(&value->flagValue.members,
                           0,
                           sizeof(value->flagValue.members));
                    value->flagValue.members.allocator = allocator;
                    return TemLangStringCopy(
                      &value->flagValue.name,
                      &typeValue->fakeValue->flagValue.name,
                      allocator);
                default:
                    break;
            }
        }
    }
    TemLangError("Unexpected types for add operator. Got '%s' and '%s'",
                 ValueTypeToString(left->type),
                 ValueTypeToString(right->type));
    return false;
}

static inline NamedValueList
StateToVariableList(const State* state, const Allocator* allocator)
{
    NamedValueList list = { 0 };
    list.allocator = allocator;
    NamedValue n = { 0 };
    for (size_t i = 0; i < state->atoms.used; ++i) {
        pAtom atom = &state->atoms.buffer[i];
        if (atom->type != AtomType_Variable) {
            continue;
        }
        if (TemLangStringCopy(&n.name, &atom->name, allocator) &&
            ValueCopy(&n.value, &atom->variable.value, allocator)) {
            NamedValueListAppend(&list, &n);
        }
    }
    NamedValueFree(&n);
    return list;
}

static inline bool
StructMemberToFakeValue(const StructMember* m,
                        const State* state,
                        const Allocator* allocator,
                        pNamedValue nv)
{
    Value tempValue = { 0 };
    if (m->isKeyword) {
        Token token = { .type = TokenType_Keyword, .keyword = m->keyword };
        Expression e = { 0 };
        if (!TokenToExpression(&token, &e, allocator) ||
            !EvaluateExpression(&e, state, &tempValue, allocator)) {
            return false;
        }
        ExpressionFree(&e);
    } else {
        Value newLeft = { 0 };
        newLeft.type = ValueType_String;
        Value nullValue = { .type = ValueType_Null };
        if (!TemLangStringCopy(&newLeft.string, &m->typeName, allocator) ||
            !EvaluateGetExpression(&newLeft,
                                   &nullValue,
                                   state,
                                   allocator,
                                   GetOperator_Type,
                                   &tempValue)) {
            return false;
        }
        ValueFree(&newLeft);
    }
    bool result = true;
    switch (m->quantity) {
        case 0: {
            pValue fakeValue = tempValue.fakeValue;
            ValueListValue list = { .allocator = allocator,
                                    .isArray = false,
                                    .values = { .buffer = NULL,
                                                .allocator = allocator,
                                                .size = 0,
                                                .used = 0 } };
            list.exampleValue = allocator->allocate(sizeof(Value));
            if (!ValueCopy(list.exampleValue, fakeValue, allocator)) {
                result = false;
            }
            fakeValue->type = ValueType_List;
            fakeValue->list = list;
        } break;
        case 1:
            break;
        default: {
            pValue fakeValue = tempValue.fakeValue;
            ValueListValue list = { .allocator = allocator,
                                    .isArray = true,
                                    .values = { .buffer = NULL,
                                                .allocator = allocator,
                                                .size = 0,
                                                .used = 0 } };
            list.exampleValue = allocator->allocate(sizeof(Value));
            if (!ValueCopy(list.exampleValue, fakeValue, allocator)) {
                result = false;
            }
            for (size_t i = 0; result && i < m->quantity; ++i) {
                result = ValueListAppend(&list.values, fakeValue);
            }
            fakeValue->type = ValueType_List;
            fakeValue->list = list;
        } break;
    }
    if (result) {
        result = TemLangStringCopy(&nv->name, &m->name, allocator) &&
                 ValueCopy(&nv->value, &tempValue, allocator);
    }
    ValueFree(&tempValue);
    return result;
}

static inline TemLangString
NamedValueListToString(const NamedValueList* list, const Allocator* allocator)
{
    LIST_TO_STRING((*list), s, NamedValueToString, allocator);
    return s;
}

static inline void
NamedValueFree(NamedValue* v)
{
    TemLangStringFree(&v->name);
    ValueFree(&v->value);
}

static inline bool
NamedValueCopy(NamedValue* dest,
               const NamedValue* src,
               const Allocator* allocator)
{
    NamedValueFree(dest);
    return TemLangStringCopy(&dest->name, &src->name, allocator) &&
           ValueCopy(&dest->value, &src->value, allocator);
}

static inline TemLangString
ExpressionListToString(const ExpressionList* list, const Allocator* allocator)
{
    LIST_TO_STRING((*list), s, ExpressionToString, allocator);
    return s;
}

static inline bool
CaptureVariables(State* dest,
                 const State* src,
                 const TemLangStringList* captures,
                 const InstructionSource source)
{
    bool result = true;
    for (size_t i = 0; result && i < captures->used; ++i) {
        const TemLangString* name = &captures->buffer[i];
        {
            if (AtomListFindIf(&dest->atoms,
                               (AtomListFindFunc)AtomNameEquals,
                               name,
                               NULL,
                               NULL)) {
                TemLangError("Variable '%s' was captured multiple times",
                             name->buffer);
                result = false;
                break;
            }
        }
        const Atom* atom = NULL;
        AtomListFindIf(
          &src->atoms, (AtomListFindFunc)AtomNameEquals, name, &atom, NULL);
        if (atom == NULL) {
            AtomNotFoundError(name);
            result = false;
            break;
        }

        if (atom->type != AtomType_Variable) {
            UnexpectedAtomTypeError(&source, AtomType_Variable, atom->type);
            result = false;
            break;
        }
        result = AtomListAppend(&dest->atoms, atom);
    }
    if (!result) {
        TemLangError("Failed to capture variables");
    }
    return result;
}

static inline bool
UpdateCapturedVariables(State* dest,
                        const State* src,
                        const TemLangStringList* captures,
                        const Allocator* allocator)
{
    for (size_t i = 0; i < captures->used; ++i) {
        const TemLangString* name = &captures->buffer[i];
        const Atom* newAtom = NULL;
        const Atom* oldAtom = NULL;
        AtomListFindIf(
          &src->atoms, (AtomListFindFunc)AtomNameEquals, name, &newAtom, NULL);
        AtomListFindIf(
          &dest->atoms, (AtomListFindFunc)AtomNameEquals, name, &oldAtom, NULL);
        if (newAtom == NULL || oldAtom == NULL) {
            TemLangError("Compiler error! Captured variable '%s' was removed.",
                         name->buffer);
            return false;
        }
        Atom* a = (Atom*)oldAtom;
        if (!AtomCopy(a, newAtom, allocator)) {
            return false;
        }
    }
    return true;
}

static inline void
FunctionDefinitionFree(FunctionDefinition* f)
{
    InstructionListFree(&f->instructions);
    if (f->type == FunctionType_Procedure) {
        TemLangStringListFree(&f->captures);
    } else {
        TemLangStringFree(&f->leftParameter);
        TemLangStringFree(&f->rightParameter);
    }
}

static inline bool
FunctionDefinitionCopy(FunctionDefinition* dest,
                       const FunctionDefinition* src,
                       const Allocator* allocator)
{
    FunctionDefinitionFree(dest);
    dest->type = src->type;
    if (!InstructionListCopy(
          &dest->instructions, &src->instructions, allocator)) {
        return false;
    }
    if (src->type == FunctionType_Procedure) {
        return TemLangStringListCopy(
          &dest->captures, &src->captures, allocator);
    } else {
        return TemLangStringCopy(
                 &dest->leftParameter, &src->leftParameter, allocator) &&
               TemLangStringCopy(
                 &dest->rightParameter, &src->rightParameter, allocator);
    }
}

static inline TemLangString
FunctionDefinitionToString(const FunctionDefinition* f,
                           const Allocator* allocator)
{
    TemLangString a = TemLangStringCreate("", allocator);
    {
        switch (f->type) {
            case FunctionType_Unary: {
                TemLangStringAppendFormat(
                  a, "\"parameter\": \"%s\",", f->leftParameter.buffer);
            } break;
            case FunctionType_Binary: {
                TemLangStringAppendFormat(
                  a,
                  "\"leftParameter\": \"%s\", \"rightParameter\": \"%s\",",
                  f->leftParameter.buffer,
                  f->rightParameter.buffer);
            } break;
            case FunctionType_Procedure: {
                TemLangString c =
                  TemLangStringListToString(&f->captures, allocator);
                TemLangStringAppendFormat(a, "\"captures\": %s,", c.buffer);
                TemLangStringFree(&c);
            } break;
            default:
                break;
        }
    }
    LIST_TO_STRING(f->instructions, b, InstructionToString, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"type\": \"%s\", %s \"instructions\": %s }",
                              FunctionTypeToString(f->type),
                              a.buffer,
                              b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

static inline bool
TokensToMatchBranch(const Token* tokens,
                    const size_t size,
                    const Allocator* allocator,
                    pMatchBranchList list)
{
    if (size == 0) {
        TemLangError("Match expression must have at least on branch");
        return false;
    }
    bool result = true;
    for (size_t i = 0; result && i < size - 1; i += 2) {
        MatchBranch branch = { 0 };
        result = TokenToExpression(&tokens[i], &branch.matcher, allocator);
        if (!result) {
            goto cleanup;
        }
        if (tokens[i + 1].type == TokenType_Scope) {
            branch.branch.type = MatchBranchType_Instructions;
            branch.branch.instructions =
              TokensToInstructions(&tokens[i + 1].tokens, allocator);
        } else {
            branch.branch.type = MatchBranchType_Expression;
            result = TokenToExpression(
              &tokens[i + 1], &branch.branch.expression, allocator);
        }
        result = MatchBranchListAppend(list, &branch);
    cleanup:
        MatchBranchFree(&branch);
    }
    return result;
}

static inline void
BranchFree(Branch* b)
{
    switch (b->type) {
        case MatchBranchType_Expression:
            ExpressionFree(&b->expression);
            break;
        case MatchBranchType_Instructions:
            InstructionListFree(&b->instructions);
            break;
        default:
            break;
    }
    memset(b, 0, sizeof(Branch));
}

static inline bool
BranchCopy(Branch* dest, const Branch* src, const Allocator* allocator)
{
    BranchFree(dest);
    dest->type = src->type;
    switch (src->type) {
        case MatchBranchType_Expression:
            return ExpressionCopy(
              &dest->expression, &src->expression, allocator);
        case MatchBranchType_Instructions:
            return InstructionListCopy(
              &dest->instructions, &src->instructions, allocator);
        default:
            return true;
    }
}

static inline TemLangString
BranchToString(const Branch* b, const Allocator* allocator)
{
    switch (b->type) {
        case MatchBranchType_Instructions:
            return InstructionListToString(&b->instructions, allocator);
        case MatchBranchType_Expression:
            return ExpressionToString(&b->expression, allocator);
        default:
            return TemLangStringCreate("null", allocator);
    }
}

static inline void
MatchBranchFree(MatchBranch* m)
{
    ExpressionFree(&m->matcher);
    BranchFree(&m->branch);
}

static inline bool
MatchBranchCopy(MatchBranch* dest,
                const MatchBranch* src,
                const Allocator* allocator)
{
    MatchBranchFree(dest);
    return ExpressionCopy(&dest->matcher, &src->matcher, allocator) &&
           BranchCopy(&dest->branch, &src->branch, allocator);
}

static inline TemLangString
MatchBranchToString(const MatchBranch* m, const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&m->matcher, allocator);
    TemLangString b = BranchToString(&m->branch, allocator);
    TemLangStringCreateFormat(
      s, allocator, "{ \"matcher\": %s, \"branch\": %s }", a.buffer, b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

static inline void
MatchExpressionFree(MatchExpression* m)
{
    ExpressionFree(&m->matcher);
    TemLangStringListFree(&m->captures);
    MatchBranchListFree(&m->branches);
    BranchFree(&m->defaultBranch);
}

static inline bool
MatchExpressionCopy(MatchExpression* dest,
                    const MatchExpression* src,
                    const Allocator* allocator)
{
    MatchExpressionFree(dest);
    return MatchBranchListCopy(&dest->branches, &src->branches, allocator) &&
           ExpressionCopy(&dest->matcher, &src->matcher, allocator) &&
           TemLangStringListCopy(&dest->captures, &src->captures, allocator) &&
           BranchCopy(&dest->defaultBranch, &src->defaultBranch, allocator);
}

static inline TemLangString
MatchBranchListToString(const MatchBranchList* list, const Allocator* allocator)
{
    LIST_TO_STRING((*list), s, MatchBranchToString, allocator);
    return s;
}

static inline TemLangString
MatchExpressionToString(const MatchExpression* m, const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&m->matcher, allocator);
    TemLangString b = TemLangStringListToString(&m->captures, allocator);
    TemLangString c = MatchBranchListToString(&m->branches, allocator);
    TemLangString d = BranchToString(&m->defaultBranch, allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"value\": %s, \"captures\": %s, \"branches\": %s, "
      "\"defaultBranch\": %s }",
      a.buffer,
      b.buffer,
      c.buffer,
      d.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    TemLangStringFree(&c);
    TemLangStringFree(&d);
    return s;
}

static inline void
MatchInstructionFree(MatchInstruction* m)
{
    MatchExpressionFree(&m->expression);
    TemLangStringListFree(&m->captures);
}

static inline bool
MatchInstructionCopy(MatchInstruction* dest,
                     const MatchInstruction* src,
                     const Allocator* allocator)
{
    MatchInstructionFree(dest);
    return MatchExpressionCopy(
             &dest->expression, &src->expression, allocator) &&
           TemLangStringListCopy(&dest->captures, &src->captures, allocator);
}

static inline TemLangString
MatchInstructionToString(const MatchInstruction* m, const Allocator* allocator)
{
    TemLangString a = MatchExpressionToString(&m->expression, allocator);
    TemLangString b = TemLangStringListToString(&m->captures, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"expression\": %s, \"captures\": %s }",
                              a.buffer,
                              b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

static inline bool
EvaluateBranchExpression(const Branch* branch,
                         State* state,
                         pValue value,
                         const Allocator* allocator)
{
    switch (branch->type) {
        case MatchBranchType_Instructions: {
            bool result = true;
            for (size_t j = 0; result && value->type == ValueType_Null &&
                               j < branch->instructions.used;
                 ++j) {
                result = StateProcessInstruction(
                  state, &branch->instructions.buffer[j], allocator, value);
                if (!result) {
                    InstructionError(&branch->instructions.buffer[j]);
                }
            }
            return result;
        } break;
        case MatchBranchType_Expression:
            return EvaluateExpression(
              &branch->expression, state, value, allocator);
            break;
        default:
            break;
    }
    return true;
}

static inline bool
EvaluateMatchExpression(const MatchExpression* m,
                        State* state,
                        pValue value,
                        const Allocator* allocator)
{
    bool result = false;
    bool done = false;
    Value targetValue = { 0 };
    if (!EvaluateExpression(&m->matcher, state, &targetValue, allocator)) {
        return false;
    }
    switch (targetValue.type) {
        case ValueType_Variant: {
            for (size_t i = 0; !done && i < m->branches.used; ++i) {
                const MatchBranch* branch = &m->branches.buffer[i];
                if (branch->matcher.type != ExpressionType_UnaryVariable) {
                    TemLangError(
                      "Expected variable for variant match. Got '%s'",
                      ExpressionTypeToString(branch->matcher.type));
                    result = false;
                    done = true;
                    break;
                }
                done =
                  TemLangStringCompare(&targetValue.variantValue.memberName,
                                       &branch->matcher.identifier) ==
                  ComparisonOperator_EqualTo;
                if (!done) {
                    continue;
                }
                TemLangString varName = { 0 };
                TemLangStringCopy(
                  &varName, &targetValue.variantValue.memberName, allocator);
                if (!StateAddValue(
                      state, &varName, targetValue.variantValue.value)) {
                    TemLangError(
                      "Cannot add variable '%s' because it already exists. "
                      "Must execute this match expression in another scope",
                      varName.buffer);
                    result = false;
                    goto cleanupVariantMatch;
                }
                result = EvaluateBranchExpression(
                  &branch->branch, state, value, allocator);
                if (!AtomListRemoveIf(&state->atoms,
                                      (AtomListRemoveIfFunc)AtomNameEquals,
                                      &varName,
                                      allocator)) {
                    TemLangError("Failed to remove atom '%s' after calling "
                                 "variant match branch",
                                 varName.buffer);
                    result = false;
                }
            cleanupVariantMatch:
                TemLangStringFree(&varName);
            }
        } break;
        default: {
            for (size_t i = 0; !done && i < m->branches.used; ++i) {
                const MatchBranch* branch = &m->branches.buffer[i];
                Value b = { 0 };
                result =
                  EvaluateExpression(&branch->matcher, state, &b, allocator);
                if (!result) {
                    continue;
                }
                done = ValuesMatch(&targetValue, &b);
                ValueFree(&b);
                if (!done) {
                    continue;
                }
                result = EvaluateBranchExpression(
                  &branch->branch, state, value, allocator);
            }
        } break;
    }
    if (!done) {
        result =
          EvaluateBranchExpression(&m->defaultBranch, state, value, allocator);
    }
    ValueFree(&targetValue);
    return result;
}

static inline bool
ExpressionMatchExpressionCopy(Expression* dest,
                              const Expression* src,
                              const Allocator* allocator)
{
    dest->matchExpression = allocator->allocate(sizeof(MatchExpression));
    dest->matchAllocator = allocator;
    return MatchExpressionCopy(
      dest->matchExpression, src->matchExpression, allocator);
}

static inline bool
TokenToMatchExpression(const Token* token,
                       pExpression e,
                       const Allocator* allocator)
{
    const Token* tokens = token->tokens.buffer;
    const size_t size = token->tokens.used;
    if (size < 2) {
        TemLangError("Match expressions must have at least one branch");
        return false;
    }
    if (size % 2 != 0) {
        TemLangError("Match expressions must have a default branch");
        return false;
    }
    e->type = ExpressionType_UnaryMatch;
    e->matchAllocator = allocator;
    e->matchExpression = allocator->allocate(sizeof(MatchExpression));
    e->matchExpression->defaultBranch.type = MatchBranchType_Expression;
    e->matchExpression->branches.allocator = allocator;
    return TokenToExpression(
             &tokens[0], &e->matchExpression->matcher, allocator) &&
           TokensToMatchBranch(tokens + 1,
                               size - 1UL,
                               allocator,
                               &e->matchExpression->branches) &&
           TokenToExpression(&tokens[size - 1UL],
                             &e->matchExpression->defaultBranch.expression,
                             allocator);
}

static inline bool
NamedValueInStructDefinition(const NamedValue* nv,
                             const State* state,
                             const Allocator* allocator,
                             const StructDefinition* d,
                             const bool log)
{
    for (size_t i = 0; i < d->members.used; ++i) {
        const StructMember* m = &d->members.buffer[i];
        if (TemLangStringCompare(&m->name, &nv->name) !=
            ComparisonOperator_EqualTo) {
            continue;
        }
        StructMember temp = { 0 };
        const bool result =
          NamedValueToStructMember(nv, state, &temp, allocator, log) &&
          m->isKeyword == temp.isKeyword &&
          (temp.isKeyword
             ? m->keyword == temp.keyword
             : (TemLangStringCompare(&m->typeName, &temp.typeName) ==
                ComparisonOperator_EqualTo));
        StructMemberFree(&temp);
        return result;
    }
    return false;
}

static inline bool
NamedValueToStructMember(const NamedValue* nv,
                         const State* state,
                         pStructMember m,
                         const Allocator* allocator,
                         const bool log)
{
    bool result = false;
    bool checkParent = true;
    m->quantity = 1;
    switch (nv->value.type) {
        case ValueType_Null: {
            m->isKeyword = true;
            m->keyword = Keyword_Null;
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_Number: {
            m->isKeyword = true;
            if (nv->value.rangedNumber.hasRange) {
                m->keyword =
                  CTypeToKeyword(RangeToCType(&nv->value.rangedNumber.range));
            } else {
                m->keyword =
                  CTypeToKeyword(NumberToCType(&nv->value.rangedNumber.number));
            }
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_Boolean: {
            m->isKeyword = true;
            m->keyword = Keyword_bool;
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_External: {
            m->isKeyword = true;
            m->keyword = Keyword_external;
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_String: {
            m->isKeyword = true;
            m->keyword = Keyword_string;
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_Flag: {
            m->isKeyword = false;
            result = TemLangStringCopy(&m->name, &nv->name, allocator) &&
                     TemLangStringCopy(
                       &m->typeName, &nv->value.flagValue.name, allocator);
        } break;
        case ValueType_Enum: {
            m->isKeyword = false;
            result = TemLangStringCopy(&m->name, &nv->name, allocator) &&
                     TemLangStringCopy(
                       &m->typeName, &nv->value.enumValue.name, allocator);
        } break;
        case ValueType_Variant: {
            m->isKeyword = false;
            result = TemLangStringCopy(&m->name, &nv->name, allocator) &&
                     TemLangStringCopy(
                       &m->typeName, &nv->value.variantValue.name, allocator);
        } break;
        case ValueType_List: {
            StructMember m2 = { 0 };
            NamedValue new_nv = { 0 };
            if (!ValueCopy(
                  &new_nv.value, nv->value.list.exampleValue, allocator) ||
                !TemLangStringCopy(&new_nv.name, &nv->name, allocator) ||
                !NamedValueToStructMember(
                  &new_nv, state, &m2, allocator, log)) {
                result = false;
                break;
            }

            if (m2.isKeyword) {
                m->isKeyword = true;
                m->keyword = m2.keyword;
                if (nv->value.list.isArray) {
                    m->quantity = nv->value.list.values.used;
                } else {
                    m->quantity = 0;
                }
            } else {
                TemLangString s = StructMemberToTypeName(&m2, allocator);
                m->isKeyword = false;
                if (nv->value.list.isArray) {
                    TemLangStringCopy(&m->typeName, &s, allocator);
                    m->quantity = nv->value.list.values.used;
                } else {
                    TemLangStringCreateFormat(t, allocator, "%s", s.buffer);
                    m->typeName = t;
                    m->quantity = 0;
                }
                TemLangStringFree(&s);
            }

            NamedValueFree(&new_nv);
            StructMemberFree(&m2);
            result = TemLangStringCopy(&m->name, &nv->name, allocator);
        } break;
        case ValueType_Struct: {
            TemLangStringList matches = {
                .allocator = allocator, .buffer = NULL, .used = 0UL, .size = 0UL
            };
            for (size_t i = 0; i < state->atoms.used; ++i) {
                const Atom* atom = &state->atoms.buffer[i];
                if (atom->type != AtomType_Struct) {
                    continue;
                }
                const StructDefinition* d = &atom->structDefinition;
                if (d->isVariant) {
                    continue;
                }
                if (d->members.used != nv->value.structValues->used) {
                    continue;
                }
                bool allMatch = true;
                for (size_t j = 0; j < nv->value.structValues->used; ++j) {
                    if (!NamedValueInStructDefinition(
                          &nv->value.structValues->buffer[j],
                          state,
                          allocator,
                          d,
                          log)) {
                        allMatch = false;
                        break;
                    }
                }
                if (allMatch) {
                    TemLangStringListAppend(&matches, &atom->name);
                }
            }
            switch (matches.used) {
                case 0:
                    if (log && state->parent == NULL) {
                        TemLangError(
                          "No match found for potential struct member '%s'",
                          nv->name.buffer);
                    }
                    result = false;
                    break;
                case 1:
                    result =
                      TemLangStringCopy(&m->name, &nv->name, allocator) &&
                      TemLangStringCopy(
                        &m->typeName, &matches.buffer[0], allocator);
                    break;
                default:
                    checkParent = false;
                    if (log && state->parent == NULL) {
                        TemLangString listS =
                          TemLangStringListToString(&matches, allocator);
                        TemLangError("Multiple matches found for potential "
                                     "struct member '%s': %s",
                                     nv->name.buffer,
                                     listS.buffer);
                        TemLangStringFree(&listS);
                    }
                    result = false;
                    break;
            }
            TemLangStringListFree(&matches);
        } break;
        case ValueType_Type: {
            NamedValue f = { .name = nv->name, .value = *nv->value.fakeValue };
            return NamedValueToStructMember(&f, state, m, allocator, log);
        } break;
        default:
            result = false;
            if (log) {
                TemLangError("Structs cannot contain value type '%s'",
                             ValueTypeToString(nv->value.type));
            }
            break;
    }

    if (checkParent && !result && state->parent != NULL) {
        return NamedValueToStructMember(nv, state->parent, m, allocator, log);
    }
    return result;
}

CREATE_VALUE_INDEX(const)
CREATE_VALUE_INDEX()

static inline TemLangString
VariableToTemLang(const TemLangString* name,
                  const Variable* variable,
                  const Allocator* allocator);

static inline TemLangString
ValueToTemLang(const Value* value, const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator };
    switch (value->type) {
        case ValueType_Null:
            s = TemLangStringCreate("null", allocator);
            break;
        case ValueType_Number:
            s = NumberToString(&value->rangedNumber.number, allocator);
            break;
        case ValueType_String:
            TemLangStringAppendFormat(s, "\"%s\"", value->string.buffer);
            break;
        case ValueType_Boolean:
            s = TemLangStringCreate(value->b ? "true" : "false", allocator);
            break;
        case ValueType_Enum:
            TemLangStringAppendFormat(s,
                                      "#%s + \"%s\"",
                                      value->enumValue.name.buffer,
                                      value->enumValue.value.buffer);
            break;
        case ValueType_Flag:
            TemLangStringAppendFormat(
              s, "(null + #%s)", value->flagValue.name.buffer);
            for (size_t i = 0; i < value->flagValue.members.used; ++i) {
                TemLangStringAppendFormat(
                  s,
                  " +  (#%s + \"%s\")",
                  value->flagValue.name.buffer,
                  value->flagValue.members.buffer[i].buffer);
            }
            break;
        case ValueType_Struct:
            TemLangStringAppendChars(&s, "{{ ");
            for (size_t i = 0; i < value->structValues->used; ++i) {
                const NamedValue* nv = &value->structValues->buffer[i];
                Variable v = { .type = VariableType_Immutable,
                               .value = nv->value };
                TemLangString temp =
                  VariableToTemLang(&nv->name, &v, allocator);
                TemLangStringAppend(&s, &temp);
                TemLangStringFree(&temp);
            }
            TemLangStringAppendChars(&s, " }}");
            break;
        case ValueType_Variant: {
            TemLangString temp =
              ValueToTemLang(value->variantValue.value, allocator);
            TemLangStringAppendFormat(s,
                                      "{{ let %s %s }} + #%s",
                                      value->variantValue.memberName.buffer,
                                      temp.buffer,
                                      value->variantValue.name.buffer);
            TemLangStringFree(&temp);
        } break;
        case ValueType_List:
            if (value->list.isArray) {
                TemLangStringAppendChars(&s, "[| ");
            } else {
                TemLangStringAppendChars(&s, "[ ");
            }
            for (size_t i = 0; i < value->list.values.used; ++i) {
                TemLangString temp =
                  ValueToTemLang(&value->list.values.buffer[i], allocator);
                TemLangStringAppendFormat(s, "%s ", temp.buffer);
                TemLangStringFree(&temp);
            }
            if (value->list.isArray) {
                TemLangStringAppendChars(&s, " |]");
            } else {
                TemLangStringAppendChars(&s, " ]");
            }
            break;
        default:
            break;
    }
    return s;
}

static inline TemLangString
VariableToTemLang(const TemLangString* name,
                  const Variable* variable,
                  const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator };

    const char* c = NULL;
    switch (variable->type) {
        case VariableType_Constant:
            c = "constant";
            break;
        case VariableType_Mutable:
            c = "mutableLet";
            break;
        default:
            c = "let";
            break;
    }
    TemLangString temp = ValueToSimpleString(&variable->value, allocator);
    TemLangStringAppendFormat(s, "%s %s %s\n", c, name->buffer, temp.buffer);
    TemLangStringFree(&temp);

    return s;
}

static inline TemLangString
StateToTemLang(const State* state, const Allocator* allocator)
{
    TemLangString s = { .allocator = allocator };
    for (size_t i = 0; i < state->atoms.used; ++i) {
        const Atom* atom = &state->atoms.buffer[i];
        if (atom == NULL || atom->type != AtomType_Variable) {
            continue;
        }
        TemLangString s1 =
          VariableToTemLang(&atom->name, &atom->variable, allocator);
        TemLangStringAppend(&s, &s1);
        TemLangStringFree(&s1);
    }
    return s;
}