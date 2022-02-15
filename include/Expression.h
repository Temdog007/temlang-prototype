#pragma once

#include "Allocator.h"
#include "Error.h"
#include "ExpressionType.h"
#include "InstructionList.h"
#include "List.h"
#include "Number.h"
#include "Operator.h"
#include "Token.h"
#include "Value.h"

#include <float.h>

typedef struct State State, *pState;

typedef struct Variable Variable, *pVariable;

typedef struct Expression Expression, *pExpression;

MAKE_LIST(Expression)

typedef struct MatchExpression MatchExpression, *pMatchExpression;

static inline void
MatchExpressionFree(MatchExpression*);

static inline bool
MatchExpressionCopy(MatchExpression*, const MatchExpression*, const Allocator*);

static inline TemLangString
MatchExpressionToString(const MatchExpression*, const Allocator*);

static inline bool
EvaluateMatchExpression(const MatchExpression*,
                        State*,
                        pValue,
                        const Allocator*);

static inline bool
TokenToMatchExpression(const Token* token,
                       pExpression e,
                       const Allocator* allocator);

static inline bool
ExpressionMatchExpressionCopy(Expression* dest,
                              const Expression* src,
                              const Allocator* allocator);

typedef struct MatchBranchList MatchBranchList, *pMatchBranchList;

static inline bool
TokensToMatchBranch(const Token* tokens,
                    const size_t size,
                    const Allocator* allocator,
                    pMatchBranchList list);

typedef struct Expression
{
    ExpressionType type;
    union
    {
        Value value;
        TemLangString identifier;
        InstructionList instructions;
        struct
        {
            pMatchExpression matchExpression;
            const Allocator* matchAllocator;
        };
        struct
        {
            ExpressionList expressions;
            bool isArray;
        };
        struct
        {
            pExpression left;
            pExpression right;
            Operator op;
            const Allocator* expressionAllocator;
        };
    };
} Expression, *pExpression;

static inline TemLangString
InstructionToString(const Instruction* i, const Allocator* allocator);

static inline bool
InstructionCopy(Instruction* dest,
                const Instruction* src,
                const Allocator* allocator);

static inline InstructionList
TokensToInstructions(const TokenList* tokens, const Allocator* allocator);

static inline bool
CheckInstructionSize(const InstructionList* list, const char* name)
{
    if (list->used == 0 || list->buffer == NULL) {
        TemLangError("'%s' must have at least one instruction", name);
        return false;
    }
    return true;
}

static inline void
InstructionFree(Instruction* i);

static inline bool
EvaluateExpression(const Expression* e,
                   const State* state,
                   pValue value,
                   const Allocator* allocator);

static inline Value*
EvaluateExpressionToReference(const Expression* e,
                              State* state,
                              const Allocator* allocator);

static inline bool
ExpressionCopy(Expression*, const Expression*, const Allocator*);

static inline TemLangString
ExpressionToString(const Expression* e, const Allocator* allocator)
{
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"type\": \"%s\", \"value\": ",
                              ExpressionTypeToString(e->type));
    switch (e->type) {
        case ExpressionType_UnaryValue: {
            TemLangString ns = ValueToString(&e->value, allocator);
            TemLangStringAppendFormat(s, "%s }", ns.buffer);
            TemLangStringFree(&ns);
        } break;
        case ExpressionType_UnaryVariable: {
            TemLangStringAppendFormat(s, "\"%s\" }", e->identifier.buffer);
        } break;
        case ExpressionType_UnaryScope:
        case ExpressionType_UnaryStruct: {
            TemLangString n =
              InstructionListToString(&e->instructions, allocator);
            TemLangStringAppend(&s, &n);
            TemLangStringFree(&n);
            TemLangStringAppendChars(&s, " }");
        } break;
        case ExpressionType_UnaryList: {
            LIST_TO_STRING(e->expressions, n, ExpressionToString, allocator);
            TemLangStringAppend(&s, &n);
            TemLangStringFree(&n);
            TemLangStringAppendChars(&s, " }");
        } break;
        case ExpressionType_UnaryMatch: {
            TemLangString n =
              MatchExpressionToString(e->matchExpression, allocator);
            TemLangStringAppend(&s, &n);
            TemLangStringFree(&n);
            TemLangStringAppendChars(&s, " }");
        } break;
        case ExpressionType_Binary: {
            TemLangString l = ExpressionToString(e->left, allocator);
            TemLangString r = ExpressionToString(e->right, allocator);
            TemLangString o = OperatorString(&e->op, allocator);
            TemLangStringAppendFormat(
              s,
              "{ \"left\": %s, \"operator\": %s, \"right\": %s } }",
              l.buffer,
              o.buffer,
              r.buffer);
            TemLangStringFree(&l);
            TemLangStringFree(&r);
            TemLangStringFree(&o);
        } break;
        case ExpressionType_Nullary:
        default: {
            TemLangStringAppendFormat(s, "null }");
        } break;
    }
    return s;
}

