#pragma once

#include <stdlib.h>

#include "Random.h"

#if __ANDROID__ || __EMSCRIPTEN__
typedef int (*__compar_fn_t)(const void*, const void*);
#endif

extern int
TemLangError(const char*, ...);

static inline int
copyFailure(const char* s)
{
    return TemLangError("Copy failure for: %s", s);
}

#define MAKE_LIST(T)                                                           \
    typedef struct T##List                                                     \
    {                                                                          \
        T* buffer;                                                             \
        uint32_t used;                                                         \
        uint32_t size;                                                         \
        const Allocator* allocator;                                            \
    } T##List, *p##T##List;                                                    \
    static inline void T##Free(T*);                                            \
    static inline bool T##Copy(T*, const T*, const Allocator*);                \
    static inline void T##ListFree(T##List*);                                  \
    static inline bool T##ListCopy(                                            \
      T##List*, const T##List*, const Allocator*);                             \
    static inline bool T##ListAppend(T##List*, const T* newValue);             \
    static inline T##List T##ListCreate(const Allocator* allocator)            \
    {                                                                          \
        return (T##List){                                                      \
            .buffer = NULL,                                                    \
            .used = 0UL,                                                       \
            .size = 0UL,                                                       \
            .allocator = allocator,                                            \
        };                                                                     \
    }

#define LIST_INIT(a)                                                           \
    {                                                                          \
        .allocator = a, .buffer = NULL, .size = 0UL, .used = 0UL               \
    }

