#pragma once

#include "CType.h"
#include "Keyword.h"
#include "Range.h"
#include "TemLangString.h"

#include <stdbool.h>
#include <stddef.h>

typedef struct StructMember
{
    TemLangString name;
    bool isKeyword;
    union
    {
        TemLangString typeName;
        Keyword keyword;
    };
    size_t quantity;
} StructMember, *pStructMember;

typedef struct TempStructMember
{
    const char* name;
    size_t nameLength;
    bool isKeyword;
    union
    {
        struct
        {
            const char* typeName;
            size_t typeNameLength;
        };
        Keyword keyword;
    };
    size_t quantity;
} TempStructMember, *pTempStructMember;

static inline StructMember
ToRealStructMember(const TempStructMember* m, const Allocator* allocator)
{
    StructMember r = { 0 };
    if (m->name != NULL) {
        r.name =
          TemLangStringCreateFromSize(m->name, m->nameLength + 1, allocator);
    }
    r.isKeyword = m->isKeyword;
    if (m->isKeyword) {
        r.keyword = m->keyword;
    } else {
        r.typeName = TemLangStringCreateFromSize(
          m->typeName, m->typeNameLength + 1, allocator);
    }
    r.quantity = m->quantity;
    return r;
}

static inline CType
KeywordToCType(const Keyword k)
{
    switch (k) {
        case Keyword_bool:
            return CType_bool;
        case Keyword_i8:
            return CType_i8;
        case Keyword_i16:
            return CType_i16;
        case Keyword_i32:
            return CType_i32;
        case Keyword_i64:
            return CType_i64;
        case Keyword_u8:
            return CType_u8;
        case Keyword_u16:
            return CType_u16;
        case Keyword_u32:
            return CType_u32;
        case Keyword_u64:
            return CType_u64;
        case Keyword_f32:
            return CType_f32;
        case Keyword_f64:
            return CType_f64;
        case Keyword_string:
            return CType_string;
        default:
            return CType_ptr;
    }
}

typedef struct Expression Expression, *pExpression;
static inline bool
StructMemberToExpression(const StructMember*, pExpression, const Allocator*);

static inline ComparisonOperator
StructMemberCompareNameToString(const StructMember* a, const TemLangString* b)
{
    return TemLangStringCompare(&a->name, b);
}

static inline bool
StructMemberNameEquals(const StructMember* a, const TemLangString* b)
{
    return StructMemberCompareNameToString(a, b) == ComparisonOperator_EqualTo;
}

static inline ComparisonOperator
StructMemberCompareTypeNameToString(const StructMember* a,
                                    const TemLangString* b)
{
    return a->isKeyword ? ComparisonOperator_LessThan
                        : TemLangStringCompare(&a->typeName, b);
}

static inline bool
StructMemberTypeNameEqualsToString(const StructMember* a,
                                   const TemLangString* b)
{
    return StructMemberCompareTypeNameToString(a, b) ==
           ComparisonOperator_EqualTo;
}

static inline ComparisonOperator
StructMemberCompareName(const StructMember* a, const StructMember* b)
{
    return StructMemberCompareNameToString(a, &b->name);
}

static inline bool
StructMembersNameEquals(const StructMember* a, const StructMember* b)
{
    return StructMemberCompareNameToString(a, &b->name) ==
           ComparisonOperator_EqualTo;
}

static inline void
StructMemberFree(StructMember* m)
{
    TemLangStringFree(&m->name);
    if (!m->isKeyword) {
        TemLangStringFree(&m->typeName);
    }
    memset(m, 0, sizeof(StructMember));
}

static inline bool
StructMemberCopy(StructMember* dest,
                 const StructMember* src,
                 const Allocator* allocator)
{
    StructMemberFree(dest);
    if (!TemLangStringCopy(&dest->name, &src->name, allocator)) {
        return false;
    }
    dest->isKeyword = src->isKeyword;
    dest->quantity = src->quantity;
    if (src->isKeyword) {
        dest->keyword = src->keyword;
        return true;
    } else {
        return TemLangStringCopy(&dest->typeName, &src->typeName, allocator);
    }
}

static inline TemLangString
StructMemberToString(const StructMember* m, const Allocator* allocator)
{
    const char* a = NULL;
    if (m->isKeyword) {
        a = KeywordToString(m->keyword);
    } else {
        a = m->typeName.buffer;
    }
    TemLangStringCreateFormat(
      s,
      allocator,
      "{ \"name\": \"%s\", \"quantity\": %zu, \"type\": \"%s\" }",
      m->name.buffer,
      m->quantity,
      a);
    return s;
}

MAKE_LIST(StructMember);
DEFAULT_MAKE_LIST_FUNCTIONS(StructMember);

static inline TemLangString
StructMemberToTypeName(const StructMember* m, const Allocator* allocator)
{
    if (m->isKeyword) {
        return TemLangStringCreate(
          CTypeToTypeString(KeywordToCType(m->keyword)), allocator);
    } else {
        return TemLangStringClone(&m->typeName, allocator);
    }
}

typedef struct State State, *pState;
static inline bool
StructMemberToRange(const StructMember* m, const State*, pRange);

static inline bool
KeywordToRange(const Keyword keyword, pRange);