static inline void
ExpressionFree(Expression* e)
{
    switch (e->type) {
        case ExpressionType_UnaryValue:
            ValueFree(&e->value);
            break;
        case ExpressionType_UnaryVariable:
            TemLangStringFree(&e->identifier);
            break;
        case ExpressionType_UnaryScope:
        case ExpressionType_UnaryStruct:
            InstructionListFree(&e->instructions);
            break;
        case ExpressionType_UnaryList:
            ExpressionListFree(&e->expressions);
            break;
        case ExpressionType_UnaryMatch:
            MatchExpressionFree(e->matchExpression);
            e->matchAllocator->free(e->matchExpression);
            break;
        case ExpressionType_Binary:
            OperatorFree(&e->op);
            if (e->left != NULL) {
                ExpressionFree(e->left);
                e->expressionAllocator->free(e->left);
            }
            if (e->right != NULL) {
                ExpressionFree(e->right);
                e->expressionAllocator->free(e->right);
            }
            break;
        default:
            break;
    }
    memset(e, 0, sizeof(Expression));
}

static inline bool
ExpressionCopy(Expression* dest,
               const Expression* src,
               const Allocator* allocator)
{
    ExpressionFree(dest);
    dest->type = src->type;
    switch (src->type) {
        case ExpressionType_Nullary:
            return true;
        case ExpressionType_UnaryValue:
            return ValueCopy(&dest->value, &src->value, allocator);
        case ExpressionType_UnaryVariable:
            return TemLangStringCopy(
              &dest->identifier, &src->identifier, allocator);
        case ExpressionType_UnaryScope:
        case ExpressionType_UnaryStruct:
            return InstructionListCopy(
              &dest->instructions, &src->instructions, allocator);
        case ExpressionType_UnaryList:
            dest->isArray = src->isArray;
            return ExpressionListCopy(
              &dest->expressions, &src->expressions, allocator);
        case ExpressionType_UnaryMatch:
            return ExpressionMatchExpressionCopy(dest, src, allocator);
        case ExpressionType_Binary:
            dest->left = allocator->allocate(sizeof(Expression));
            dest->right = allocator->allocate(sizeof(Expression));
            dest->expressionAllocator = allocator;
            return ExpressionCopy(dest->left, src->left, allocator) &&
                   OperatorCopy(&dest->op, &src->op, allocator) &&
                   ExpressionCopy(dest->right, src->right, allocator);
        default:
            copyFailure(ExpressionTypeToString(src->type));
            return false;
    }
}

static inline bool
TokensToExpression(const Token*,
                   size_t,
                   pExpression,
                   const Allocator* allocator);

static inline void
makeRangeValue(pExpression e, const Range range, const Allocator* allocator)
{
    e->type = ExpressionType_UnaryValue;
    e->value.type = ValueType_Type;
    e->value.fakeValue = allocator->allocate(sizeof(Value));
    e->value.fakeValue->type = ValueType_Number;
    e->value.fakeValue->rangedNumber.hasRange = true;
    e->value.fakeValue->rangedNumber.range = range;
    e->value.fakeValueAllocator = allocator;
}

