#pragma once

#include "Allocator.h"
#include "List.h"
#include "Number.h"
#include "Range.h"
#include "ValueType.h"

typedef struct Value Value, *pValue;
MAKE_LIST(Value);

static inline TemLangString
ValueToString(const Value*, const Allocator*);

typedef struct Variable Variable, *pVariable;

typedef struct NamedValue NamedValue, *pNamedValue;

MAKE_LIST(NamedValue);

static inline TemLangString
NamedValueToString(const NamedValue* n, const Allocator* allocator);

static inline TemLangString
NamedValueListToString(const NamedValueList* list, const Allocator* allocator);

typedef struct EnumValue
{
    TemLangString name;
    TemLangString value;
} EnumValue, *pEnumValue;

static inline void
EnumValueFree(EnumValue* e)
{
    TemLangStringFree(&e->name);
    TemLangStringFree(&e->value);
}

static inline bool
EnumValueCopy(EnumValue* dest, const EnumValue* src, const Allocator* allocator)
{
    EnumValueFree(dest);
    return TemLangStringCopy(&dest->name, &src->name, allocator) &&
           TemLangStringCopy(&dest->value, &src->value, allocator);
}

static inline TemLangString
EnumValueToString(const EnumValue* e, const Allocator* allocator)
{
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"name\": \"%s\", \"value\": \"%s\" }",
                              e->name.buffer,
                              e->value.buffer);
    return s;
}

typedef struct FlagValue
{
    TemLangString name;
    TemLangStringList members;
} FlagValue, *pFlagValue;

static inline void
FlagValueFree(FlagValue* e)
{
    TemLangStringFree(&e->name);
    TemLangStringListFree(&e->members);
}

static inline bool
FlagValueCopy(FlagValue* dest, const FlagValue* src, const Allocator* allocator)
{
    FlagValueFree(dest);
    return TemLangStringListCopy(&dest->members, &src->members, allocator) &&
           TemLangStringCopy(&dest->name, &src->name, allocator);
}

static inline TemLangString
FlagValueToString(const FlagValue* e, const Allocator* allocator)
{
    TemLangString n = TemLangStringListToString(&e->members, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"name\": \"%s\", \"value\": %s }",
                              e->name.buffer,
                              n.buffer);
    TemLangStringFree(&n);
    return s;
}

typedef struct VariantValue
{
    TemLangString name;
    TemLangString memberName;
    pValue value;
    const Allocator* allocator;
} VariantValue, *pVariantValue;

static inline void
VariantValueFree(VariantValue* v)
{
    TemLangStringFree(&v->name);
    TemLangStringFree(&v->memberName);
    if (v->value != NULL) {
        ValueFree(v->value);
        v->allocator->free(v->value);
        v->value = NULL;
    }
}

static inline bool
VariantValueCopy(VariantValue* dest,
                 const VariantValue* src,
                 const Allocator* allocator);

static inline TemLangString
VariantValueToString(const VariantValue* v, const Allocator* allocator)
{
    TemLangString a = v->value == NULL ? TemLangStringCreate("null", allocator)
                                       : ValueToString(v->value, allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"name\": \"%s\", \"member\": \"%s\", \"value\": %s }",
      v->name.buffer,
      v->memberName.buffer,
      a.buffer);
    TemLangStringFree(&a);
    return s;
}

typedef struct ValueListValue
{
    pValue exampleValue;
    const Allocator* allocator;
    ValueList values;
    bool isArray;
} ValueListValue, *pValueListValue;

static inline void
ValueListValueFree(ValueListValue*);

static inline bool
ValueListValueCopy(ValueListValue*, const ValueListValue*, const Allocator*);

static inline TemLangString
ValueListValueString(const ValueListValue*, const Allocator*);

typedef struct Value
{
    ValueType type;
    union
    {
        RangedNumber rangedNumber;
        bool b;
        void* ptr;
        TemLangString string;
        ValueListValue list;
        struct
        {
            pValue fakeValue;
            const Allocator* fakeValueAllocator;
        };
        EnumValue enumValue;
        FlagValue flagValue;
        VariantValue variantValue;
        struct
        {
            pNamedValueList structValues;
            const Allocator* structValuesAllocator;
        };
    };
} Value, *pValue;

static inline void
ValueFree(Value* v)
{
    switch (v->type) {
        case ValueType_String:
        case ValueType_Data:
            TemLangStringFree(&v->string);
            break;
        case ValueType_List:
            ValueListValueFree(&v->list);
            break;
        case ValueType_Type:
            ValueFree(v->fakeValue);
            v->fakeValueAllocator->free(v->fakeValue);
            break;
        case ValueType_Enum:
            EnumValueFree(&v->enumValue);
            break;
        case ValueType_Flag:
            FlagValueFree(&v->flagValue);
            break;
        case ValueType_Struct:
            NamedValueListFree(v->structValues);
            v->structValuesAllocator->free(v->structValues);
            break;
        case ValueType_Variant:
            VariantValueFree(&v->variantValue);
            break;
        default:
            break;
    }
    memset(v, 0, sizeof(Value));
}

