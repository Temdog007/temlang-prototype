#pragma once

#include "Allocator.h"
#include "BooleanOperator.h"
#include "ComparisonOperator.h"
#include "GetOperator.h"
#include "List.h"
#include "NumberOperator.h"
#include "OperatorType.h"
#include "TemLangString.h"

typedef struct Operator
{
    OperatorType type;
    union
    {
        BooleanOperator booleanOperator;
        ComparisonOperator comparisonOperator;
        NumberOperator numberOperator;
        TemLangString functionCall;
        GetOperator getOperator;
    };
} Operator, *pOperator;

static inline TemLangString
OperatorString(const Operator* op, const Allocator* allocator)
{
    const char* t = NULL;
    switch (op->type) {
        case OperatorType_Boolean:
            t = BooleanOperatorToString(op->booleanOperator);
            break;
        case OperatorType_Comparison:
            t = ComparisonOperatorToString(op->comparisonOperator);
            break;
        case OperatorType_Number:
            t = NumberOperatorToString(op->numberOperator);
            break;
        case OperatorType_Function:
            t = op->functionCall.buffer;
            break;
        case OperatorType_Get:
            t = GetOperatorToString(op->getOperator);
            break;
        default:
            break;
    }
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"type\": \"%s\", \"value\": \"%s\" }",
                              OperatorTypeToString(op->type),
                              t);
    return s;
}

static inline int
OperatorPrecedence(const Operator* o)
{
    switch (o->type) {
        case OperatorType_Get:
            switch (o->getOperator) {
                case GetOperator_Type:
                    return 41;
                case GetOperator_Member:
                    return 42;
                case GetOperator_MakeList:
                    return 43;
                case GetOperator_Length:
                    return 44;
                default:
                    break;
            }
            break;
        case OperatorType_Comparison:
            switch (o->comparisonOperator) {
                case ComparisonOperator_EqualTo:
                    return 31;
                case ComparisonOperator_GreaterThan:
                    return 32;
                case ComparisonOperator_LessThan:
                    return 33;
                default:
                    break;
            }
            break;
        case OperatorType_Boolean:
            switch (o->booleanOperator) {
                case BooleanOperator_Or:
                    return 21;
                case BooleanOperator_Xor:
                    return 22;
                case BooleanOperator_And:
                    return 23;
                case BooleanOperator_Not:
                    return 24;
                default:
                    break;
            }
            break;
        case OperatorType_Number:
            switch (o->numberOperator) {
                case NumberOperator_Add:
                case NumberOperator_Subtract:
                    return 11;
                case NumberOperator_Multiply:
                case NumberOperator_Divide:
                    return 12;
                case NumberOperator_Modulo:
                    return 13;
                default:
                    break;
            }
            break;
        case OperatorType_Function:
        default:
            break;
    }
    return 0;
}

static inline void
OperatorFree(Operator* op)
{
    switch (op->type) {
        case OperatorType_Function:
            TemLangStringFree(&op->functionCall);
            break;
        default:
            break;
    }
    memset(op, 0, sizeof(Operator));
}

static inline bool
OperatorCopy(Operator* dest, const Operator* src, const Allocator* allocator)
{
    OperatorFree(dest);
    dest->type = src->type;
    switch (src->type) {
        case OperatorType_Number:
            dest->numberOperator = src->numberOperator;
            break;
        case OperatorType_Get:
            dest->getOperator = src->getOperator;
            break;
        case OperatorType_Boolean:
            dest->booleanOperator = src->booleanOperator;
            break;
        case OperatorType_Comparison:
            dest->comparisonOperator = src->comparisonOperator;
            break;
        case OperatorType_Function:
            return TemLangStringCopy(
              &dest->functionCall, &src->functionCall, allocator);
        default:
            copyFailure(OperatorTypeToString(src->type));
            return false;
    }
    return true;
}

MAKE_LIST(Operator)

static inline bool
ApplyBooleanOperator(bool a, bool b, BooleanOperator op)
{
    switch (op) {
        case BooleanOperator_And:
            return a && b;
        case BooleanOperator_Or:
            return a || b;
        case BooleanOperator_Xor:
            return a ^ b;
        case BooleanOperator_Not:
            return a != b;
        default:
            return false;
    }
}

DEFAULT_MAKE_LIST_FUNCTIONS(Operator);