static inline bool
TokenToExpression(const Token* token, pExpression e, const Allocator* allocator)
{
    switch (token->type) {
        case TokenType_Character: {
            e->type = ExpressionType_UnaryValue;
            e->value.type = ValueType_Number;
            e->value.rangedNumber.hasRange = true;
            e->value.rangedNumber.range.min = NumberFromInt(INT8_MIN);
            e->value.rangedNumber.range.max = NumberFromInt(INT8_MAX);
            e->value.rangedNumber.number.i = (int64_t)token->c;
            e->value.rangedNumber.number.type = NumberType_Signed;
            return true;
        } break;
        case TokenType_Keyword:
            switch (token->keyword) {
                case Keyword_Null:
                    e->type = ExpressionType_Nullary;
                    return true;
                case Keyword_True:
                    e->type = ExpressionType_UnaryValue;
                    e->value.type = ValueType_Boolean;
                    e->value.b = true;
                    return true;
                case Keyword_False:
                    e->type = ExpressionType_UnaryValue;
                    e->value.type = ValueType_Boolean;
                    e->value.b = false;
                    return true;
                case Keyword_i8:
                case Keyword_i16:
                case Keyword_i32:
                case Keyword_i64:
                case Keyword_u8:
                case Keyword_u16:
                case Keyword_u32:
                case Keyword_u64:
                case Keyword_f32:
                case Keyword_f64: {
                    Range r = { 0 };
                    if (KeywordToRange(token->keyword, &r)) {
                        makeRangeValue(e, r, allocator);
                        return true;
                    }
                    return false;
                } break;
                case Keyword_external:
                    e->type = ExpressionType_UnaryValue;
                    e->value.type = ValueType_Type;
                    e->value.fakeValue = allocator->allocate(sizeof(Value));
                    e->value.fakeValue->type = ValueType_External;
                    e->value.fakeValue->ptr = NULL;
                    e->value.fakeValueAllocator = allocator;
                    return true;
                case Keyword_bool:
                    e->type = ExpressionType_UnaryValue;
                    e->value.type = ValueType_Type;
                    e->value.fakeValue = allocator->allocate(sizeof(Value));
                    e->value.fakeValue->type = ValueType_Boolean;
                    e->value.fakeValue->b = false;
                    e->value.fakeValueAllocator = allocator;
                    return true;
                case Keyword_string:
                    e->type = ExpressionType_UnaryValue;
                    e->value.type = ValueType_Type;
                    e->value.fakeValue = allocator->allocate(sizeof(Value));
                    e->value.fakeValue->type = ValueType_String;
                    memset(
                      &e->value.fakeValue->string, 0, sizeof(TemLangString));
                    e->value.fakeValueAllocator = allocator;
                    return true;
                default:
                    break;
            }
            break;
        case TokenType_Number:
            e->type = ExpressionType_UnaryValue;
            e->value.type = ValueType_Number;
            e->value.rangedNumber.hasRange = false;
            e->value.rangedNumber.number = token->number;
            return true;
        case TokenType_String:
            e->type = ExpressionType_UnaryValue;
            e->value.type = ValueType_String;
            e->value.string = TemLangStringCreateFromSize(
              token->string, token->length + 1, allocator);
            TemLangStringRemoveBackslahses(&e->value.string);
            return true;
        case TokenType_Type:
            e->type = ExpressionType_Binary;
            e->op.type = OperatorType_Get;
            e->op.getOperator = GetOperator_Type;
            e->expressionAllocator = allocator;
            e->left = allocator->allocate(sizeof(Expression));
            e->left->type = ExpressionType_Nullary;
            e->right = allocator->allocate(sizeof(Expression));
            e->right->type = ExpressionType_UnaryValue;
            e->right->value.type = ValueType_String;
            e->right->value.string = TemLangStringCreateFromSize(
              token->string, token->length + 1, allocator);
            return true;
        case TokenType_Struct:
            e->type = ExpressionType_UnaryStruct;
            e->instructions = TokensToInstructions(&token->tokens, allocator);
            return CheckInstructionSize(&e->instructions, "Struct");
        case TokenType_Scope:
            e->type = ExpressionType_UnaryScope;
            e->instructions = TokensToInstructions(&token->tokens, allocator);
            return CheckInstructionSize(&e->instructions, "Scope");
        case TokenType_Match:
            return TokenToMatchExpression(token, e, allocator);
        case TokenType_Array:
        case TokenType_List: {
            e->type = ExpressionType_UnaryList;
            e->isArray = token->type == TokenType_Array;
            e->expressions.allocator = allocator;
            bool result = true;
            for (size_t i = 0; result && i < token->tokens.used; ++i) {
                Expression newE = { 0 };
                if (!TokenToExpression(
                      &token->tokens.buffer[i], &newE, allocator)) {
                    return false;
                }
                result = ExpressionListAppend(&e->expressions, &newE);
                ExpressionFree(&newE);
            }
            return result;
        } break;
        case TokenType_Expression:
            return TokensToExpression(
              token->tokens.buffer, token->tokens.used, e, allocator);
        case TokenType_Identifier:
            e->type = ExpressionType_UnaryVariable;
            e->identifier = TemLangStringCreateFromSize(
              token->string, token->length + 1, allocator);
            return true;
        case TokenType_ListInitialization: {
            e->type = ExpressionType_Binary;
            e->op.type = OperatorType_Get;
            e->op.getOperator = GetOperator_MakeList;
            e->expressionAllocator = allocator;
            e->left = allocator->allocate(sizeof(Expression));
            e->left->type = ExpressionType_Nullary;
            e->right = allocator->allocate(sizeof(Expression));
            StructMember m =
              ToRealStructMember(&token->structMember, allocator);
            const bool result =
              StructMemberToExpression(&m, e->right, allocator);
            StructMemberFree(&m);
            return result;
        } break;
        default:
            break;
    }
    return false;
}