static inline bool
ValueCopy(Value* dest, const Value* src, const Allocator* allocator)
{
    ValueFree(dest);
    dest->type = src->type;
    switch (src->type) {
        case ValueType_Null:
            return true;
        case ValueType_Number:
            dest->rangedNumber = src->rangedNumber;
            return true;
        case ValueType_Boolean:
            dest->b = src->b;
            return true;
        case ValueType_External:
            dest->ptr = src->ptr;
            return true;
        case ValueType_Type:
            dest->fakeValueAllocator = allocator;
            dest->fakeValue = allocator->allocate(sizeof(Value));
            return ValueCopy(dest->fakeValue, src->fakeValue, allocator);
        case ValueType_String:
        case ValueType_Data:
            return TemLangStringCopy(&dest->string, &src->string, allocator);
        case ValueType_Flag:
            return FlagValueCopy(&dest->flagValue, &src->flagValue, allocator);
        case ValueType_Enum:
            return EnumValueCopy(&dest->enumValue, &src->enumValue, allocator);
        case ValueType_Struct:
            dest->structValuesAllocator = allocator;
            dest->structValues = allocator->allocate(sizeof(NamedValueList));
            return NamedValueListCopy(
              dest->structValues, src->structValues, allocator);
        case ValueType_Variant:
            return VariantValueCopy(
              &dest->variantValue, &src->variantValue, allocator);
        case ValueType_List:
            return ValueListValueCopy(&dest->list, &src->list, allocator);
        default:
            copyFailure(ValueTypeToString(src->type));
            return false;
    }
}

static inline TemLangString
ValueToString(const Value* v, const Allocator* allocator)
{
    TemLangString n = { 0 };
    switch (v->type) {
        case ValueType_Number:
            n = RangedNumberToString(&v->rangedNumber, allocator);
            break;
        case ValueType_Boolean:
            n = TemLangStringCreate(v->b ? "true" : "false", allocator);
            break;
        case ValueType_External: {
            TemLangStringCreateFormat(
              a, allocator, "\"%p\"", v->ptr == NULL ? "null" : v->ptr);
            n = a;
        } break;
        case ValueType_String:
        case ValueType_Data: {
            TemLangStringCreateFormat(
              n2, allocator, "\"%s\"", v->string.buffer);
            n = n2;
        } break;
        case ValueType_Type:
            n = ValueToString(v->fakeValue, allocator);
            break;
        case ValueType_List: {
            n = ValueListValueString(&v->list, allocator);
        } break;
        case ValueType_Enum:
            n = EnumValueToString(&v->enumValue, allocator);
            break;
        case ValueType_Flag:
            n = FlagValueToString(&v->flagValue, allocator);
            break;
        case ValueType_Struct:
            n = NamedValueListToString(v->structValues, allocator);
            break;
        case ValueType_Variant:
            n = VariantValueToString(&v->variantValue, allocator);
            break;
        default:
            n = TemLangStringCreate("null", allocator);
            break;
    }
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"type\": \"%s\",  \"value\": %s }",
                              ValueTypeToString(v->type),
                              n.buffer);
    TemLangStringFree(&n);
    return s;
}

static inline TemLangString
ValueToSimpleString(const Value* value, const Allocator* allocator)
{
    switch (value->type) {
        case ValueType_Number:
            return NumberToString(&value->rangedNumber.number, allocator);
        case ValueType_String:
            return TemLangStringClone(&value->string, allocator);
        case ValueType_Boolean:
            return TemLangStringCreate(value->b ? "true" : "false", allocator);
        case ValueType_Enum:
            return TemLangStringClone(&value->enumValue.value, allocator);
        case ValueType_Flag: {
            TemLangString s = { .allocator = allocator };
            TemLangStringAppendChar(&s, '[');
            for (size_t i = 0; i < value->flagValue.members.used; ++i) {
                TemLangStringAppend(&s, &value->flagValue.members.buffer[i]);
                if (i != value->flagValue.members.used - 1) {
                    TemLangStringAppendChar(&s, ',');
                }
            }
            TemLangStringAppendChar(&s, ']');
            return s;
        } break;
        case ValueType_Type:
            return ValueToSimpleString(value->fakeValue, allocator);
        default:
            return ValueToString(value, allocator);
    }
}

static inline bool
ValueTypesMatch(const Value* left,
                ValueType leftTarget,
                const Value* right,
                ValueType rightTarget)
{
    return left->type == leftTarget && right->type == rightTarget;
}

