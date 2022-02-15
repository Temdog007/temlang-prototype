#pragma once

#include "CType.h"
#include "ComparisonOperator.h"
#include "Number.h"
#include "NumberOperator.h"

#include <float.h>

typedef struct Range
{
    Number min, max;
} Range, *pRange;

static inline TemLangString
RangeToString(const Range* range, const Allocator* allocator)
{
    TemLangString min = NumberToString(&range->min, allocator);
    TemLangString max = NumberToString(&range->max, allocator);
    TemLangStringCreateFormat(
      s, allocator, "{ \"min\": %s, \"max\": %s }", min.buffer, max.buffer);
    TemLangStringFree(&min);
    TemLangStringFree(&max);
    return s;
}

static inline bool
numberInRange(const Number* number, const Range* range)
{
    if (number->type == NumberType_Float &&
        range->min.type != NumberType_Float &&
        range->max.type != NumberType_Float) {
        return false;
    }
    switch (NumberCompare(&range->min, number)) {
        case ComparisonOperator_GreaterThan:
            return false;
        default:
            switch (NumberCompare(number, &range->max)) {
                case ComparisonOperator_GreaterThan:
                    return false;
                default:
                    break;
            }
            break;
    }
    return true;
}

static inline void
numberNotInRangeError(const Number* n,
                      const Range* r,
                      const Allocator* allocator)
{
    TemLangString a = NumberToString(n, allocator);
    TemLangString b = RangeToString(r, allocator);
    TemLangError("Number '%s' is not in range '%s'", a.buffer, b.buffer);
    TemLangStringFree(&a);
    TemLangStringFree(&b);
}

typedef struct RangedNumber
{
    Range range;
    Number number;
    bool hasRange;
} RangedNumber, *pRangedNumber;

static inline TemLangString
RangedNumberToString(const RangedNumber* r, const Allocator* allocator)
{
    if (r->hasRange) {
        TemLangString rStr = RangeToString(&r->range, allocator);
        TemLangString value = NumberToString(&r->number, allocator);
        TemLangStringCreateFormat(s,
                                  allocator,
                                  "{ \"range\": %s, \"number\": %s }",
                                  rStr.buffer,
                                  value.buffer);
        TemLangStringFree(&rStr);
        TemLangStringFree(&value);
        return s;
    }

    return NumberToString(&r->number, allocator);
}

static inline bool
RangedNumberValid(const RangedNumber* r)
{
    return r->hasRange ? numberInRange(&r->number, &r->range) : true;
}

static inline Number
NumberClampToRange(const Number* n, const Range* r)
{
    Number rval =
      NumberCompare(&r->min, n) == ComparisonOperator_GreaterThan ? r->min : *n;
    return NumberCompare(&rval, &r->max) == ComparisonOperator_GreaterThan
             ? r->max
             : rval;
}

static inline CType
RangeToCType(const Range* r)
{
    if (r->min.type == NumberType_Float || r->max.type == NumberType_Float) {
        const double min = NumberToDouble(&r->min);
        const double max = NumberToDouble(&r->max);
        return fabs(min) > FLT_MAX || fabs(max) > FLT_MAX ? CType_f64
                                                          : CType_f32;
    }
    if (NumberToCType(&r->max) == CType_u64) {
        return CType_u64;
    }
    const int64_t min = NumberToInt(&r->min);
    const int64_t max = NumberToInt(&r->max);
    struct Data
    {
        int64_t min;
        int64_t max;
        CType type;
    };
    const struct Data data[] = {
        { .min = 0L, .max = UINT8_MAX, .type = CType_u8 },
        { .min = 0L, .max = UINT16_MAX, .type = CType_u16 },
        { .min = 0L, .max = UINT32_MAX, .type = CType_u32 },
        { .min = INT8_MIN, .max = INT8_MAX, .type = CType_i8 },
        { .min = INT16_MIN, .max = INT16_MAX, .type = CType_i16 },
        { .min = INT32_MIN, .max = INT32_MAX, .type = CType_i32 },
        { .min = INT64_MIN, .max = INT64_MAX, .type = CType_i64 },
    };
    for (size_t i = 0; i < sizeof(data) / sizeof(struct Data); ++i) {
        struct Data d = data[i];
        if (d.min <= min && max <= d.max) {
            return d.type;
        }
    }
    return min < 0 ? CType_i64 : CType_u64;
}

static inline const char*
CTypeToTypeString(const CType t)
{
    switch (t) {
        case CType_bool:
            return "boolean";
        case CType_i8:
            return "int8_t";
        case CType_i16:
            return "int16_t";
        case CType_i32:
            return "int32_t";
        case CType_i64:
            return "int64_t";
        case CType_u8:
            return "uint8_t";
        case CType_u16:
            return "uint16_t";
        case CType_u32:
            return "uint32_t";
        case CType_u64:
            return "uint64_t";
        case CType_string:
            return "TemLangString";
        case CType_f32:
            return "float";
        case CType_f64:
            return "double";
        default:
            return "NullValue";
    }
}

static inline Keyword
CTypeToKeyword(const CType t)
{
    switch (t) {
        case CType_bool:
            return Keyword_bool;
        case CType_i8:
            return Keyword_i8;
        case CType_i16:
            return Keyword_i16;
        case CType_i32:
            return Keyword_i32;
        case CType_i64:
            return Keyword_i64;
        case CType_u8:
            return Keyword_u8;
        case CType_u16:
            return Keyword_u16;
        case CType_u32:
            return Keyword_u32;
        case CType_u64:
            return Keyword_u64;
        case CType_f32:
            return Keyword_f32;
        case CType_f64:
            return Keyword_f64;
        case CType_string:
            return Keyword_string;
        case CType_ptr:
            return Keyword_external;
        default:
            return Keyword_Null;
    }
}

static inline Range
RangeFromCType(CType ctype)
{
    Range range = { 0 };
    switch (ctype) {
        case CType_f32:

        default:
            break;
    }
    return range;
}