#define MAKE_LIST_FUNCTIONS(T, TCopy, TFree)                                   \
    static inline bool T##ListIsEmpty(const T##List* list)                     \
    {                                                                          \
        return list->buffer == NULL || list->used == 0;                        \
    }                                                                          \
    static inline bool T##ListRellocateIfNeeded(T##List* list)                 \
    {                                                                          \
        if (list->buffer == NULL || list->size == 0) {                         \
            if (list->allocator == NULL) {                                     \
                TemLangError("List has no allocator!");                        \
                return false;                                                  \
            }                                                                  \
            list->size = 16;                                                   \
            T* data = (T*)list->allocator->reallocate(list->buffer,            \
                                                      sizeof(T) * list->size); \
            if (data == NULL) {                                                \
                return false;                                                  \
            }                                                                  \
            memset(data, 0, sizeof(T) * list->size);                           \
            list->buffer = data;                                               \
            list->used = 0;                                                    \
        } else if (list->used >= list->size) {                                 \
            if (list->allocator == NULL) {                                     \
                TemLangError("List has no allocator!");                        \
                return false;                                                  \
            }                                                                  \
            const size_t oldSize = list->size;                                 \
            list->size *= 2;                                                   \
            T* data = (T*)list->allocator->reallocate(list->buffer,            \
                                                      sizeof(T) * list->size); \
            if (data == NULL) {                                                \
                return false;                                                  \
            }                                                                  \
            for (size_t i = oldSize; i < list->size; ++i) {                    \
                memset(&data[i], 0, sizeof(T));                                \
            }                                                                  \
            list->buffer = data;                                               \
        }                                                                      \
        return true;                                                           \
    }                                                                          \
    static inline bool T##ListAppend(T##List* list, const T* newValue)         \
    {                                                                          \
        if (!T##ListRellocateIfNeeded(list)) {                                 \
            return false;                                                      \
        }                                                                      \
        if (TCopy(&list->buffer[list->used], newValue, list->allocator)) {     \
            ++list->used;                                                      \
            return true;                                                       \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline bool T##ListInsert(                                          \
      T##List* list, const size_t index, const T* newValue)                    \
    {                                                                          \
        if (!T##ListAppend(list, newValue)) {                                  \
            return false;                                                      \
        }                                                                      \
        if (index < list->used) {                                              \
            const T temp = list->buffer[index];                                \
            list->buffer[index] = list->buffer[list->used - 1];                \
            list->buffer[list->used - 1] = temp;                               \
            return true;                                                       \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline bool T##ListRemove(                                          \
      T##List* list, const size_t index, const Allocator* allocator)           \
    {                                                                          \
        if (list->used == 0 || index >= list->used) {                          \
            return false;                                                      \
        }                                                                      \
        for (size_t i = index + 1; i < list->used; ++i) {                      \
            T##Copy(&list->buffer[i - 1], &list->buffer[i], allocator);        \
        }                                                                      \
        T##Free(&list->buffer[list->used - 1UL]);                              \
        --list->used;                                                          \
        return true;                                                           \
    }                                                                          \
    static inline bool T##ListSwapRemove(T##List* list, const size_t index)    \
    {                                                                          \
        if (list->used == 0 || index >= list->used) {                          \
            return false;                                                      \
        }                                                                      \
        TFree(&list->buffer[index]);                                           \
        if (index != list->used - 1) {                                         \
            list->buffer[index] = list->buffer[list->used - 1];                \
            memset(&list->buffer[list->used - 1], 0, sizeof(T));               \
        }                                                                      \
        --list->used;                                                          \
        return true;                                                           \
    }                                                                          \
    static inline bool T##ListPop(T##List* list)                               \
    {                                                                          \
        return T##ListSwapRemove(list, list->used - 1);                        \
    }                                                                          \
    static inline bool T##ListRemoveValue(                                     \
      T##List* list, const T* value, const Allocator* allocator)               \
    {                                                                          \
        for (size_t i = 0; i < list->used; ++i) {                              \
            if (memcmp(&list->buffer[i], value, sizeof(T)) == 0) {             \
                return T##ListRemove(list, i, allocator);                      \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    typedef bool (*T##ListRemoveIfFunc)(const T*, const void*);                \
    static inline bool T##ListRemoveIf(T##List* list,                          \
                                       T##ListRemoveIfFunc f,                  \
                                       const void* arg,                        \
                                       const Allocator* allocator)             \
    {                                                                          \
        for (size_t i = 0; i < list->used; ++i) {                              \
            if (f(&list->buffer[i], arg)) {                                    \
                return T##ListRemove(list, i, allocator);                      \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline bool T##ListSwapRemoveValue(T##List* list, const T* value)   \
    {                                                                          \
        for (size_t i = 0; i < list->used; ++i) {                              \
            if (memcmp(&list->buffer[i], value, sizeof(T)) == 0) {             \
                return T##ListSwapRemove(list, i);                             \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline bool T##ListSwapRemoveIf(                                    \
      T##List* list, T##ListRemoveIfFunc f, const void* arg)                   \
    {                                                                          \
        for (size_t i = 0; i < list->used; ++i) {                              \
            if (f(&list->buffer[i], arg)) {                                    \
                return T##ListSwapRemove(list, i);                             \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline void T##ListFree(T##List* list)                              \
    {                                                                          \
        if (list->buffer == NULL) {                                            \
            return;                                                            \
        }                                                                      \
        for (size_t listIndex = 0; listIndex < list->used; ++listIndex) {      \
            TFree(&list->buffer[listIndex]);                                   \
        }                                                                      \
        list->allocator->free(list->buffer);                                   \
        memset(list, 0, sizeof(T##List));                                      \
    }                                                                          \
    static inline bool T##ListCopy(                                            \
      T##List* dest, const T##List* src, const Allocator* allocator)           \
    {                                                                          \
        T##ListFree(dest);                                                     \
        dest->allocator = allocator;                                           \
        if (T##ListIsEmpty(src)) {                                             \
            return true;                                                       \
        }                                                                      \
        dest->size = src->used;                                                \
        dest->buffer = allocator->allocate(dest->size * sizeof(T));            \
        for (size_t listIndex = 0; listIndex < src->used; ++listIndex) {       \
            if (!T##ListAppend(dest, &src->buffer[listIndex])) {               \
                return false;                                                  \
            }                                                                  \
        }                                                                      \
        return dest->used == src->used;                                        \
    }                                                                          \
    static inline bool T##ListFind(                                            \
      const T##List* list, const T* target, const T** result, size_t* i)       \
    {                                                                          \
        for (size_t index = 0; index < list->used; ++index) {                  \
            if (memcmp(&list->buffer[index], target, sizeof(T)) == 0) {        \
                if (result != NULL) {                                          \
                    *result = &list->buffer[index];                            \
                }                                                              \
                if (i != NULL) {                                               \
                    *i = index;                                                \
                }                                                              \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    typedef bool (*T##ListFindFunc)(const T*, const void*);                    \
    static inline bool T##ListFindIf(const T##List* list,                      \
                                     T##ListFindFunc f,                        \
                                     const void* arg,                          \
                                     const T** result,                         \
                                     size_t* i)                                \
    {                                                                          \
        for (size_t index = 0; index < list->used; ++index) {                  \
            if (f(&list->buffer[index], arg)) {                                \
                if (result != NULL) {                                          \
                    *result = &list->buffer[index];                            \
                }                                                              \
                if (i != NULL) {                                               \
                    *i = index;                                                \
                }                                                              \
                return true;                                                   \
            }                                                                  \
        }                                                                      \
        return false;                                                          \
    }                                                                          \
    static inline void T##ListShuffle(p##T##List list, pRandomState state)     \
    {                                                                          \
        size_t swaps = 0;                                                      \
        const size_t used = list->used;                                        \
        while (swaps < used) {                                                 \
            const size_t i = random64(state) % used;                           \
            const size_t j = random64(state) % used;                           \
            if (i == j) {                                                      \
                continue;                                                      \
            }                                                                  \
            const T temp = list->buffer[j];                                    \
            list->buffer[j] = list->buffer[i];                                 \
            list->buffer[i] = temp;                                            \
            ++swaps;                                                           \
        }                                                                      \
    }                                                                          \
    static inline void T##ListSort(p##T##List list, __compar_fn_t func)        \
    {                                                                          \
        qsort(list->buffer, list->used, sizeof(T), func);                      \
    }

#define DEFAULT_MAKE_LIST_FUNCTIONS(T) MAKE_LIST_FUNCTIONS(T, T##Copy, T##Free)

#define MAKE_DEFAULT_LIST(T)                                                   \
    MAKE_LIST(T);                                                              \
    DEFAULT_MAKE_LIST_FUNCTIONS(T);

#define LIST_TO_STRING(list, s, tostring, allocator)                           \
    TemLangString s = TemLangStringCreate("[ ", allocator);                    \
    for (size_t listIndex = 0; listIndex < list.used; ++listIndex) {           \
        TemLangString temporaryString =                                        \
          tostring(&list.buffer[listIndex], allocator);                        \
        TemLangStringAppend(&s, &temporaryString);                             \
        TemLangStringFree(&temporaryString);                                   \
        if (listIndex != list.used - 1) {                                      \
            TemLangStringAppendChars(&s, ", ");                                \
        }                                                                      \
    }                                                                          \
    TemLangStringAppendChars(&s, " ]");

#define MAKE_COPY_AND_FREE(T)                                                  \
    static inline void T##Free(T* t) { (void)t; }                              \
                                                                               \
    static inline bool T##Copy(T* a, const T* b, const Allocator* allocator)   \
    {                                                                          \
        *a = *b;                                                               \
        (void)allocator;                                                       \
        return true;                                                           \
    }

#define MAKE_FULL_LIST(T)                                                      \
    MAKE_LIST(T);                                                              \
    DEFAULT_MAKE_LIST_FUNCTIONS(T)