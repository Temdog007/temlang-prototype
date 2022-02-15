#pragma once

#include "ChangeFlagType.h"
#include "EnumDefinition.h"
#include "Expression.h"
#include "InstructionType.h"
#include "List.h"
#include "ListModifyType.h"
#include "MatchExpression.h"
#include "StructDefinition.h"
#include "TemLangString.h"

static inline bool
InstructionCreatesAtom(const InstructionType type)
{
    switch (type) {
        case InstructionType_CreateVariable:
        case InstructionType_DefineEnum:
        case InstructionType_DefineRange:
        case InstructionType_DefineStruct:
        case InstructionType_DefineFunction:
        case InstructionType_InlineVariable:
            return true;
        default:
            return false;
    }
}

static inline TemLangString
ExpressionListToString(const ExpressionList*, const Allocator*);

static inline bool
InstructionTypeCanBeExecuted(const InstructionType t)
{
    switch (t) {
        case InstructionType_InlineC:
        case InstructionType_InlineCFunction:
        case InstructionType_InlineCFunctionReturnStruct:
        case InstructionType_InlineCHeaders:
        case InstructionType_InlineCFile:
        case InstructionType_NoCleanup:
            return false;
        default:
            return true;
    }
}

typedef struct CreateVariableInstruction
{
    TemLangString name;
    Expression value;
    VariableType type;
} CreateVariableInstruction, *pCreateVariableInstruction;

static inline void
CreateVariableInstructionFree(CreateVariableInstruction* i)
{
    TemLangStringFree(&i->name);
    ExpressionFree(&i->value);
    memset(i, 0, sizeof(CreateVariableInstruction));
}

static inline bool
CreateVariableInstructionCopy(CreateVariableInstruction* dest,
                              const CreateVariableInstruction* src,
                              const Allocator* allocator)
{
    dest->type = src->type;
    return TemLangStringCopy(&dest->name, &src->name, allocator) &&
           ExpressionCopy(&dest->value, &src->value, allocator);
}

static inline TemLangString
CreateVariableInstructionToString(const CreateVariableInstruction* c,
                                  const Allocator* allocator)
{
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"name\": \"%s\", \"type\": \"%s\", \"value\":",
      c->name.buffer,
      VariableTypeToString(c->type));
    TemLangString n = ExpressionToString(&c->value, allocator);
    TemLangStringAppend(&s, &n);
    TemLangStringAppendChar(&s, '}');
    TemLangStringFree(&n);
    return s;
}

typedef struct UpdateVariableInstruction
{
    Expression target;
    Expression value;
} UpdateVariableInstruction, *pUpdateVariableInstruction;

static inline void
UpdateVariableInstructionFree(UpdateVariableInstruction* i)
{
    ExpressionFree(&i->target);
    ExpressionFree(&i->value);
    memset(i, 0, sizeof(UpdateVariableInstruction));
}

static inline bool
UpdateVariableInstructionCopy(UpdateVariableInstruction* dest,
                              const UpdateVariableInstruction* src,
                              const Allocator* allocator)
{
    UpdateVariableInstructionFree(dest);
    return ExpressionCopy(&dest->target, &src->target, allocator) &&
           ExpressionCopy(&dest->value, &src->value, allocator);
}

static inline TemLangString
UpdateVariableInstructionToString(const UpdateVariableInstruction* y,
                                  const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&y->target, allocator);
    TemLangString b = ExpressionToString(&y->value, allocator);
    TemLangStringCreateFormat(
      s, allocator, "{ \"target\": %s, \"value\": %s }", a.buffer, b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

typedef struct DefineRangeInstruction
{
    Expression min;
    Expression max;
    TemLangString name;
} DefineRangeInstruction, *pDefineRangeInstruction;

static inline void
DefineRangeInstructionFree(DefineRangeInstruction* i)
{
    ExpressionFree(&i->min);
    ExpressionFree(&i->max);
    TemLangStringFree(&i->name);
}

static inline bool
DefineRangeInstructionCopy(DefineRangeInstruction* dest,
                           const DefineRangeInstruction* src,
                           const Allocator* allocator)
{
    return ExpressionCopy(&dest->min, &src->min, allocator) &&
           ExpressionCopy(&dest->max, &src->max, allocator) &&
           TemLangStringCopy(&dest->name, &src->name, allocator);
}

static inline TemLangString
DefineRangeInstructionToString(const DefineRangeInstruction* i,
                               const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&i->min, allocator);
    TemLangString b = ExpressionToString(&i->max, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"min\": %s, \"max\": %s, \"name\": %s }",
                              a.buffer,
                              b.buffer,
                              i->name.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

typedef struct DefineEnumInstruction
{
    TemLangString name;
    EnumDefinition definition;
} DefineEnumInstruction, *pDefineEnumInstruction;

static inline void
DefineEnumInstructionFree(DefineEnumInstruction* dei)
{
    TemLangStringFree(&dei->name);
    EnumDefinitionFree(&dei->definition);
}

static inline bool
DefineEnumInstructionCopy(DefineEnumInstruction* dest,
                          const DefineEnumInstruction* src,
                          const Allocator* allocator)
{
    return EnumDefinitionCopy(&dest->definition, &src->definition, allocator) &&
           TemLangStringCopy(&dest->name, &src->name, allocator);
}

static inline TemLangString
DefineEnumInstructionToString(const DefineEnumInstruction* dei,
                              const Allocator* allocator)
{
    TemLangString a = EnumDefinitionToString(&dei->definition, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"name\": \"%s\",  \"members\": %s }",
                              dei->name.buffer,
                              a.buffer);
    TemLangStringFree(&a);
    return s;
}

typedef struct ChangeFlagInstruction
{
    Expression target;
    TemLangString member;
    ChangeFlagType flag;
} ChangeFlagInstruction, *pChangeFlagInstruction;

static inline void
ChangeFlagInstructionFree(ChangeFlagInstruction* dei)
{
    TemLangStringFree(&dei->member);
    ExpressionFree(&dei->target);
}

static inline bool
ChangeFlagInstructionCopy(ChangeFlagInstruction* dest,
                          const ChangeFlagInstruction* src,
                          const Allocator* allocator)
{
    dest->flag = src->flag;
    return ExpressionCopy(&dest->target, &src->target, allocator) &&
           TemLangStringCopy(&dest->member, &src->member, allocator);
}

static inline TemLangString
ChangeFlagInstructionToString(const ChangeFlagInstruction* dest,
                              const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&dest->target, allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"type\": \"%s\", \"target\": %s,  \"member\": \"%s\" }",
      ChangeFlagTypeToString(dest->flag),
      a.buffer,
      dest->member.buffer);
    TemLangStringFree(&a);
    return s;
}

typedef struct SetAllFlagInstruction
{
    Expression target;
    bool clear;
} SetAllFlagInstruction, *pSetAllFlagInstruction;

static inline void
SetAllFlagInstructionFree(SetAllFlagInstruction* dei)
{
    ExpressionFree(&dei->target);
}

static inline bool
SetAllFlagInstructionCopy(SetAllFlagInstruction* dest,
                          const SetAllFlagInstruction* src,
                          const Allocator* allocator)
{
    dest->clear = src->clear;
    return ExpressionCopy(&dest->target, &src->target, allocator);
}

static inline TemLangString
SetAllFlagInstructionToString(const SetAllFlagInstruction* dest,
                              const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&dest->target, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"clear\": %s, \"target\": %s }",
                              dest->clear ? "true" : "false",
                              a.buffer);
    TemLangStringFree(&a);
    return s;
}

typedef struct DefineStructInstruction
{
    TemLangString name;
    StructDefinition definition;
} DefineStructInstruction, *pDefineStructInstruction;

