#pragma once

#include "Allocator.h"
#include "List.h"
#include "Value.h"
#include "VariableType.h"

typedef struct Variable
{
    Value value;
    VariableType type;
} Variable, *pVariable;

static inline TemLangString
VariableToString(const Variable* var, const Allocator* allocator)
{
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"type\": \"%s\", \"value\": ",
                              VariableTypeToString(var->type));
    TemLangString n = ValueToString(&var->value, allocator);
    TemLangStringAppend(&s, &n);
    TemLangStringFree(&n);
    TemLangStringAppendChar(&s, '}');
    return s;
}

static inline void
VariableFree(Variable* v)
{
    ValueFree(&v->value);
    memset(v, 0, sizeof(Variable));
}

static inline bool
VariableCopy(Variable* dest, const Variable* src, const Allocator* allocator)
{
    VariableFree(dest);
    dest->type = src->type;
    return ValueCopy(&dest->value, &src->value, allocator);
}

typedef struct NamedValue
{
    TemLangString name;
    Value value;
} NamedValue, *pNamedValue;

static inline TemLangString
NamedValueToString(const NamedValue* n, const Allocator* allocator)
{
    TemLangString a = ValueToString(&n->value, allocator);
    TemLangStringCreateFormat(s,
                              allocator,
                              "{ \"name\": \"%s\", \"value\": %s }",
                              n->name.buffer,
                              a.buffer);
    TemLangStringFree(&a);
    return s;
}

static inline ComparisonOperator
NamedValueCompareNameToString(const NamedValue* a, const TemLangString* b)
{
    return TemLangStringCompare(&a->name, b);
}

static inline bool
NamedValueNameEqualsString(const NamedValue* a, const TemLangString* b)
{
    return NamedValueCompareNameToString(a, b) == ComparisonOperator_EqualTo;
}

static inline ComparisonOperator
NamedValueCompareName(const NamedValue* a, const NamedValue* b)
{
    return NamedValueCompareNameToString(a, &b->name);
}

static inline bool
NamedValueNameEquals(const NamedValue* a, const NamedValue* b)
{
    return NamedValueCompareName(a, b) == ComparisonOperator_EqualTo;
}

#define CREATE_VALUE_INDEX(isConst)                                            \
    static inline isConst Value* Value##isConst##Index(                        \
      const State* state, isConst Value* value, const Value* indexer)          \
    {                                                                          \
        switch (indexer->type) {                                               \
            case ValueType_Number: {                                           \
                if (value->type != ValueType_List) {                           \
                    TemLangError(                                              \
                      "Only lists can be indexed with a number. Got '%s'",     \
                      ValueTypeToString(value->type));                         \
                    break;                                                     \
                }                                                              \
                uint64_t index = 0;                                            \
                switch (indexer->rangedNumber.number.type) {                   \
                    case NumberType_Unsigned:                                  \
                        index = indexer->rangedNumber.number.u;                \
                        break;                                                 \
                    case NumberType_Signed:                                    \
                        if (indexer->rangedNumber.number.i < 0) {              \
                            TemLangError("Cannot index with a negative "       \
                                         "number. Got %" PRId64,               \
                                         indexer->rangedNumber.number.i);      \
                            return NULL;                                       \
                        }                                                      \
                        index = (uint64_t)indexer->rangedNumber.number.i;      \
                        break;                                                 \
                    default:                                                   \
                        TemLangError(                                          \
                          "Cannot index with a floating point number. Got %f", \
                          indexer->rangedNumber.number.d);                     \
                        return NULL;                                           \
                }                                                              \
                if (value->list.values.used <= index) {                        \
                    TemLangError("Index out of range. Length: %" PRIu64        \
                                 "; Index = %" PRIu64,                         \
                                 value->list.values.used,                      \
                                 index);                                       \
                    break;                                                     \
                }                                                              \
                return &value->list.values.buffer[index];                      \
            } break;                                                           \
            case ValueType_String:                                             \
                switch (value->type) {                                         \
                    case ValueType_Struct:                                     \
                        for (size_t i = 0; i < value->structValues->used;      \
                             ++i) {                                            \
                            pNamedValue v = &value->structValues->buffer[i];   \
                            switch (TemLangStringCompare(&v->name,             \
                                                         &indexer->string)) {  \
                                case ComparisonOperator_EqualTo:               \
                                    return &v->value;                          \
                                default:                                       \
                                    break;                                     \
                            }                                                  \
                        }                                                      \
                        TemLangError("Failed to find member '%s' in struct",   \
                                     indexer->string.buffer);                  \
                        break;                                                 \
                    default:                                                   \
                        TemLangError("Only structs can be indexed with a "     \
                                     "string. Got '%s'",                       \
                                     ValueTypeToString(value->type));          \
                        break;                                                 \
                }                                                              \
                break;                                                         \
            case ValueType_Enum: {                                             \
                const StateFindArgs args = { .log = true,                      \
                                             .searchParent = true };           \
                const Atom* atom = StateFindAtomConst(                         \
                  state, &indexer->enumValue.name, AtomType_Enum, args);       \
                if (atom == NULL) {                                            \
                    break;                                                     \
                }                                                              \
                size_t index = 0;                                              \
                if (!TemLangStringListFindIf(                                  \
                      &atom->enumDefinition.members,                           \
                      (TemLangStringListFindFunc)TemLangStringsAreEqual,       \
                      &indexer->enumValue.value,                               \
                      NULL,                                                    \
                      &index)) {                                               \
                    TemLangError("Failed to find enum member '%s'",            \
                                 indexer->enumValue.value.buffer);             \
                    break;                                                     \
                }                                                              \
                const Value newIndex = { .type = ValueType_Number,             \
                                         .rangedNumber = {                     \
                                           .hasRange = false,                  \
                                           .number = NumberFromInt(index) } }; \
                return Value##isConst##Index(state, value, &newIndex);         \
            } break;                                                           \
            default:                                                           \
                TemLangError("Only strings, numbers, or enums can be used to " \
                             "index values. Got '%s'",                         \
                             ValueTypeToString(indexer->type));                \
                break;                                                         \
        }                                                                      \
        return NULL;                                                           \
    }

DEFAULT_MAKE_LIST_FUNCTIONS(NamedValue);

typedef struct State State, *pState;

static inline Value*
ValueIndex(const State*, Value*, const Value*);

static inline const Value*
ValueconstIndex(const State*, const Value*, const Value*);

static inline bool
NamedValueToStructMember(const NamedValue*,
                         const State*,
                         pStructMember,
                         const Allocator*,
                         const bool log);

static inline bool
NamedValuesToStructMemberList(const NamedValueList* values,
                              const State* state,
                              pStructMemberList list,
                              const bool log)
{
    bool result = true;
    for (size_t i = 0; result && i < values->used; ++i) {
        StructMember m = { 0 };
        result = NamedValueToStructMember(
          &values->buffer[i], state, &m, list->allocator, log);
        if (result) {
            StructMemberListAppend(list, &m);
        }
        StructMemberFree(&m);
    }
    return result;
}