static inline bool
ValueTypesTryMatch(const Value* left,
                   ValueType leftTarget,
                   const Value* right,
                   ValueType rightTarget,
                   const Value** actualLeft,
                   const Value** actualRight)
{
    if (ValueTypesMatch(left, leftTarget, right, rightTarget)) {
        *actualLeft = left;
        *actualRight = right;
        return true;
    }
    if (ValueTypesMatch(right, leftTarget, left, rightTarget)) {
        *actualLeft = right;
        *actualRight = left;
        return true;
    }
    return false;
}

static inline const Value*
getUnaryValue(const Value* left, const Value* right)
{
    if (left->type == ValueType_Null) {
        return right->type == ValueType_Null ? NULL : right;
    }
    return right->type == ValueType_Null ? left : NULL;
}

static inline bool
ValueCanTransition(const Value* from,
                   const Value* to,
                   const Allocator* allocator);

static inline bool
ValueTransition(Value* from, const Value* to, const Allocator* allocator)
{
    if (from->type != to->type) {
        return false;
    }

    switch (from->type) {
        case ValueType_Number:
            if (from->rangedNumber.hasRange) {
                if (numberInRange(&to->rangedNumber.number,
                                  &from->rangedNumber.range)) {
                    from->rangedNumber.number = to->rangedNumber.number;
                    return true;
                } else {
                    numberNotInRangeError(&to->rangedNumber.number,
                                          &from->rangedNumber.range,
                                          allocator);
                    return false;
                }
            }
            from->rangedNumber.number = to->rangedNumber.number;
            return true;
        case ValueType_List:
            if (from->list.isArray != to->list.isArray) {
                TemLangError("Type mimatch. Got '%s'; Expected '%s'",
                             from->list.isArray ? "Array" : "List",
                             to->list.isArray ? "Array" : "List");
                return false;
            }
            if (!ValueCanTransition(
                  from->list.exampleValue, to->list.exampleValue, allocator)) {
                return false;
            }
            if (from->list.isArray &&
                from->list.values.used != to->list.values.used) {
                TemLangError(
                  "Quantity mismatch in array. Got %zu; Expected %zu",
                  from->list.values.used,
                  to->list.values.used);
                return false;
            }
            return ValueCopy(from, to, allocator);
        default:
            return ValueCopy(from, to, allocator);
    }
}

static inline bool
ValueCanTransition(const Value* from,
                   const Value* to,
                   const Allocator* allocator)
{
    Value temp = { 0 };
    const bool result = ValueCopy(&temp, from, allocator) &&
                        ValueTransition(&temp, to, allocator);
    ValueFree(&temp);
    return result;
}

static inline bool
ValueTryGetRange(const Value* value, pRange range)
{
    switch (value->type) {
        case ValueType_Number:
            if (value->rangedNumber.hasRange) {
                *range = value->rangedNumber.range;
                return true;
            }
            break;
        case ValueType_Type:
            return ValueTryGetRange(value->fakeValue, range);
        default:
            break;
    }
    return false;
}

static inline bool
ValueHasMembers(const Value* value)
{
    switch (value->type) {
        case ValueType_Struct:
        case ValueType_List:
            return true;
        default:
            return false;
    }
}

static inline bool
ValueToIndex(const Value* value, uint64_t* index)
{
    switch (value->type) {
        case ValueType_Number:
            switch (value->rangedNumber.number.type) {
                case NumberType_Signed:
                    if (value->rangedNumber.number.i < 0) {
                        TemLangError(
                          "Expected positive integer for index. Got %" PRId64,
                          value->rangedNumber.number.i);
                        break;
                    } else {
                        *index = (uint64_t)value->rangedNumber.number.i;
                        return true;
                    }
                case NumberType_Unsigned:
                    *index = value->rangedNumber.number.u;
                    return true;
                case NumberType_Float:
                default:
                    TemLangError("Expected positive integer for index. Got %f",
                                 value->rangedNumber.number.d);
                    break;
            }
            break;
        default:
            TemLangError("Value is not an index");
            break;
    }
    return false;
}

static inline bool
VariantValueCopy(VariantValue* dest,
                 const VariantValue* src,
                 const Allocator* allocator)
{
    VariantValueFree(dest);
    dest->allocator = allocator;
    dest->value = allocator->allocate(sizeof(Value));
    return TemLangStringCopy(&dest->name, &src->name, allocator) &&
           TemLangStringCopy(&dest->memberName, &src->memberName, allocator) &&
           ValueCopy(dest->value, src->value, allocator);
}

static inline void
ValueListValueFree(ValueListValue* v)
{
    ValueListFree(&v->values);
    if (v->exampleValue == NULL) {
        return;
    }
    ValueFree(v->exampleValue);
    v->allocator->free(v->exampleValue);
    memset(v, 0, sizeof(ValueListValue));
}