static inline void
DefineStructInstructionFree(DefineStructInstruction* i)
{
    TemLangStringFree(&i->name);
    StructDefinitionFree(&i->definition);
}

static inline bool
DefineStructInstructionCopy(DefineStructInstruction* dest,
                            const DefineStructInstruction* src,
                            const Allocator* allocator)
{
    DefineStructInstructionFree(dest);
    return TemLangStringCopy(&dest->name, &src->name, allocator) &&
           StructDefinitionCopy(&dest->definition, &src->definition, allocator);
}

static inline TemLangString
DefineStructInstructionToString(const DefineStructInstruction* d,
                                const Allocator* allocator)
{
    TemLangString a = StructDefinitionToString(&d->definition, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"name\": \"%s\", \"definition\": %s }",
                              d->name.buffer,
                              a.buffer);
    TemLangStringFree(&a);
    return s;
}

typedef struct ListModifyInstruction
{
    Expression list;
    Expression newValue[2];
    ListModifyType type;
} ListModifyInstruction, *pListModifyInstruction;

static inline void
ListModifyInstructionFree(ListModifyInstruction* i)
{
    ExpressionFree(&i->list);
    ExpressionFree(&i->newValue[0]);
    ExpressionFree(&i->newValue[1]);
}

static inline bool
ListModifyInstructionCopy(ListModifyInstruction* dest,
                          const ListModifyInstruction* src,
                          const Allocator* allocator)
{
    ListModifyInstructionFree(dest);
    dest->type = src->type;
    return ExpressionCopy(&dest->list, &src->list, allocator) &&
           ExpressionCopy(&dest->newValue[0], &src->newValue[0], allocator) &&
           ExpressionCopy(&dest->newValue[1], &src->newValue[1], allocator);
}

static inline TemLangString
ListModifyInstructionToString(const ListModifyInstruction* list,
                              const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&list->list, allocator);
    TemLangString b = ExpressionToString(&list->newValue[0], allocator);
    TemLangString c = ExpressionToString(&list->newValue[1], allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"type\": \"%s\", \"list\": %s, \"value\": [ %s, %s ] }",
      ListModifyTypeToString(list->type),
      a.buffer,
      b.buffer,
      c.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    TemLangStringFree(&c);
    return s;
}

typedef struct CaptureInstruction
{
    Expression target;
    TemLangStringList captures;
    InstructionList instructions;
} CaptureInstruction, *pCaptureInstruction;

static inline void
CaptureInstructionFree(CaptureInstruction* i)
{
    ExpressionFree(&i->target);
    TemLangStringListFree(&i->captures);
    InstructionListFree(&i->instructions);
    memset(i, 0, sizeof(CaptureInstruction));
}

static inline bool
CaptureInstructionCopy(CaptureInstruction* dest,
                       const CaptureInstruction* src,
                       const Allocator* allocator)
{
    CaptureInstructionFree(dest);
    return InstructionListCopy(
             &dest->instructions, &src->instructions, allocator) &&
           TemLangStringListCopy(&dest->captures, &src->captures, allocator) &&
           ExpressionCopy(&dest->target, &src->target, allocator);
}