static inline bool
TokenToOperator(const Token* token, pOperator op, const Allocator* allocator)
{
    switch (token->type) {
        case TokenType_BooleanOperator:
            op->type = OperatorType_Boolean;
            op->booleanOperator = token->booleanOperator;
            return true;
        case TokenType_ComparisonOperator:
            op->type = OperatorType_Comparison;
            op->comparisonOperator = token->comparisonOperator;
            return true;
        case TokenType_NumberOperator:
            op->type = OperatorType_Number;
            op->numberOperator = token->numberOperator;
            return true;
        case TokenType_GetOperator:
            op->type = OperatorType_Get;
            op->getOperator = token->getOperator;
            return true;
        case TokenType_FunctionCall:
            op->type = OperatorType_Function;
            op->functionCall = TemLangStringCreateFromSize(
              token->string, token->length + 1, allocator);
            return true;
        default:
            return false;
    }
}

static inline bool
TokensToExpressionsAndOperators(const Token* tokens,
                                const size_t length,
                                pExpressionList eList,
                                pOperatorList oList,
                                const Allocator* allocator)
{
    Operator op = { 0 };
    size_t i = 0;
    Expression e = { 0 };
    bool result = true;
    bool needOperator = false;
    if (TokenToExpression(&tokens[i], &e, allocator)) {
        ++i;
        result = ExpressionListAppend(eList, &e);
        ExpressionFree(&e);
        needOperator = true;
    } else if (TokenToOperator(&tokens[i], &op, allocator)) {
        ExpressionFree(&e);
        result = ExpressionListAppend(eList, &e);
        if (!result) {
            goto cleanup;
        }
        ExpressionFree(&e);
        result = OperatorListAppend(oList, &op);
        OperatorFree(&op);
        ++i;
        needOperator = false;
    }
    if (!result) {
        goto cleanup;
    }
    while (i < length && result) {
        if (needOperator) {
            if (TokenToOperator(&tokens[i], &op, allocator)) {
                result = OperatorListAppend(oList, &op);
                ++i;
                needOperator = false;
                continue;
            }
        } else if (TokenToExpression(&tokens[i], &e, allocator)) {
            ++i;
            result = ExpressionListAppend(eList, &e);
            ExpressionFree(&e);
            needOperator = true;
            continue;
        } else if (TokenToOperator(&tokens[i], &op, allocator)) {
            ExpressionFree(&e);
            result = ExpressionListAppend(eList, &e);
            if (!result) {
                continue;
            }
            ExpressionFree(&e);
            result = OperatorListAppend(oList, &op);
            OperatorFree(&op);
            needOperator = false;
            ++i;
            continue;
        }

        TemLangString s = TokenToString(&tokens[i], allocator);
        TokenError(tokens[i].source,
                   "Failed to parse token '%s' to operand or operator",
                   s.buffer);
        TemLangStringFree(&s);
        result = false;
        break;
    }
    if (result && eList->used == oList->used) {
        ExpressionFree(&e);
        result = ExpressionListAppend(eList, &e);
        ExpressionFree(&e);
    }
cleanup:
    OperatorFree(&op);
    ExpressionFree(&e);
    return result;
}