static inline bool
ValuesMatch(const Value* a, const Value* b)
{
    if (a->type != b->type) {
        const Value* left = NULL;
        const Value* right = NULL;
        if (ValueTypesTryMatch(
              a, ValueType_Enum, b, ValueType_String, &left, &right)) {
            return TemLangStringCompare(&left->enumValue.value,
                                        &right->string) ==
                   ComparisonOperator_EqualTo;
        }
        return false;
    }

    switch (a->type) {
        case ValueType_Null:
            return true;
        case ValueType_Number:
            return NumberCompare(&a->rangedNumber.number,
                                 &b->rangedNumber.number) ==
                   ComparisonOperator_EqualTo;
        case ValueType_Boolean:
            return a->b == b->b;
        case ValueType_String:
            return TemLangStringCompare(&a->string, &b->string) ==
                   ComparisonOperator_EqualTo;
        case ValueType_Enum:
            return TemLangStringCompare(&a->enumValue.name,
                                        &b->enumValue.name) ==
                     ComparisonOperator_EqualTo &&
                   TemLangStringCompare(&b->enumValue.value,
                                        &b->enumValue.value) ==
                     ComparisonOperator_EqualTo;
        case ValueType_Variant:
            return TemLangStringCompare(&a->variantValue.name,
                                        &b->variantValue.name) ==
                     ComparisonOperator_EqualTo &&
                   TemLangStringCompare(&a->variantValue.memberName,
                                        &b->variantValue.memberName) ==
                     ComparisonOperator_EqualTo &&
                   ValuesMatch(a->variantValue.value, b->variantValue.value);
        case ValueType_Type:
            return ValuesMatch(a->fakeValue, b->fakeValue);
        default:
            break;
    }
    return false;
}

static inline bool
ValueListValueCopy(ValueListValue* dest,
                   const ValueListValue* src,
                   const Allocator* allocator)
{
    ValueListValueFree(dest);
    dest->isArray = src->isArray;
    dest->allocator = allocator;
    dest->exampleValue = allocator->allocate(sizeof(Value));
    return ValueCopy(dest->exampleValue, src->exampleValue, allocator) &&
           ValueListCopy(&dest->values, &src->values, allocator);
}

static inline TemLangString
ValueListValueString(const ValueListValue* v, const Allocator* allocator)
{
    LIST_TO_STRING(v->values, n1, ValueToString, allocator);
    TemLangString n2 = ValueToString(v->exampleValue, allocator);
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"values\": %s, \"isArray\": %s, \"example\": %s }",
      n1.buffer,
      v->isArray ? "true" : "false",
      n2.buffer);
    TemLangStringFree(&n1);
    TemLangStringFree(&n2);
    return s;
}

static inline bool
ValueListValueIsValid(const ValueListValue* list, const Allocator* allocator)
{
    const Value* first = &list->values.buffer[0];
    if (first->type == ValueType_Number && !first->rangedNumber.hasRange) {
        TemLangError("The first value of a number list/array must be ranged "
                     "so that the range can be applied to other unranged "
                     "numbers in the list");
        return false;
    }
    for (size_t i = 1; i < list->values.used; ++i) {
        if (!ValueCanTransition(first, &list->values.buffer[i], allocator)) {
            TemLangError(
              "List/array is invalid because the type for values at index 0 "
              "and %zu do not match",
              i);
            return false;
        }
    }
    return true;
}

static inline char
NumberOperatorToChar(const NumberOperator n)
{
    switch (n) {
        case NumberOperator_Add:
            return '+';
        case NumberOperator_Subtract:
            return '-';
        case NumberOperator_Multiply:
            return '*';
        case NumberOperator_Divide:
            return '/';
        case NumberOperator_Modulo:
        default:
            return '%';
    }
}

static inline const char*
BooleanOperatorToChars(const BooleanOperator b)
{
    switch (b) {
        case BooleanOperator_And:
            return "&&";
        case BooleanOperator_Xor:
            return "^";
        case BooleanOperator_Not:
            return "!";
        default:
            return "||";
    }
}

static inline const char*
ComparisonOperatorToChars(const ComparisonOperator c)
{
    switch (c) {
        case ComparisonOperator_EqualTo:
            return "==";
        case ComparisonOperator_LessThan:
            return "<";
        default:
            return ">";
    }
}

static inline bool
ValueToChar(const Value* value, char* c)
{
    if (value->type != ValueType_Number) {
        return false;
    }
    int64_t i = NumberToInt(&value->rangedNumber.number);
    if (INT8_MIN <= i && i <= INT8_MAX) {
        *c = (char)i;
        return true;
    }
    return false;
}

DEFAULT_MAKE_LIST_FUNCTIONS(Value);