static inline TemLangString
CaptureInstructionToString(const CaptureInstruction* i,
                           const Allocator* allocator)
{
    TemLangString a = ExpressionToString(&i->target, allocator);
    TemLangString b = TemLangStringListToString(&i->captures, allocator);
    TemLangString c = InstructionListToString(&i->instructions, allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"target\": %s, \"captures\": %s, \"instructions\": %s }",
      a.buffer,
      b.buffer,
      c.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    TemLangStringFree(&c);
    return s;
}

typedef struct MatchInstruction
{
    MatchExpression expression;
    TemLangStringList captures;
} MatchInstruction, *pMatchInstruction;

static inline void
MatchInstructionFree(MatchInstruction* m);

static inline bool
MatchInstructionCopy(MatchInstruction* m,
                     const MatchInstruction*,
                     const Allocator*);

static inline TemLangString
MatchInstructionToString(const MatchInstruction* m, const Allocator*);

typedef struct Instruction
{
    InstructionType type;
    InstructionSource source;
    union
    {
        CreateVariableInstruction createVariable;
        UpdateVariableInstruction updateVariable;
        DefineRangeInstruction defineRange;
        DefineEnumInstruction defineEnum;
        ChangeFlagInstruction changeFlag;
        SetAllFlagInstruction setAllFlag;
        DefineStructInstruction defineStruct;
        ListModifyInstruction listModify;
        Expression expression;
        ExpressionList printExpressions;
        CaptureInstruction captureInstruction;
        MatchInstruction matchInstruction;
        TemLangString procedureName;
        TemLangString verifyName;
        InstructionList instructions;
        struct
        {
            TemLangString functionName;
            FunctionDefinition functionDefinition;
        };
        struct
        {
            Expression inlinedFunctionName;
            Expression inlinedFunctionArgs;
            InstructionList inlinedFunctionInstructions;
        };
        struct
        {
            TemLangString definitionName;
            StructMember definitionType;
            VariableType definitionVariableType;
        };
        struct
        {
            Expression fromContainer;
            TemLangString toContainer;
            bool toArray;
        };
        struct
        {
            TemLangString dataName;
            Expression dataFile;
            bool dataIsBinary;
        };
        struct
        {
            Expression ifCondition;
            Expression ifResult;
        };
        struct
        {
            TemLangString formatName;
            Expression formatArgs;
            TemLangString formatString;
        };
        struct
        {
            NumberRound numberRound;
            Expression numberRoundTarget;
            TemLangString numberRoundName;
            StructMember numberRoundMember;
        };
    };
} Instruction, *pInstruction;

static inline void
InstructionFree(Instruction* i)
{
    InstructionSourceFree(&i->source);
    switch (i->type) {
        case InstructionType_CreateVariable:
            CreateVariableInstructionFree(&i->createVariable);
            break;
        case InstructionType_UpdateVariable:
            UpdateVariableInstructionFree(&i->updateVariable);
            break;
        case InstructionType_DefineRange:
            DefineRangeInstructionFree(&i->defineRange);
            break;
        case InstructionType_DefineEnum:
            DefineEnumInstructionFree(&i->defineEnum);
            break;
        case InstructionType_ChangeFlag:
            ChangeFlagInstructionFree(&i->changeFlag);
            break;
        case InstructionType_SetAllFlag:
            SetAllFlagInstructionFree(&i->setAllFlag);
            break;
        case InstructionType_DefineStruct:
            DefineStructInstructionFree(&i->defineStruct);
            break;
        case InstructionType_ListModify:
            ListModifyInstructionFree(&i->listModify);
            break;
        case InstructionType_Verify:
            TemLangStringFree(&i->verifyName);
            break;
        case InstructionType_Return:
        case InstructionType_Inline:
        case InstructionType_InlineC:
        case InstructionType_InlineFile:
        case InstructionType_InlineCFile:
            ExpressionFree(&i->expression);
            break;
        case InstructionType_IfReturn:
            ExpressionFree(&i->ifCondition);
            ExpressionFree(&i->ifResult);
            break;
        case InstructionType_InlineData:
            ExpressionFree(&i->dataFile);
            TemLangStringFree(&i->dataName);
            break;
        case InstructionType_InlineCFunction:
        case InstructionType_InlineCFunctionReturnStruct:
            ExpressionFree(&i->inlinedFunctionName);
            ExpressionFree(&i->inlinedFunctionArgs);
            InstructionListFree(&i->inlinedFunctionInstructions);
            break;
        case InstructionType_While:
        case InstructionType_Until:
        case InstructionType_Iterate:
            CaptureInstructionFree(&i->captureInstruction);
            break;
        case InstructionType_Print:
        case InstructionType_Error:
            ExpressionListFree(&i->printExpressions);
            break;
        case InstructionType_DefineFunction:
            TemLangStringFree(&i->functionName);
            FunctionDefinitionFree(&i->functionDefinition);
            break;
        case InstructionType_Run:
            TemLangStringFree(&i->procedureName);
            break;
        case InstructionType_Match:
            MatchInstructionFree(&i->matchInstruction);
            break;
        case InstructionType_InlineVariable:
            TemLangStringFree(&i->definitionName);
            StructMemberFree(&i->definitionType);
            break;
        case InstructionType_ConvertContainer:
            ExpressionFree(&i->fromContainer);
            TemLangStringFree(&i->toContainer);
            break;
        case InstructionType_NoCompile:
        case InstructionType_NoCleanup:
            InstructionListFree(&i->instructions);
            break;
        case InstructionType_Format:
            TemLangStringFree(&i->formatName);
            ExpressionFree(&i->formatArgs);
            TemLangStringFree(&i->formatString);
            break;
        case InstructionType_NumberRound:
            TemLangStringFree(&i->numberRoundName);
            ExpressionFree(&i->numberRoundTarget);
            StructMemberFree(&i->numberRoundMember);
            break;
        default:
            break;
    }
    memset(i, 0, sizeof(Instruction));
}

static inline bool
InstructionCopy(Instruction* dest,
                const Instruction* src,
                const Allocator* allocator)
{
    InstructionFree(dest);
    dest->type = src->type;
    if (!InstructionSourceCopy(&dest->source, &src->source, allocator)) {
        return false;
    }
    switch (src->type) {
        case InstructionType_InlineCHeaders:
            return true;
        case InstructionType_CreateVariable:
            return CreateVariableInstructionCopy(
              &dest->createVariable, &src->createVariable, allocator);
        case InstructionType_UpdateVariable:
            return UpdateVariableInstructionCopy(
              &dest->updateVariable, &src->updateVariable, allocator);
        case InstructionType_DefineRange:
            return DefineRangeInstructionCopy(
              &dest->defineRange, &src->defineRange, allocator);
        case InstructionType_DefineEnum:
            return DefineEnumInstructionCopy(
              &dest->defineEnum, &src->defineEnum, allocator);
        case InstructionType_ChangeFlag:
            return ChangeFlagInstructionCopy(
              &dest->changeFlag, &src->changeFlag, allocator);
        case InstructionType_SetAllFlag:
            return SetAllFlagInstructionCopy(
              &dest->setAllFlag, &src->setAllFlag, allocator);
        case InstructionType_DefineStruct:
            return DefineStructInstructionCopy(
              &dest->defineStruct, &src->defineStruct, allocator);
        case InstructionType_ListModify:
            return ListModifyInstructionCopy(
              &dest->listModify, &src->listModify, allocator);
        case InstructionType_Verify:
            return TemLangStringCopy(
              &dest->verifyName, &src->verifyName, allocator);
        case InstructionType_Return:
        case InstructionType_Inline:
        case InstructionType_InlineC:
        case InstructionType_InlineFile:
        case InstructionType_InlineCFile:
            return ExpressionCopy(
              &dest->expression, &src->expression, allocator);
        case InstructionType_IfReturn:
            return ExpressionCopy(
                     &dest->ifCondition, &src->ifCondition, allocator) &&
                   ExpressionCopy(&dest->ifResult, &src->ifResult, allocator);
        case InstructionType_InlineData:
            dest->dataIsBinary = src->dataIsBinary;
            return TemLangStringCopy(
                     &dest->dataName, &src->dataName, allocator) &&
                   ExpressionCopy(&dest->dataFile, &src->dataFile, allocator);
        case InstructionType_InlineCFunction:
        case InstructionType_InlineCFunctionReturnStruct:
            return InstructionListCopy(&dest->inlinedFunctionInstructions,
                                       &src->inlinedFunctionInstructions,
                                       allocator) &&
                   ExpressionCopy(&dest->inlinedFunctionName,
                                  &src->inlinedFunctionName,
                                  allocator) &&
                   ExpressionCopy(&dest->inlinedFunctionArgs,
                                  &src->inlinedFunctionArgs,
                                  allocator);
        case InstructionType_While:
        case InstructionType_Until:
        case InstructionType_Iterate:
            return CaptureInstructionCopy(
              &dest->captureInstruction, &src->captureInstruction, allocator);
        case InstructionType_Print:
        case InstructionType_Error:
            return ExpressionListCopy(
              &dest->printExpressions, &src->printExpressions, allocator);
        case InstructionType_DefineFunction:
            return TemLangStringCopy(
                     &dest->functionName, &src->functionName, allocator) &&
                   FunctionDefinitionCopy(&dest->functionDefinition,
                                          &src->functionDefinition,
                                          allocator);
        case InstructionType_Run:
            return TemLangStringCopy(
              &dest->procedureName, &src->procedureName, allocator);
        case InstructionType_Match:
            return MatchInstructionCopy(
              &dest->matchInstruction, &src->matchInstruction, allocator);
        case InstructionType_InlineVariable:
            dest->definitionVariableType = src->definitionVariableType;
            return TemLangStringCopy(
                     &dest->definitionName, &src->definitionName, allocator) &&
                   StructMemberCopy(
                     &dest->definitionType, &src->definitionType, allocator);
        case InstructionType_ConvertContainer:
            dest->toArray = src->toArray;
            return ExpressionCopy(
                     &dest->fromContainer, &src->fromContainer, allocator) &&
                   TemLangStringCopy(
                     &dest->toContainer, &src->toContainer, allocator);
        case InstructionType_NoCompile:
        case InstructionType_NoCleanup:
            return InstructionListCopy(
              &dest->instructions, &src->instructions, allocator);
        case InstructionType_Format:
            return TemLangStringCopy(
                     &dest->formatName, &src->formatName, allocator) &&
                   ExpressionCopy(
                     &dest->formatArgs, &src->formatArgs, allocator) &&
                   TemLangStringCopy(
                     &dest->formatString, &src->formatString, allocator);
        case InstructionType_NumberRound:
            dest->numberRound = src->numberRound;
            return TemLangStringCopy(&dest->numberRoundName,
                                     &src->numberRoundName,
                                     allocator) &&
                   ExpressionCopy(&dest->numberRoundTarget,
                                  &src->numberRoundTarget,
                                  allocator) &&
                   StructMemberCopy(&dest->numberRoundMember,
                                    &src->numberRoundMember,
                                    allocator);
        default:
            copyFailure(InstructionTypeToString(src->type));
            return false;
    }
}

static inline TemLangString
InstructionToString(const Instruction* i, const Allocator* allocator)
{
    TemLangString a = InstructionSourceToString(&i->source, allocator);
    TemLangString b = { .allocator = allocator };
    switch (i->type) {
        case InstructionType_InlineCHeaders:
            b = TemLangStringCreate("", allocator);
            break;
        case InstructionType_CreateVariable:
            b =
              CreateVariableInstructionToString(&i->createVariable, allocator);
            break;
        case InstructionType_UpdateVariable:
            b =
              UpdateVariableInstructionToString(&i->updateVariable, allocator);
            break;
        case InstructionType_DefineRange:
            b = DefineRangeInstructionToString(&i->defineRange, allocator);
            break;
        case InstructionType_DefineEnum:
            b = DefineEnumInstructionToString(&i->defineEnum, allocator);
            break;
        case InstructionType_ChangeFlag:
            b = ChangeFlagInstructionToString(&i->changeFlag, allocator);
            break;
        case InstructionType_SetAllFlag:
            b = SetAllFlagInstructionToString(&i->setAllFlag, allocator);
            break;
        case InstructionType_DefineStruct:
            b = DefineStructInstructionToString(&i->defineStruct, allocator);
            break;
        case InstructionType_ListModify:
            b = ListModifyInstructionToString(&i->listModify, allocator);
            break;
        case InstructionType_Verify: {
            TemLangStringCreateFormat(
              c, allocator, "\"%s\"", i->verifyName.buffer);
            b = c;
        } break;
        case InstructionType_While:
        case InstructionType_Until:
        case InstructionType_Iterate:
            b = CaptureInstructionToString(&i->captureInstruction, allocator);
            break;
        case InstructionType_Print:
        case InstructionType_Error:
            b = ExpressionListToString(&i->printExpressions, allocator);
            break;
        case InstructionType_DefineFunction: {
            TemLangString c =
              FunctionDefinitionToString(&i->functionDefinition, allocator);
            TemLangStringCreateFormat(d,
                                      allocator,
                                      "{ \"name\": \"%s\", \"value\": %s }",
                                      i->functionName.buffer,
                                      c.buffer);
            TemLangStringFree(&c);
            b = d;
        } break;
        case InstructionType_NoCompile:
        case InstructionType_NoCleanup: {
            b = InstructionListToString(&i->instructions, allocator);
        } break;
        case InstructionType_Return:
        case InstructionType_Inline:
        case InstructionType_InlineC:
        case InstructionType_InlineFile:
        case InstructionType_InlineCFile:
            b = ExpressionToString(&i->expression, allocator);
            break;
        case InstructionType_IfReturn: {
            TemLangString c = ExpressionToString(&i->ifCondition, allocator);
            TemLangString d = ExpressionToString(&i->ifResult, allocator);
            TemLangStringCreateFormat(e,
                                      allocator,
                                      "{ \"if\": %s, \"result\": %s }",
                                      c.buffer,
                                      d.buffer);
            TemLangStringFree(&c);
            TemLangStringFree(&d);
            b = e;
        } break;
        case InstructionType_InlineData: {
            TemLangString c = ExpressionToString(&i->dataFile, allocator);
            TemLangStringCreateFormat(
              d,
              allocator,
              "{ \"name\": \"%s\", \"value\": %s, \"isBinary\": %s }",
              i->dataName.buffer,
              c.buffer,
              i->dataIsBinary ? "true" : "false");
            TemLangStringFree(&c);
            b = d;
        } break;
        case InstructionType_InlineCFunction: {
            TemLangString c =
              ExpressionToString(&i->inlinedFunctionName, allocator);
            TemLangString d = InstructionListToString(
              &i->inlinedFunctionInstructions, allocator);
            TemLangStringAppendFormat(
              b,
              "{ \"definition\": \"%s\", \"instructions\": %s }",
              c.buffer,
              d.buffer);
            TemLangStringFree(&c);
            TemLangStringFree(&d);
        } break;
        case InstructionType_InlineCFunctionReturnStruct: {
            TemLangString c =
              ExpressionToString(&i->inlinedFunctionName, allocator);
            TemLangString e =
              ExpressionToString(&i->inlinedFunctionArgs, allocator);
            TemLangString d = InstructionListToString(
              &i->inlinedFunctionInstructions, allocator);
            TemLangStringAppendFormat(
              b,
              "{ \"name\": \"%s\", \"arguments\": %s, \"instructions\": %s }",
              c.buffer,
              e.buffer,
              d.buffer);
            TemLangStringFree(&c);
            TemLangStringFree(&e);
            TemLangStringFree(&d);
        } break;
        case InstructionType_Run: {
            TemLangStringCreateFormat(
              c, allocator, "\"%s\"", i->procedureName.buffer);
            b = c;
        } break;
        case InstructionType_Match:
            b = MatchInstructionToString(&i->matchInstruction, allocator);
            break;
        case InstructionType_InlineVariable: {
            TemLangString c =
              StructMemberToString(&i->definitionType, allocator);
            TemLangStringAppendFormat(
              b,
              "{ \"name\": \"%s\", \"type\": \"%s\", \"type\": \"%s\" }",
              i->definitionName.buffer,
              c.buffer,
              VariableTypeToString(i->definitionVariableType));
            TemLangStringFree(&c);
        } break;
        case InstructionType_ConvertContainer: {
            TemLangString c = ExpressionToString(&i->fromContainer, allocator);
            TemLangStringAppendFormat(
              b,
              "{ \"from\": %s, \"to\": \"%s\", \"toArray\": %s }",
              c.buffer,
              i->toContainer.buffer,
              i->toArray ? "true" : "false");
            TemLangStringFree(&c);
        } break;
        case InstructionType_Format: {
            TemLangString c = ExpressionToString(&i->formatArgs, allocator);
            TemLangStringAppendFormat(
              b,
              "{ \"name\": \"%s\", \"arguments\": %s, \"format\": \"%s\"}",
              i->formatName.buffer,
              c.buffer,
              i->formatString.buffer);
            TemLangStringFree(&c);
        } break;
        case InstructionType_NumberRound: {
            TemLangString c =
              ExpressionToString(&i->numberRoundTarget, allocator);
            TemLangString d =
              StructMemberToString(&i->numberRoundMember, allocator);
            TemLangStringAppendFormat(b,
                                      "{ \"name\": \"%s\", \"expression\": %s, "
                                      "\"action\": \"%s\", \"target\": %s }",
                                      i->numberRoundName.buffer,
                                      c.buffer,
                                      NumberRoundToString(i->numberRound),
                                      d.buffer);
            TemLangStringFree(&c);
            TemLangStringFree(&d);
        } break;
        default:
            TemLangError("Printing instruction '%s' not implemented",
                         InstructionTypeToString(i->type));
            break;
    }
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"type\": \"%s\", \"source\": %s, \"value\": %s }",
      InstructionTypeToString(i->type),
      a.buffer,
      b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
    return s;
}

static inline bool
TokensToTemLangStringList(const TokenList* tokens,
                          const InstructionSource source,
                          const Allocator* allocator,
                          const char* name,
                          pTemLangStringList list)
{
    if (list->allocator == NULL) {
        list->allocator = allocator;
    }
    bool result = true;
    for (size_t i = 0; result && i < tokens->used; ++i) {
        const Token* t = &tokens->buffer[i];
        if (t->type != TokenType_Identifier) {
            UnexpectedTokenTypeError(&source, TokenType_Identifier, t->type);
            result = false;
            break;
        }
        TemLangString n =
          TemLangStringCreateFromSize(t->string, t->length + 1, allocator);
        if (TemLangStringListFindIf(
              list,
              (TemLangStringListFindFunc)TemLangStringsAreEqual,
              &n,
              NULL,
              NULL)) {
            result = false;
            TemLangError("%s has duplicate member '%s'(%s:%zu)",
                         name,
                         n.buffer,
                         source.source.buffer,
                         source.lineNumber);
        } else {
            result = TemLangStringListAppend(list, &n);
        }
        TemLangStringFree(&n);
    }
    return result;
}

static inline bool
TokensToStringPairList(const TokenList* tokens,
                       const InstructionSource source,
                       const Allocator* allocator,
                       const char* name,
                       pTemLangStringList list)
{
    bool result = true;
    for (size_t i = 0; result && i < tokens->used - 1; i += 2) {
        const Token* t1 = &tokens->buffer[i];
        const Token* t2 = &tokens->buffer[i + 1];
        if (t1->type != TokenType_Identifier) {
            UnexpectedTokenTypeError(&source, TokenType_Identifier, t1->type);
            result = false;
            break;
        }
        if (t2->type != TokenType_Identifier) {
            UnexpectedTokenTypeError(&source, TokenType_Identifier, t2->type);
            result = false;
            break;
        }
        TemLangString key =
          TemLangStringCreateFromSize(t1->string, t1->length + 1, allocator);
        TemLangString value =
          TemLangStringCreateFromSize(t2->string, t2->length + 1, allocator);
        if (TemLangStringListFindIf(
              list,
              (TemLangStringListFindFunc)TemLangStringsAreEqual,
              &key,
              NULL,
              NULL)) {
            result = false;
            TemLangError("%s has duplicate key '%s'(%s:%zu)",
                         name,
                         key.buffer,
                         source.source.buffer,
                         source.lineNumber);
        } else {
            result = TemLangStringListAppend(list, &key) &&
                     TemLangStringListAppend(list, &value);
        }
        TemLangStringFree(&key);
        TemLangStringFree(&value);
    }
    if (result && tokens->used % 2 != 0) {
        const TokenList temp = { .buffer = &tokens->buffer[tokens->used - 1],
                                 .size = 1,
                                 .used = 1,
                                 .allocator = NULL };
        return TokensToTemLangStringList(&temp, source, allocator, name, list);
    }
    return result;
}

static inline bool
TokensToInstruction(const InstructionStarter starter,
                    const Token* tokens,
                    const size_t size,
                    const Allocator* allocator,
                    pInstruction instruction)
{
    switch (starter) {
        case InstructionStarter_NoCompile:
        case InstructionStarter_NoCleanup: {
            if (size != 1) {
                TemLangError("Expected 1 token. Got %zu tokens.", size);
                return false;
            }
            CHECK_TOKEN_LIST(
              tokens[0], &instruction->source, { return false; });
            instruction->type = starter == InstructionStarter_NoCompile
                                  ? InstructionType_NoCompile
                                  : InstructionType_NoCleanup;
            instruction->instructions =
              TokensToInstructions(&tokens[0].tokens, allocator);
            return true;
        } break;
        case InstructionStarter_Return:
            if (size == 0) {
                TemLangError("Return expected an expression. Got 0 tokens.");
                return false;
            }
            instruction->type = InstructionType_Return;
            return TokensToExpression(
              tokens, size, &instruction->expression, allocator);
            break;
        case InstructionStarter_IfReturn:
        case InstructionStarter_ReturnIf: {
            if (size != 2) {
                TemLangError("If return expects 2 tokens. Got %zu", size);
                return false;
            }
            instruction->type = InstructionType_IfReturn;
            return TokenToExpression(
                     &tokens[0], &instruction->ifCondition, allocator) &&
                   TokenToExpression(
                     &tokens[1], &instruction->ifResult, allocator);
        } break;
        case InstructionStarter_Print:
        case InstructionStarter_Error: {
            instruction->type = starter == InstructionStarter_Print
                                  ? InstructionType_Print
                                  : InstructionType_Error;
            bool result = true;
            instruction->printExpressions.allocator = allocator;
            for (size_t i = 0; result && i < size; ++i) {
                Expression e = { 0 };
                result =
                  TokenToExpression(&tokens[i], &e, allocator) &&
                  ExpressionListAppend(&instruction->printExpressions, &e);
                ExpressionFree(&e);
            }
            return result;
        } break;
        case InstructionStarter_Let:
        case InstructionStarter_MLet:
        case InstructionStarter_MutableLet:
        case InstructionStarter_Const:
        case InstructionStarter_Constant: {
            if (tokens[0].type != TokenType_Identifier) {
                UnexpectedTokenTypeError(
                  &instruction->source, TokenType_Identifier, tokens[0].type);
                return false;
            }
            if (size == 1) {
                UnexpectedEndError(instruction->source);
                return false;
            }
            instruction->createVariable.name = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            Token token = { .type = TokenType_Expression };
            TokenList list = { 0 };
            list.buffer = (Token*)(tokens + 1);
            list.used = size - 1;
            list.size = size - 1;
            token.tokens = list;
            switch (starter) {
                case InstructionStarter_MLet:
                case InstructionStarter_MutableLet:
                    instruction->createVariable.type = VariableType_Mutable;
                    break;
                case InstructionStarter_Const:
                case InstructionStarter_Constant:
                    instruction->createVariable.type = VariableType_Constant;
                    break;
                default:
                    instruction->createVariable.type = VariableType_Immutable;
                    break;
            }
            instruction->type = InstructionType_CreateVariable;
            return TokenToExpression(
              &token, &instruction->createVariable.value, allocator);
        } break;
        case InstructionStarter_Set: {
            if (size < 2) {
                UnexpectedEndError(instruction->source);
                return false;
            }
            if (!TokenToExpression(
                  &tokens[0], &instruction->updateVariable.target, allocator)) {
                return false;
            }
            Token token = { .type = TokenType_Expression };
            TokenList list = { 0 };
            list.buffer = (Token*)(tokens + 1);
            list.used = size - 1;
            list.size = size - 1;
            token.tokens = list;
            instruction->type = InstructionType_UpdateVariable;
            return TokenToExpression(
              &token, &instruction->updateVariable.value, allocator);
        } break;
        case InstructionStarter_Range: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for range creation. Got %zu",
                             size);
                return false;
            }
            if (tokens[0].type != TokenType_Identifier) {
                UnexpectedTokenTypeError(
                  &instruction->source, TokenType_Identifier, tokens[0].type);
                return false;
            }

            instruction->type = InstructionType_DefineRange;
            instruction->defineRange.name = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            return TokenToExpression(
                     &tokens[1], &instruction->defineRange.min, allocator) &&
                   TokenToExpression(
                     &tokens[2], &instruction->defineRange.max, allocator);
        } break;
        case InstructionStarter_Enum:
        case InstructionStarter_Flag: {
            if (size != 2) {
                TemLangError("Expected 2 tokens for enum creation. Got %zu",
                             size);
                return false;
            }
            if (tokens[0].type != TokenType_Identifier) {
                UnexpectedTokenTypeError(
                  &instruction->source, TokenType_Identifier, tokens[0].type);
                return false;
            }
            if (!TokenHasList(&tokens[1])) {
                UnexpectedTypeError(&instruction->source,
                                    "List",
                                    TokenTypeToString(tokens[1].type));
                return false;
            }
            instruction->type = InstructionType_DefineEnum;
            instruction->defineEnum.name = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->defineEnum.definition.isFlag =
              starter == InstructionStarter_Flag;
            instruction->defineEnum.definition.members.allocator = allocator;
            char name[1024] = { 0 };
            snprintf(name,
                     sizeof(name),
                     "%s '%s'",
                     instruction->defineEnum.definition.isFlag ? "Flag"
                                                               : "Enum",
                     instruction->defineEnum.name.buffer);
            const bool result = TokensToTemLangStringList(
              &tokens[1].tokens,
              instruction->source,
              allocator,
              name,
              &instruction->defineEnum.definition.members);
            return result ? tokens[1].tokens.used ==
                              instruction->defineEnum.definition.members.used
                          : false;
        } break;
        case InstructionStarter_On:
        case InstructionStarter_Off:
        case InstructionStarter_Toggle: {
            if (size != 2) {
                TemLangError(
                  "Expected 2 tokens for on/off instruction. Got %zu", size);
                return false;
            }
            if (tokens[1].type != TokenType_Identifier) {
                UnexpectedTokenTypeError(
                  &instruction->source, tokens[1].type, TokenType_Identifier);
                return false;
            }

            instruction->type = InstructionType_ChangeFlag;
            if (!TokenToExpression(
                  &tokens[0], &instruction->changeFlag.target, allocator)) {
                return false;
            }

            instruction->changeFlag.member = TemLangStringCreateFromSize(
              tokens[1].string, tokens[1].length + 1, allocator);
            switch (starter) {
                case InstructionStarter_On:
                    instruction->changeFlag.flag = ChangeFlagType_Add;
                    break;
                case InstructionStarter_Off:
                    instruction->changeFlag.flag = ChangeFlagType_Remove;
                    break;
                default:
                    instruction->changeFlag.flag = ChangeFlagType_Toggle;
                    break;
            }
            return true;
        } break;
        case InstructionStarter_All:
        case InstructionStarter_Clear: {
            if (size != 1) {
                TemLangError("Expected 1 token for on/off instruction. Got %zu",
                             size);
                return false;
            }
            instruction->type = InstructionType_SetAllFlag;
            instruction->setAllFlag.clear = starter == InstructionStarter_Clear;
            return TokenToExpression(
              &tokens[0], &instruction->setAllFlag.target, allocator);
        } break;
        case InstructionStarter_Struct:
        case InstructionStarter_Resource:
        case InstructionStarter_Variant: {
            const bool isResource = starter == InstructionStarter_Resource;
            if (isResource) {
                if (size != 4) {
                    TemLangError(
                      "Expected 4 tokens for resource instruction. Got %zu",
                      size);
                    return false;
                }
            } else if (size != 2) {
                TemLangError(
                  "Expected 2 tokens for struct/variant instruction. Got %zu",
                  size);
                return false;
            }
            if (tokens[0].type != TokenType_Identifier) {
                UnexpectedTokenTypeError(
                  &instruction->source, TokenType_Identifier, tokens[0].type);
                return false;
            }
            if (!TokenHasList(&tokens[1])) {
                UnexpectedTypeError(&instruction->source,
                                    "List",
                                    TokenTypeToString(tokens[1].type));
                return false;
            }
            if (tokens[1].tokens.used == 0) {
                TemLangError("Structs/variants must have at least 1 member");
                return false;
            }
            if (tokens[1].tokens.used % 2 != 0) {
                TemLangError("Expected even number of tokens. Got %zu",
                             tokens[1].tokens.used);
                return false;
            }
            instruction->type = InstructionType_DefineStruct;
            instruction->defineStruct.definition.isVariant =
              starter == InstructionStarter_Variant;
            instruction->defineStruct.name = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->defineStruct.definition.members.allocator = allocator;
            bool result = true;
            for (size_t i = 0; result && i < tokens[1].tokens.used; i += 2) {
                const Token* t = &tokens[1].tokens.buffer[i];
                const Token* name = &tokens[1].tokens.buffer[i + 1];
                if (name->type != TokenType_Identifier) {
                    UnexpectedTokenTypeError(
                      &instruction->source, TokenType_Identifier, name->type);
                    result = false;
                    break;
                }
                TempStructMember tempM = { 0 };
                if (!TempStructMemberFromToken(t, &tempM)) {
                    TemLangString s = TokenToString(t, allocator);
                    TemLangError("Token '%s' is not a type", s.buffer);
                    TemLangStringFree(&s);
                    result = false;
                    goto cleanupStructMember;
                }
                StructMember m = ToRealStructMember(&tempM, allocator);
                m.name = TemLangStringCreateFromSize(
                  name->string, name->length + 1, allocator);
                size_t index = 0;
                if (StructMemberListFindIf(
                      &instruction->defineStruct.definition.members,
                      (StructMemberListFindFunc)StructMembersNameEquals,
                      &m,
                      NULL,
                      &index)) {
                    result = false;
                    TemLangError("Duplicate member '%s' found in struct '%s' "
                                 "at indexes %zu and %zu",
                                 m.name.buffer,
                                 instruction->defineStruct.name.buffer,
                                 i,
                                 index);
                } else {
                    result = StructMemberListAppend(
                      &instruction->defineStruct.definition.members, &m);
                }
            cleanupStructMember:
                StructMemberFree(&m);
            }
            if (result && isResource) {
                CHECK_TOKEN(tokens[2],
                            TokenType_Identifier,
                            &instruction->source,
                            { result = false; });
                CHECK_TOKEN_LIST(
                  tokens[3], &instruction->source, { result = false; });

                instruction->defineStruct.definition.destructorTargetName =
                  TemLangStringCreateFromSize(
                    tokens[2].string, tokens[2].length + 1, allocator);
                instruction->defineStruct.definition.deleteInstructions =
                  TokensToInstructions(&tokens[3].tokens, allocator);
            }
            return result ? tokens[1].tokens.used / 2 ==
                              instruction->defineStruct.definition.members.used
                          : false;
        } break;
        case InstructionStarter_Pop:
        case InstructionStarter_Empty: {
            if (size != 1) {
                TemLangError(
                  "Expected 1 token for pop/empty/sort instruction. Got %zu",
                  size);
                return false;
            }
            instruction->type = InstructionType_ListModify;
            switch (starter) {
                case InstructionStarter_Pop:
                    instruction->listModify.type = ListModifyType_Pop;
                    break;
                default:
                    instruction->listModify.type = ListModifyType_Empty;
                    break;
            }
            return TokenToExpression(
              &tokens[0], &instruction->listModify.list, allocator);
        } break;
        case InstructionStarter_Append:
        case InstructionStarter_Remove:
        case InstructionStarter_SwapRemove: {
            if (size != 2) {
                TemLangError(
                  "Expected 2 tokens for append/remove instruction. Got %zu",
                  size);
                return false;
            }
            instruction->type = InstructionType_ListModify;
            switch (starter) {
                case InstructionStarter_Append:
                    instruction->listModify.type = ListModifyType_Append;
                    break;
                case InstructionStarter_Remove:
                    instruction->listModify.type = ListModifyType_Remove;
                    break;
                default:
                    instruction->listModify.type = ListModifyType_SwapRemove;
                    break;
            }
            return TokenToExpression(
                     &tokens[0], &instruction->listModify.list, allocator) &&
                   TokenToExpression(&tokens[1],
                                     &instruction->listModify.newValue[0],
                                     allocator);
        } break;
        case InstructionStarter_Insert: {
            if (size != 3) {
                TemLangError(
                  "Expected 3 tokens for insert instruction. Got %zu", size);
                return false;
            }
            instruction->type = InstructionType_ListModify;
            instruction->listModify.type = ListModifyType_Insert;
            return TokenToExpression(
                     &tokens[0], &instruction->listModify.list, allocator) &&
                   TokenToExpression(&tokens[1],
                                     &instruction->listModify.newValue[0],
                                     allocator) &&
                   TokenToExpression(&tokens[2],
                                     &instruction->listModify.newValue[1],
                                     allocator);
        } break;
        case InstructionStarter_While:
        case InstructionStarter_Until:
        case InstructionStarter_Iterate: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for %s instruction. Got %zu",
                             InstructionStarterToString(starter),
                             size);
                return false;
            }

            if (!TokenHasList(&tokens[1]) || !TokenHasList(&tokens[2])) {
                UnexpectedTokenTypeError(
                  &instruction->source,
                  TokenType_List,
                  TokenHasList(&tokens[1]) ? tokens[2].type : tokens[1].type);
                return false;
            }

            switch (starter) {
                case InstructionStarter_Iterate:
                    instruction->type = InstructionType_Iterate;
                    break;
                case InstructionStarter_Until:
                    instruction->type = InstructionType_Until;
                    break;
                default:
                    instruction->type = InstructionType_While;
                    break;
            }

            instruction->captureInstruction.instructions =
              TokensToInstructions(&tokens[2].tokens, allocator);
            instruction->captureInstruction.captures.allocator = allocator;
            return CheckInstructionSize(
                     &instruction->captureInstruction.instructions,
                     InstructionStarterToString(starter)) &&
                   TokenToExpression(&tokens[0],
                                     &instruction->captureInstruction.target,
                                     allocator) &&
                   TokensToTemLangStringList(
                     &tokens[1].tokens,
                     instruction->source,
                     allocator,
                     InstructionTypeToString(instruction->type),
                     &instruction->captureInstruction.captures);
        } break;
        case InstructionStarter_Nullary: {
            if (size != 2) {
                TemLangError(
                  "Expected 2 tokens for nullary definition. Got %zu", size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN_LIST(
              tokens[1], &instruction->source, { return false; });
            instruction->type = InstructionType_DefineFunction;
            instruction->functionDefinition.type = FunctionType_Nullary;
            instruction->functionName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->functionDefinition.instructions =
              TokensToInstructions(&tokens[1].tokens, allocator);
            return CheckInstructionSize(
              &instruction->functionDefinition.instructions, "Nullary");
        } break;
        case InstructionStarter_Unary: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for unary definition. Got %zu",
                             size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN(tokens[1], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN_LIST(
              tokens[2], &instruction->source, { return false; });
            instruction->type = InstructionType_DefineFunction;
            instruction->functionDefinition.type = FunctionType_Unary;
            instruction->functionName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->functionDefinition.leftParameter =
              TemLangStringCreateFromSize(
                tokens[1].string, tokens[1].length + 1, allocator);
            instruction->functionDefinition.instructions =
              TokensToInstructions(&tokens[2].tokens, allocator);
            return CheckInstructionSize(
              &instruction->functionDefinition.instructions, "Unary");
        } break;
        case InstructionStarter_Binary: {
            if (size != 4) {
                TemLangError("Expected 4 tokens for binary definition. Got %zu",
                             size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN(tokens[1], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN(tokens[2], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN_LIST(
              tokens[3], &instruction->source, { return false; });
            instruction->type = InstructionType_DefineFunction;
            instruction->functionDefinition.type = FunctionType_Binary;
            instruction->functionName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->functionDefinition.leftParameter =
              TemLangStringCreateFromSize(
                tokens[1].string, tokens[1].length + 1, allocator);
            instruction->functionDefinition.rightParameter =
              TemLangStringCreateFromSize(
                tokens[2].string, tokens[2].length + 1, allocator);
            instruction->functionDefinition.instructions =
              TokensToInstructions(&tokens[3].tokens, allocator);
            return CheckInstructionSize(
              &instruction->functionDefinition.instructions, "Binary");
        } break;
        case InstructionStarter_Procedure: {
            if (size != 3) {
                TemLangError(
                  "Expected 3 tokens for procedure definition. Got %zu", size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN_LIST(
              tokens[1], &instruction->source, { return false; });
            CHECK_TOKEN_LIST(
              tokens[2], &instruction->source, { return false; });
            instruction->type = InstructionType_DefineFunction;
            instruction->functionDefinition.type = FunctionType_Procedure;
            instruction->functionName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->functionDefinition.instructions =
              TokensToInstructions(&tokens[2].tokens, allocator);
            instruction->functionDefinition.captures.allocator = allocator;
            return CheckInstructionSize(
                     &instruction->functionDefinition.instructions,
                     "Procedure") &&
                   TokensToTemLangStringList(
                     &tokens[1].tokens,
                     instruction->source,
                     allocator,
                     "Procedure",
                     &instruction->functionDefinition.captures);
        } break;
        case InstructionStarter_Run: {
            if (size != 1) {
                TemLangError("Expected 1 token for run instruction. Got %zu",
                             size);
                return false;
            }
            instruction->type = InstructionType_Run;
            instruction->procedureName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            return true;
        } break;
        case InstructionStarter_Match: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for %s instruction. Got %zu",
                             InstructionStarterToString(starter),
                             size);
                return false;
            }

            if (!TokenHasList(&tokens[1]) || !TokenHasList(&tokens[2])) {
                UnexpectedTokenTypeError(
                  &instruction->source,
                  TokenType_List,
                  TokenHasList(&tokens[1]) ? tokens[2].type : tokens[1].type);
                return false;
            }

            instruction->type = InstructionType_Match;
            pMatchExpression exp = &instruction->matchInstruction.expression;
            exp->branches.allocator = allocator;
            if (!TokensToMatchBranch(tokens[2].tokens.buffer,
                                     tokens[2].tokens.used,
                                     allocator,
                                     &exp->branches)) {
                return false;
            }

            if (tokens[2].tokens.used % 2 == 0) {
                exp->defaultBranch.type = MatchBranchType_None;
            } else {
                const Token* last =
                  &tokens[2].tokens.buffer[tokens[2].tokens.used - 1];
                if (last->type == TokenType_Scope) {
                    exp->defaultBranch.type = MatchBranchType_Instructions;
                    exp->defaultBranch.instructions =
                      TokensToInstructions(&last->tokens, allocator);
                } else {
                    exp->defaultBranch.type = MatchBranchType_Expression;
                    if (!TokenToExpression(
                          last, &exp->defaultBranch.expression, allocator)) {
                        return false;
                    }
                }
            }

            if (instruction->matchInstruction.expression.branches.used == 0) {
                TemLangError("Cannot have match expression with no branches");
                return false;
            }
            instruction->matchInstruction.captures.allocator = allocator;
            return TokenToExpression(&tokens[0], &exp->matcher, allocator) &&
                   TokensToTemLangStringList(
                     &tokens[1].tokens,
                     instruction->source,
                     allocator,
                     InstructionTypeToString(instruction->type),
                     &instruction->matchInstruction.captures);
        } break;
        case InstructionStarter_Inline: {
            instruction->type = InstructionType_Inline;
            return TokensToExpression(
              tokens, size, &instruction->expression, allocator);
        } break;
        case InstructionStarter_InlineC: {
            instruction->type = InstructionType_InlineC;
            return TokensToExpression(
              tokens, size, &instruction->expression, allocator);
        } break;
        case InstructionStarter_InlineFile: {
            instruction->type = InstructionType_InlineFile;
            return TokensToExpression(
              tokens, size, &instruction->expression, allocator);
        } break;
        case InstructionStarter_InlineCFile: {
            instruction->type = InstructionType_InlineCFile;
            return TokensToExpression(
              tokens, size, &instruction->expression, allocator);
        } break;
        case InstructionStarter_InlineData:
        case InstructionStarter_InlineText: {
            if (size != 2) {
                TemLangError("Expected 2 tokens for inline data "
                             "instruction. Got %zu",
                             size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            instruction->type = InstructionType_InlineData;
            instruction->dataIsBinary =
              InstructionStarter_InlineData == starter;
            instruction->dataName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            return TokenToExpression(
              &tokens[1], &instruction->dataFile, allocator);
        } break;
        case InstructionStarter_InlineCFunction: {
            if (size != 2) {
                TemLangError("Expected 2 tokens for inlined C function "
                             "instruction. Got %zu",
                             size);
                return false;
            }
            instruction->type = InstructionType_InlineCFunction;
            CHECK_TOKEN_LIST(tokens[1], &instruction->source, { return false; })
            instruction->inlinedFunctionInstructions =
              TokensToInstructions(&tokens[1].tokens, allocator);
            size_t returns = 0;
            for (size_t i = 0;
                 i < instruction->inlinedFunctionInstructions.used;
                 ++i) {
                if (instruction->inlinedFunctionInstructions.buffer[i].type ==
                    InstructionType_Return) {
                    ++returns;
                }
            }
            if (returns > 1) {
                TemLangError("Inlined C functions can only have one return "
                             "statement. Got %zu",
                             returns);
                return false;
            }
            return TokenToExpression(
              &tokens[0], &instruction->inlinedFunctionName, allocator);
        } break;
        case InstructionStarter_InlineCFunctionReturnStruct: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for inlined C function "
                             "with return struct "
                             "instruction. Got %zu",
                             size);
                return false;
            }
            instruction->type = InstructionType_InlineCFunctionReturnStruct;
            CHECK_TOKEN_LIST(tokens[2], &instruction->source, { return false; })
            instruction->inlinedFunctionInstructions =
              TokensToInstructions(&tokens[2].tokens, allocator);
            return TokenToExpression(&tokens[0],
                                     &instruction->inlinedFunctionName,
                                     allocator) &&
                   TokenToExpression(
                     &tokens[1], &instruction->inlinedFunctionArgs, allocator);
        } break;
        case InstructionStarter_InlineVariable: {
            if (size != 3) {
                TemLangError("Expected 3 tokens for inlined variable "
                             "instruction. Got %zu",
                             size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            })
            CHECK_TOKEN(tokens[2], TokenType_Keyword, &instruction->source, {
                return false;
            })
            switch (tokens[2].keyword) {
                case Keyword_True:
                    instruction->definitionVariableType = VariableType_Mutable;
                    break;
                case Keyword_False:
                    instruction->definitionVariableType =
                      VariableType_Immutable;
                    break;
                default:
                    TemLangError("Expected 'true' or 'false' keyword. Got '%s'",
                                 KeywordToString(tokens[2].keyword));
                    return false;
            }
            instruction->type = InstructionType_InlineVariable;
            instruction->definitionName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            TempStructMember m = { 0 };
            if (!TempStructMemberFromToken(&tokens[1], &m)) {
                return false;
            }
            instruction->definitionType = ToRealStructMember(&m, allocator);
            return true;
        } break;
        case InstructionStarter_Verify: {
            if (size != 1) {
                TemLangError("Expected 1 token for veirfy instruction. Got %zu",
                             size);
                return false;
            }

            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            instruction->type = InstructionType_Verify;
            instruction->verifyName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            return true;
        } break;
        case InstructionStarter_ToArray:
        case InstructionStarter_ToList: {
            if (size != 2) {
                TemLangError("Expected 2 tokens for container conversion",
                             size);
                return false;
            }
            CHECK_TOKEN(tokens[1], TokenType_Identifier, &instruction->source, {
                return false;
            })
            instruction->type = InstructionType_ConvertContainer;
            instruction->toArray = starter == InstructionStarter_ToArray;
            instruction->toContainer = TemLangStringCreateFromSize(
              tokens[1].string, tokens[1].length + 1, allocator);
            return TokenToExpression(
              &tokens[0], &instruction->fromContainer, allocator);
        } break;
        case InstructionStarter_InlineCHeaders: {
            if (size != 0) {
                TemLangError("Expected 0 tokens for inline C headers. Got %zu",
                             size);
                return false;
            }
            instruction->type = InstructionType_InlineCHeaders;
            return true;
        } break;
        case InstructionStarter_Format: {
            if (size != 3) {
                TemLangError(
                  "Expected 3 tokens for format instruction. Got %zu", size);
                return false;
            }
            CHECK_TOKEN(tokens[0], TokenType_Identifier, &instruction->source, {
                return false;
            });
            CHECK_TOKEN(tokens[2], TokenType_String, &instruction->source, {
                return false;
            });
            instruction->type = InstructionType_Format;
            instruction->formatName = TemLangStringCreateFromSize(
              tokens[0].string, tokens[0].length + 1, allocator);
            instruction->formatString = TemLangStringCreateFromSize(
              tokens[2].string, tokens[2].length + 1, allocator);
            return TokenToExpression(
              &tokens[1], &instruction->formatArgs, allocator);
        } break;
        case InstructionStarter_Ceil:
        case InstructionStarter_Round:
        case InstructionStarter_Floor: {
            if (size != 3) {
                TemLangError(
                  "Expected 3 tokens for format instruction. Got %zu", size);
                return false;
            }
            CHECK_TOKEN(tokens[2], TokenType_Identifier, &instruction->source, {
                return false;
            });
            instruction->type = InstructionType_NumberRound;
            switch (starter) {
                case InstructionStarter_Ceil:
                    instruction->numberRound = NumberRound_Ceil;
                    break;
                case InstructionStarter_Round:
                    instruction->numberRound = NumberRound_Round;
                    break;
                case InstructionStarter_Floor:
                    instruction->numberRound = NumberRound_Floor;
                    break;
                default:
                    return false;
            }
            TempStructMember m = { 0 };
            if (!TempStructMemberFromToken(&tokens[1], &m)) {
                return false;
            }
            instruction->numberRoundMember = ToRealStructMember(&m, allocator);
            instruction->numberRoundName = TemLangStringCreateFromSize(
              tokens[2].string, tokens[2].length + 1, allocator);
            return TokenToExpression(
              &tokens[0], &instruction->numberRoundTarget, allocator);
        } break;
        default: {
            InstructionStarterError(starter);
            return false;
        } break;
    }
}

static inline InstructionList
TokensToInstructions(const TokenList* tokens, const Allocator* allocator)
{
    InstructionList instructions = {
        .allocator = allocator, .buffer = NULL, .size = 0, .used = 0
    };

    size_t i = 0;
    Instruction instruction = { 0 };
    while (i < tokens->used) {
        instruction.source =
          TokenSourceToInstructionSource(tokens->buffer[i].source, allocator);
        if (tokens->buffer[i].type != TokenType_InstructionStarter) {
            // Execute the rests of the tokens as an expression
            goto parseExpression;
        }
        const size_t start = i + 1;
        const size_t end =
          nextInstructionStarter(tokens->buffer, start, tokens->used);
        const size_t length = end - start;
        if (!TokensToInstruction(tokens->buffer[i].starter,
                                 tokens->buffer + start,
                                 length,
                                 allocator,
                                 &instruction)) {
            TemLangError("Failed to parse tokens to an instruction (%s:%zu)",
                         instruction.source.source.buffer,
                         instruction.source.lineNumber);
            i = tokens->used;
            goto cleanup;
        }
        InstructionListAppend(&instructions, &instruction);
        i = end;
    cleanup:
        InstructionFree(&instruction);
        continue;
    parseExpression:
        if (i >= tokens->used) {
            UnexpectedTokenTypeError(&instruction.source,
                                     TokenType_InstructionStarter,
                                     tokens->buffer[i].type);
            i = tokens->used;
            goto cleanup;
        }
        break;
    }

    return instructions;
}

static inline TemLangString
InstructionListToString(const InstructionList* list, const Allocator* allocator)
{
    LIST_TO_STRING((*list), s, InstructionToString, allocator);
    return s;
}

DEFAULT_MAKE_LIST_FUNCTIONS(Instruction);
static inline const TemLangString*
InstructionCreationName(const Instruction* i)
{
    switch (i->type) {
        case InstructionType_CreateVariable:
            return &i->createVariable.name;
        case InstructionType_DefineEnum:
            return &i->defineEnum.name;
        case InstructionType_DefineStruct:
            return &i->defineStruct.name;
        case InstructionType_DefineFunction:
            return &i->functionName;
        case InstructionType_DefineRange:
            return &i->defineRange.name;
        case InstructionType_InlineVariable:
            return &i->definitionName;
        default:
            return NULL;
    }
}