static inline bool
FlattenListsToBinaryExpression(const ExpressionList* eList,
                               const size_t eIndex,
                               const size_t eSize,
                               const OperatorList* oList,
                               const size_t oIndex,
                               const size_t oSize,
                               const Allocator* allocator,
                               pExpression e)
{
    const intptr_t oCount = (intptr_t)oSize - (intptr_t)oIndex;
    const intptr_t eCount = (intptr_t)eSize - (intptr_t)eIndex;

    if (oCount <= 0) {
        if (eCount == 1) {
            return ExpressionCopy(e, &eList->buffer[eIndex], allocator);
        } else {
            return false;
        }
    }
    if (eCount <= 0) {
        return false;
    }
    e->expressionAllocator = allocator;
    if (eCount == 2 && oCount == 1) {
        e->type = ExpressionType_Binary;
        e->left = allocator->allocate(sizeof(Expression));
        e->right = allocator->allocate(sizeof(Expression));
        return ExpressionCopy(e->left, &eList->buffer[eIndex], allocator) &&
               OperatorCopy(&e->op, &oList->buffer[oIndex], allocator) &&
               ExpressionCopy(e->right, &eList->buffer[eIndex + 1], allocator);
    }
    int index = -1;
    int precedence = INT32_MAX;
    for (size_t i = oIndex; i < oSize; ++i) {
        const int p = OperatorPrecedence(&oList->buffer[i]);
        if (p <= precedence) {
            precedence = p;
            index = i;
        }
    }
    if (index < (int)oIndex || index >= (int)oSize) {
        return false;
    }
    e->type = ExpressionType_Binary;
    e->left = allocator->allocate(sizeof(Expression));
    if (!FlattenListsToBinaryExpression(
          eList, eIndex, index + 1, oList, oIndex, index, allocator, e->left)) {
        return false;
    }
    e->right = allocator->allocate(sizeof(Expression));
    if (!FlattenListsToBinaryExpression(eList,
                                        index + 1,
                                        eSize,
                                        oList,
                                        index + 1,
                                        oSize,
                                        allocator,
                                        e->right)) {
        return false;
    }
    return OperatorCopy(&e->op, &oList->buffer[index], allocator);
}

static inline bool
TokensToExpression(const Token* tokens,
                   const size_t length,
                   pExpression e,
                   const Allocator* allocator)
{
    switch (length) {
        case 0:
            return false;
        case 1:
            return TokenToExpression(&tokens[0], e, allocator);
        default: {
            ExpressionList eList = {
                .buffer = NULL, .size = 0, .used = 0, .allocator = allocator
            };
            OperatorList oList = {
                .buffer = NULL, .size = 0, .used = 0, .allocator = allocator
            };
            if (!TokensToExpressionsAndOperators(
                  tokens, length, &eList, &oList, allocator)) {
                return false;
            }
            if (eList.used != oList.used + 1) {
                TemLangError("Failed to compile the proper number of operands "
                             "and operators");
                return false;
            }
            const bool result = FlattenListsToBinaryExpression(
              &eList, 0, eList.used, &oList, 0, oList.used, allocator, e);
            ExpressionListFree(&eList);
            OperatorListFree(&oList);
            return result;
        } break;
    }
    return true;
}

static inline bool
EvaluateExpression(const Expression* e,
                   const State*,
                   pValue value,
                   const Allocator* allocator);

static inline bool
EvaluateAddExpression(const Value* left,
                      const Value* right,
                      const State* state,
                      const Allocator* allocator,
                      pValue value);

