#pragma once

#if TEMLANG_USE_CGLM

#include "Allocator.h"
#include "Serialize.h"

#include <cglm/vec2.h>
#include <cglm/vec3.h>
#include <cglm/vec4.h>

#include <cglm/mat2.h>
#include <cglm/mat3.h>
#include <cglm/mat4.h>

#define MAKE_CGLM_FUNCTIONS(T, count)                                          \
    static inline void T##Free(T* t) { (void)t; }                              \
    static inline bool T##Copy(T* a, const T* b, const Allocator* t)           \
    {                                                                          \
        (void)t;                                                               \
        T* noConst = (T*)b;                                                    \
        glm_##T##_copy(*noConst, *a);                                          \
        return true;                                                           \
    }                                                                          \
    static inline size_t T##Serialize(                                         \
      const T* value, pBytes bytes, const bool e)                              \
    {                                                                          \
        size_t total = 0;                                                      \
        const float* f = (const float*)&(*value)[0];                           \
        for (size_t i = 0; i < count; ++i) {                                   \
            total += floatSerialize(&f[i], bytes, e);                          \
        }                                                                      \
        return total;                                                          \
    }                                                                          \
    static inline size_t T##Deserialize(                                       \
      T* value, const Bytes* bytes, const size_t offset, const bool e)         \
    {                                                                          \
        size_t total = 0;                                                      \
        float* f = (float*)&(*value)[0];                                       \
        for (size_t i = 0; i < count; ++i) {                                   \
            total += floatDeserialize(&f[i], bytes, total + offset, e);        \
        }                                                                      \
        return total;                                                          \
    }

MAKE_CGLM_FUNCTIONS(vec2, 2);
MAKE_CGLM_FUNCTIONS(vec3, 3);
MAKE_CGLM_FUNCTIONS(vec4, 4);
MAKE_CGLM_FUNCTIONS(mat2, 4);
MAKE_CGLM_FUNCTIONS(mat3, 9);
MAKE_CGLM_FUNCTIONS(mat4, 16);

#endif