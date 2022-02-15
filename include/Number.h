#pragma once

#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>

#include "Allocator.h"
#include "CType.h"
#include "ComparisonOperator.h"
#include "NumberOperator.h"
#include "NumberType.h"
#include "TemLangString.h"

typedef struct Number
{
    NumberType type;
    union
    {
        int64_t i;
        uint64_t u;
        double d;
    };
} Number, *pNumber;

static inline Number
NumberFromDouble(const double d)
{
    Number n = { .type = NumberType_Float, .d = d };
    return n;
}

static inline Number
NumberFromInt(const int64_t i)
{
    Number n = { .type = NumberType_Signed, .i = i };
    return n;
}

static inline Number
NumberFromUInt(const uint64_t u)
{
    Number n = { .type = NumberType_Unsigned, .u = u };
    return n;
}

static inline TemLangString
NumberToString(const Number* n, const Allocator* a)
{
    switch (n->type) {
        case NumberType_Signed: {
            TemLangStringCreateFormat(newS, a, "%" PRId64, n->i);
            return newS;
        } break;
        case NumberType_Unsigned: {
            TemLangStringCreateFormat(newS, a, "%" PRIu64, n->u);
            return newS;
        } break;
        default: {
            TemLangStringCreateFormat(newS, a, "%0.6f", n->d);
            return newS;
        } break;
    }
}

static inline double
NumberToDouble(const Number* a)
{
    switch (a->type) {
        case NumberType_Signed:
            return (double)a->i;
        case NumberType_Unsigned:
            return (double)a->u;
        default:
            return a->d;
    }
}

static inline int64_t
NumberToInt(const Number* n)
{
    switch (n->type) {
        case NumberType_Signed:
            return n->i;
        case NumberType_Unsigned:
            return (int64_t)n->u;
        default:
            return (int64_t)n->d;
    }
}

static inline uint64_t
NumberToUInt(const Number* n)
{
    switch (n->type) {
        case NumberType_Signed:
            return (uint64_t)n->i;
        case NumberType_Unsigned:
            return n->u;
        default:
            return (uint64_t)n->d;
    }
}

static inline bool
NumberTryToUInt(const Number* n, uint64_t* value)
{
    switch (n->type) {
        case NumberType_Signed:
            if (n->i >= 0L) {
                *value = (uint64_t)n->i;
                return true;
            }
            break;
        case NumberType_Unsigned:
            *value = n->u;
            return true;
        default:
            if (n->d >= 0.f) {
                *value = (uint64_t)n->d;
                return true;
            }
            break;
    }
    return false;
}

#define RUN_COMPARISON(a, b)                                                   \
    if (a < b) {                                                               \
        return ComparisonOperator_LessThan;                                    \
    } else if (a > b) {                                                        \
        return ComparisonOperator_GreaterThan;                                 \
    }                                                                          \
    return ComparisonOperator_EqualTo;

static inline ComparisonOperator
NumberCompare(const Number* a, const Number* b)
{
    if (a->type != b->type) {
        double ad = NumberToDouble(a);
        double bd = NumberToDouble(b);
        RUN_COMPARISON(ad, bd);
    }

    switch (a->type) {
        case NumberType_Signed:
            RUN_COMPARISON(a->i, b->i);
        case NumberType_Unsigned:
            RUN_COMPARISON(a->u, b->u);
        default:
            RUN_COMPARISON(a->d, b->d);
    }
}

#define DEFAULT_MODULO(a, b) (a % b)

#define APPLY_NUMBER_OP(a, b, n, modulo, op)                                   \
    switch (op) {                                                              \
        case NumberOperator_Add:                                               \
            n = a + b;                                                         \
            break;                                                             \
        case NumberOperator_Subtract:                                          \
            n = a - b;                                                         \
            break;                                                             \
        case NumberOperator_Multiply:                                          \
            n = a * b;                                                         \
            break;                                                             \
        case NumberOperator_Divide:                                            \
            n = a / b;                                                         \
            break;                                                             \
        default:                                                               \
            n = modulo(a, b);                                                  \
            break;                                                             \
    }

static inline Number
ApplyNumberOperator(const Number* a, const Number* b, NumberOperator op)
{
    Number n = { 0 };
    if (a->type == b->type) {
        switch (a->type) {
            case NumberType_Signed:
                n.type = NumberType_Signed;
                APPLY_NUMBER_OP(a->i, b->i, n.i, DEFAULT_MODULO, op);
                break;
            case NumberType_Unsigned:
                if (op == NumberOperator_Subtract && b->u > a->u) {
                    n.type = NumberType_Signed;
                    APPLY_NUMBER_OP(
                      (int64_t)a->u, (int64_t)b->u, n.i, DEFAULT_MODULO, op);
                } else {
                    n.type = NumberType_Unsigned;
                    APPLY_NUMBER_OP(a->u, b->u, n.u, DEFAULT_MODULO, op);
                }
                break;
            case NumberType_Float:
            default:
                n.type = NumberType_Float;
                APPLY_NUMBER_OP(a->d, b->d, n.d, fmod, op);
                break;
        }
    } else if (a->type == NumberType_Float || b->type == NumberType_Float) {
        const double ad = NumberToDouble(a);
        const double bd = NumberToDouble(b);
        n.type = NumberType_Float;
        APPLY_NUMBER_OP(ad, bd, n.d, fmod, op);
    } else {
        const int64_t ad = a->type == NumberType_Signed ? a->i : (int64_t)a->u;
        const int64_t bd = b->type == NumberType_Signed ? b->i : (int64_t)b->u;
        n.type = NumberType_Signed;
        APPLY_NUMBER_OP(ad, bd, n.i, DEFAULT_MODULO, op);
    }
    return n;
}

static inline CType
NumberToCType(const Number* n)
{
    if (n->type == NumberType_Float) {
        const double d = NumberToDouble(n);
        return fabs(d) < FLT_MAX ? CType_f32 : CType_f64;
    }
    if (n->type == NumberType_Unsigned && n->u > INT64_MAX) {
        return CType_u64;
    }
    const int64_t value = NumberToInt(n);
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
        { .min = INT64_MIN, .max = INT64_MAX, .type = CType_i64 }
    };
    for (size_t i = 0; i < sizeof(data) / sizeof(struct Data); ++i) {
        struct Data d = data[i];
        if (d.min <= value && value <= d.max) {
            return d.type;
        }
    }
    return value < 0 ? CType_i64 : CType_u64;
}