static inline bool
EvaluateBooleanExpression(const Value* left,
                          const Value* right,
                          const BooleanOperator op,
                          pValue value)
{
    if (ValueTypesMatch(left, ValueType_Boolean, right, ValueType_Boolean)) {
        value->type = ValueType_Boolean;
        value->b = ApplyBooleanOperator(left->b, right->b, op);
        return true;
    }
    const Value* target = getUnaryValue(left, right);
    if (target == NULL || target->type != ValueType_Boolean) {
        return false;
    }
    if (op == BooleanOperator_Not) {
        value->b = !target->b;
        value->type = ValueType_Boolean;
        return true;
    }
    return false;
}

static inline bool
EvaluateNumberExpression(const Value* left,
                         const Value* right,
                         const State* state,
                         const Allocator* allocator,
                         const NumberOperator op,
                         pValue value)
{
    if (ValueTypesMatch(left, ValueType_Number, right, ValueType_Number)) {
        value->type = ValueType_Number;
        value->rangedNumber.number = ApplyNumberOperator(
          &left->rangedNumber.number, &right->rangedNumber.number, op);
        value->rangedNumber.hasRange = false;
        return true;
    }
    if (ValueTypesMatch(left, ValueType_Flag, right, ValueType_Flag) &&
        TemLangStringsAreEqual(&left->flagValue.name, &right->flagValue.name)) {
        switch (op) {
            case NumberOperator_Add:
                value->type = ValueType_Flag;
                if (!TemLangStringListCopy(&value->flagValue.members,
                                           &left->flagValue.members,
                                           allocator)) {
                    return false;
                }
                for (size_t i = 0; i < right->flagValue.members.used; ++i) {
                    const TemLangString* s =
                      &right->flagValue.members.buffer[i];
                    if (!TemLangStringListFindIf(
                          &value->flagValue.members,
                          (TemLangStringListFindFunc)TemLangStringsAreEqual,
                          s,
                          NULL,
                          NULL)) {
                        TemLangStringListAppend(&value->flagValue.members, s);
                    }
                }
                break;
            case NumberOperator_Subtract:
                value->type = ValueType_Flag;
                if (!TemLangStringListCopy(&value->flagValue.members,
                                           &left->flagValue.members,
                                           allocator)) {
                    return false;
                }
                for (size_t i = 0; i < right->flagValue.members.used; ++i) {
                    const TemLangString* s =
                      &right->flagValue.members.buffer[i];
                    size_t index = 0;
                    while (TemLangStringListFindIf(
                      &value->flagValue.members,
                      (TemLangStringListFindFunc)TemLangStringsAreEqual,
                      s,
                      NULL,
                      &index)) {
                        TemLangStringListSwapRemove(&value->flagValue.members,
                                                    index);
                    }
                }
                break;
            default:
                TemLangError("Expected '+' or '-' operator on flags. Got '%c'",
                             NumberOperatorToChar(op));
                return false;
        }
        return TemLangStringCopy(
          &value->flagValue.name, &left->flagValue.name, allocator);
    }
    switch (op) {
        case NumberOperator_Add:
            return EvaluateAddExpression(left, right, state, allocator, value);
        case NumberOperator_Multiply: {
            const Value* target = NULL;
            uint64_t count = 0;
            if (left->type == ValueType_Number) {
                if (!ValueToIndex(left, &count)) {
                    return false;
                }
                target = right;
            } else if (right->type == ValueType_Number) {
                if (!ValueToIndex(right, &count)) {
                    return false;
                }
                target = left;
            } else {
                TemLangError(
                  "Expected on operand to be a number. Got '%s' and '%s'",
                  ValueTypeToString(left->type),
                  ValueTypeToString(right->type));
                return false;
            }
            value->type = ValueType_List;
            value->list.allocator = allocator;
            value->list.isArray = true;
            value->list.exampleValue = allocator->allocate(sizeof(Value));
            value->list.values.allocator = allocator;
            if (!ValueCopy(value->list.exampleValue, target, allocator)) {
                return false;
            }
            for (uint64_t i = 0; i < count; ++i) {
                if (!ValueListAppend(&value->list.values, target)) {
                    return false;
                }
            }
            return value->list.values.used == count;
        } break;
        default:
            break;
    }
    const Value* target = getUnaryValue(left, right);
    if (target == NULL || target->type != ValueType_Number) {
        return false;
    }
    if (op == NumberOperator_Subtract) {
        Number n = { .type = NumberType_Signed, .i = -1L };
        value->type = ValueType_Number;
        value->rangedNumber.number = ApplyNumberOperator(
          &target->rangedNumber.number, &n, NumberOperator_Multiply);
        value->rangedNumber.hasRange = false;
        return true;
    }
    return false;
}

static inline bool
EvaluateComparisonExpression(const Value* left,
                             const Value* right,
                             const ComparisonOperator op,
                             pValue value)
{
    if (op == ComparisonOperator_EqualTo) {
        value->b = ValuesMatch(left, right);
        value->type = ValueType_Boolean;
        return true;
    }
    if (ValueTypesMatch(left, ValueType_Number, right, ValueType_Number)) {
        value->type = ValueType_Boolean;
        value->b = op == NumberCompare(&left->rangedNumber.number,
                                       &right->rangedNumber.number);
        return true;
    }
    if (ValueTypesMatch(left, ValueType_String, right, ValueType_String)) {
        value->type = ValueType_Boolean;
        value->b = op == TemLangStringCompare(&left->string, &right->string);
        return true;
    }
    return false;
}

static inline bool
EvaluateGetExpression(const Value*,
                      const Value*,
                      const State*,
                      const Allocator*,
                      const GetOperator,
                      pValue);
static inline bool
EvaluateFunctionExpression(const Value*,
                           const TemLangString*,
                           const Value*,
                           const State*,
                           const Allocator*,
                           pValue);

static inline bool
EvaluateBinaryExpression(const Expression* eLeft,
                         const Operator* op,
                         const Expression* eRight,
                         const State* state,
                         const Allocator* allocator,
                         pValue value)
{
    bool result = false;
    Value left = { 0 };
    Value right = { 0 };
    if (!EvaluateExpression(eLeft, state, &left, allocator) ||
        !EvaluateExpression(eRight, state, &right, allocator)) {
        goto cleanup;
    }
    switch (op->type) {
        case OperatorType_Boolean:
            result = EvaluateBooleanExpression(
              &left, &right, op->booleanOperator, value);
            break;
        case OperatorType_Number:
            result = EvaluateNumberExpression(
              &left, &right, state, allocator, op->numberOperator, value);
            break;
        case OperatorType_Comparison:
            result = EvaluateComparisonExpression(
              &left, &right, op->comparisonOperator, value);
            break;
        case OperatorType_Get:
            result = EvaluateGetExpression(
              &left, &right, state, allocator, op->getOperator, value);
            break;
        case OperatorType_Function:
            result = EvaluateFunctionExpression(
              &left, &op->functionCall, &right, state, allocator, value);
            break;
        default:
            break;
    }
cleanup:
    ValueFree(&left);
    ValueFree(&right);
    return result;
}

static inline void
EvaluateExpressionError(const Expression* e)
{
    TemLangError("Failed to evaluate '%s' expression",
                 ExpressionTypeToString(e->type));
}

static inline bool
StructMemberToExpression(const StructMember* m,
                         pExpression e,
                         const Allocator* allocator)
{
    if (m->isKeyword) {
        Token token = { .type = TokenType_Keyword, .keyword = m->keyword };
        return TokenToExpression(&token, e, allocator);
    } else {
        e->type = ExpressionType_Binary;
        e->op.type = OperatorType_Get;
        e->op.getOperator = GetOperator_Type;
        e->expressionAllocator = allocator;
        e->left = allocator->allocate(sizeof(Expression));
        e->left->type = ExpressionType_Nullary;
        e->right = allocator->allocate(sizeof(Expression));
        e->right->type = ExpressionType_UnaryValue;
        e->right->value.type = ValueType_String;
        return TemLangStringCopy(
          &e->right->value.string, &m->typeName, allocator);
    }
}

DEFAULT_MAKE_LIST_FUNCTIONS(Expression);