#pragma once

#include <memory.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "Misc.h"

typedef struct Allocator
{
    void* (*allocate)(size_t);
    void* (*reallocate)(void*, size_t);
    void (*free)(void*);
    size_t (*totalSize)();
    size_t (*used)();
} Allocator, *pAllocator;

static inline void*
zmalloc(const size_t size)
{
    void* p = calloc(1, size);
    memset(p, 0, size);
    return p;
}

static inline Allocator
makeDefaultAllocator()
{
    Allocator a = { 0 };
    a.allocate = zmalloc;
    a.reallocate = realloc;
    a.free = free;
    return a;
}

static inline void*
no_alloc(const size_t size)
{
    (void)size;
    fputs("Allocate called with no_alloc function", stderr);
    abort();
}

static inline void*
no_realloc(void* data, const size_t size)
{
    (void)data;
    (void)size;
    fputs("Allocate called with no_realloc function", stderr);
    abort();
}

static void
no_free(void* d)
{
    (void)d;
}

static inline Allocator
makeNoAllocator()
{
    return (Allocator){ .allocate = no_alloc,
                        .reallocate = no_realloc,
                        .free = no_free,
                        .totalSize = NULL,
                        .used = NULL };
}

#if __ANDROID__
#define ALLOCATOR_ALIGNMENT 16
#else
#define ALLOCATOR_ALIGNMENT (2 * sizeof(void*))
#endif

// Arena allocator:
// https://www.gingerbill.org/article/2019/02/08/memory-allocation-strategies-002/

typedef struct ArenaAllocator
{
    const char* name;
    uint8_t* buffer;
    size_t totalSize;
    size_t used;
    void* previousAllocation;
    size_t previousAllocationSize;
} ArenaAllocator, *pArenaAllocator;

static inline void
defaultArenaAllocatorOutOfMemory(const ArenaAllocator* a, const size_t size)
{
    fprintf(stderr,
            "Arena (%s) ran out of memory; Attempted to allocate %zu bytes but "
            "%zu/%zu bytes were already allocated\n",
            a->name == NULL ? "<Unnamed>" : a->name,
            size,
            a->used,
            a->totalSize);
}

#if HAS_ARENA_OUT_OF_MEMORY
extern void
arenaAllocatorOutOfMemory(const ArenaAllocator*, size_t);
#else
static inline void
arenaAllocatorOutOfMemory(const ArenaAllocator* a, const size_t size)
{
    defaultArenaAllocatorOutOfMemory(a, size);
    abort();
}
#endif

static inline size_t
arenaAllocatorSaveState(const ArenaAllocator* a)
{
    return a->used;
}

static inline void
arenaAllocatorRestore(ArenaAllocator* a, const size_t s)
{
    if (s < a->used) {
        a->used = s;
        a->previousAllocation = NULL;
        a->previousAllocationSize = 0;
    }
}

#define ARENA_SAVE_STATE(a, f)                                                 \
    {                                                                          \
        const size_t saveState = arenaAllocatorSaveState(a);                   \
        f;                                                                     \
        arenaAllocatorRestore(a, saveState);                                   \
    }

static inline void
arenaAllocatorReset(ArenaAllocator* a)
{
    a->used = 0;
    a->previousAllocation = NULL;
    a->previousAllocationSize = 0;
}

static inline bool
isPowerOfTwo(uintptr_t x)
{
    return (x & (x - 1)) == 0;
}

static inline uintptr_t
alignForward(uintptr_t ptr, const uintptr_t align)
{
    const uintptr_t modulo = ptr & (align - 1);
    if (modulo != 0) {
        ptr += align - modulo;
    }
    return ptr;
}

static inline void*
ArenaAllocatorAllocate(ArenaAllocator* arena, const size_t size)
{
    uintptr_t current = (uintptr_t)arena->buffer + (uintptr_t)arena->used;
    uintptr_t used = alignForward(current, ALLOCATOR_ALIGNMENT);
    used -= (uintptr_t)arena->buffer;

    if (used + size > arena->totalSize) {
        arenaAllocatorOutOfMemory(arena, size);
        return NULL;
    }

    void* ptr = &arena->buffer[used];
    arena->used = used + size;
    memset(ptr, 0, size);
    arena->previousAllocation = ptr;
    arena->previousAllocationSize = size;
    return ptr;
}

static inline void*
ArenaAllocatorRellocate(ArenaAllocator* arena,
                        void* oldPtr,
                        const size_t newSize)
{
    if (oldPtr != NULL && arena->previousAllocation == oldPtr) {
        if (arena->previousAllocationSize > newSize) {
            arena->used -= (arena->previousAllocationSize - newSize);
        } else {
            const size_t diff = newSize - arena->previousAllocationSize;
            if (arena->used + diff > arena->totalSize) {
                arenaAllocatorOutOfMemory(arena, diff);
                return NULL;
            }
            arena->used += diff;
            memset(&arena->buffer[arena->used], 0, diff);
        }
        arena->previousAllocationSize = newSize;
        return oldPtr;
    }

    void* newData = ArenaAllocatorAllocate(arena, newSize);
    if (newData == NULL) {
        arenaAllocatorOutOfMemory(arena, newSize);
        return NULL;
    }
    if (oldPtr != NULL) {
        memmove(newData, oldPtr, newSize);
    }
    return newData;
}

static inline void
ArenaAllocatorFree(void* oldPtr)
{
    (void)oldPtr;
}

#define MAKE_ARENA_ALLOCATOR(T)                                                \
    static ArenaAllocator T = { .name = #T };                                  \
    static inline void* T##_alloc(const size_t size)                           \
    {                                                                          \
        return ArenaAllocatorAllocate(&T, size);                               \
    }                                                                          \
    static inline void* T##_realloc(void* buffer, const size_t newSize)        \
    {                                                                          \
        return ArenaAllocatorRellocate(&T, buffer, newSize);                   \
    }                                                                          \
    static inline size_t T##_used() { return T.used; }                         \
    static inline size_t T##_totalSize() { return T.totalSize; }               \
    static inline Allocator make_##T##_allocator()                             \
    {                                                                          \
        Allocator a = { 0 };                                                   \
        a.allocate = T##_alloc;                                                \
        a.reallocate = T##_realloc;                                            \
        a.free = ArenaAllocatorFree;                                           \
        a.used = T##_used;                                                     \
        a.totalSize = T##_totalSize;                                           \
        return a;                                                              \
    }

// Free List allocator: https://github.com/mtrebi/memory-allocators

typedef enum PlacementPolicy
{
    PlacementPolicy_First,
    PlacementPolicy_Best
} PlacementPolicy,
  *pPlacementPolicy;

typedef struct FreeListNode
{
    size_t blockSize;
    struct FreeListNode* next;
} FreeListNode, *pFreeListNode;

static inline void
FreeListNodeInsert(pFreeListNode* head,
                   pFreeListNode previousNode,
                   pFreeListNode newNode)
{
    if (previousNode == NULL) {
        newNode->next = *head;
        *head = newNode;
    } else {
        newNode->next = previousNode->next;
        previousNode->next = newNode;
    }
}

static inline void
FreeListNodeRemove(pFreeListNode* head,
                   pFreeListNode previousNode,
                   pFreeListNode deleteNode)
{
    if (previousNode == NULL) {
        *head = deleteNode->next;
    } else {
        previousNode->next = deleteNode->next;
    }
}

typedef struct FreeListAllocator
{
    const char* name;
    void* startPtr;
    size_t used;
    size_t totalSize;
    FreeListNode* list;
    PlacementPolicy policy;
} FreeListAllocator, *pFreeListAllocator;

static inline bool
FreeListAllocatorHasPointer(const FreeListAllocator* allocator, void* ptr)
{
    if (ptr == NULL) {
        return false;
    }
    const size_t startAddress = (size_t)ptr;
    if (startAddress < (size_t)allocator->startPtr ||
        startAddress >= (size_t)allocator->startPtr + allocator->totalSize) {
        return false;
    }
    return true;
}

static inline void
FreeListAllocatorCleanup(pFreeListAllocator allocator)
{
    pFreeListNode node = allocator->list;
    while (node != NULL) {
        if (!FreeListAllocatorHasPointer(allocator, node->next)) {
            node->next = NULL;
            break;
        }
        node = node->next;
    }
}

static inline size_t
FreeListAllocatorTotalSize(const FreeListAllocator* allocator)
{
    size_t size = 0;
    const FreeListNode* node = allocator->list;
    while (node != NULL) {
        size += node->blockSize;
        node = node->next;
    }
    return size;
}

static inline bool
FreeListAllocatorVerify(const FreeListAllocator* allocator, const int line)
{
    const size_t size = FreeListAllocatorTotalSize(allocator);
    const bool valid = size == allocator->totalSize - allocator->used;
#if _DEBUG
    if (!valid) {
        fprintf(stderr,
                "Free list allocator failed verification on line %d. Expected "
                "%zu; Got %zu\n",
                line,
                allocator->totalSize - allocator->used,
                size);
    }
#else
    (void)line;
#endif
    return valid;
}

static inline size_t
FreeListAllocatorBlockCount(pFreeListAllocator allocator)
{
    size_t i = 0;
    pFreeListNode node = allocator->list;
    while (node != NULL) {
        ++i;
        node = node->next;
    }
    return i;
}

static inline void
FreeListAllocatorReset(pFreeListAllocator allocator)
{
    pFreeListNode first = (pFreeListNode)allocator->startPtr;
    first->blockSize = allocator->totalSize;
    first->next = NULL;
    allocator->used = 0;
    allocator->list = NULL;
    FreeListNodeInsert(&allocator->list, NULL, first);
}

static inline FreeListAllocator
FreeListAllocatorCreate(const char* name,
                        const size_t totalSize,
                        const PlacementPolicy policy)
{
    FreeListAllocator allocator = { .startPtr = malloc(totalSize),
                                    .totalSize = totalSize,
                                    .used = 0,
                                    .list = NULL,
                                    .policy = policy,
                                    .name = name };
    FreeListAllocatorReset(&allocator);
    return allocator;
}

static inline void
FreeListAllocatorDelete(pFreeListAllocator allocator)
{
    if (allocator->startPtr == NULL) {
        return;
    }
    free(allocator->startPtr);
    memset(allocator, 0, sizeof(FreeListAllocator));
}

static inline void
FreeListAllocatorCoalescence(pFreeListAllocator allocator,
                             pFreeListNode previousNode,
                             pFreeListNode freeNode)
{
    if (freeNode->next != NULL &&
        (size_t)freeNode + freeNode->blockSize == (size_t)freeNode->next) {
#if LOG_ALLOCATOR
        printf(
          "/*Merging freeNode (%p) and freeNode->next (%p). %zu blocks*/\n",
          freeNode,
          freeNode->next,
          FreeListAllocatorBlockCount(allocator));
#endif
        freeNode->blockSize += freeNode->next->blockSize;
        FreeListNodeRemove(&allocator->list, freeNode, freeNode->next);
#if LOG_ALLOCATOR
        printf("/*Merged: %zu blocks*/\n",
               FreeListAllocatorBlockCount(allocator));
#endif
    }
    if (previousNode != NULL &&
        (size_t)previousNode + previousNode->blockSize == (size_t)freeNode) {
#if LOG_ALLOCATOR
        printf("/*Merging previousNode (%p) and freeNode (%p). %zu blocks*/\n",
               previousNode,
               freeNode,
               FreeListAllocatorBlockCount(allocator));
#endif
        previousNode->blockSize += freeNode->blockSize;
        FreeListNodeRemove(&allocator->list, previousNode, freeNode);
#if LOG_ALLOCATOR
        printf("/*Merged: %zu blocks*/\n",
               FreeListAllocatorBlockCount(allocator));
#endif
    }
}

static inline void
FreeListAllocatorFree(pFreeListAllocator allocator, void* ptr)
{
    const size_t currentAddress = (size_t)ptr;
#if _DEBUG
    if (currentAddress < (size_t)allocator->startPtr ||
        currentAddress >= (size_t)allocator->startPtr + allocator->totalSize) {
        fprintf(
          stderr,
          "Attempted to free pointer (%p) that doesn't belong to the free "
          "list allocator\n",
          ptr);
        return;
    }
#endif
    const size_t headerAddress = currentAddress - sizeof(FreeListNode);

    pFreeListNode freeNode = (pFreeListNode)headerAddress;
#if LOG_ALLOCATOR
    if (freeNode->blockSize == 0) {
        fprintf(stderr,
                "Node (%p) with 0 block size was found during freeing\n",
                freeNode);
    }
    if ((freeNode->blockSize % ALLOCATOR_ALIGNMENT) != 0) {
        fprintf(stderr, "Invalid block size: %zu\n", freeNode->blockSize);
    }
#endif
    freeNode->next = NULL;

#if LOG_ALLOCATOR
    printf("/*Free'ing %zu bytes. %zu blocks*/\n",
           freeNode->blockSize,
           FreeListAllocatorBlockCount(allocator));
#endif

    pFreeListNode it = allocator->list;
    pFreeListNode itPrev = NULL;
    while (it != NULL) {
        if (freeNode < it) {
            FreeListNodeInsert(&allocator->list, itPrev, freeNode);
            break;
        }
        itPrev = it;
        it = it->next;
    }

    allocator->used -= freeNode->blockSize;

#if LOG_ALLOCATOR
    printf("/*Free'd %zu bytes. %zu bytes used. %zu blocks*/\n",
           freeNode->blockSize,
           allocator->used,
           FreeListAllocatorBlockCount(allocator));
#endif
    FreeListAllocatorCoalescence(allocator, itPrev, freeNode);
}

static inline void
defaultFreeListAllocatorOutOfMemory(const FreeListAllocator* a,
                                    const size_t size)
{
    fprintf(
      stderr,
      "Free list (%s) ran out of memory; Attempted to allocate %zu bytes but "
      "%zu/%zu bytes were already allocated\n",
      a->name == NULL ? "<Unnamed>" : a->name,
      size,
      a->used,
      a->totalSize);
}

#if HAS_FREE_LIST_OUT_OF_MEMORY
extern void
freeListAllocatorOutOfMemory(const FreeListAllocator* a, const size_t size);
#else
static inline void
freeListAllocatorOutOfMemory(const FreeListAllocator* a, const size_t size)
{
    defaultFreeListAllocatorOutOfMemory(a, size);
    abort();
}
#endif

static inline void
FreeListAllocatorFindFirst(pFreeListAllocator allocator,
                           const size_t size,
                           pFreeListNode* previousNode,
                           pFreeListNode* foundNode)
{
    pFreeListNode it = allocator->list;
    pFreeListNode itPrev = NULL;
    while (it != NULL) {
        if (it->blockSize >= size) {
            break;
        }
        itPrev = it;
        it = it->next;
    }
    *previousNode = itPrev;
    *foundNode = it;
}

static inline void
FreeListAllocatorFindBest(pFreeListAllocator allocator,
                          const size_t size,
                          pFreeListNode* previousNode,
                          pFreeListNode* foundNode)
{
    size_t smallestDiff = SIZE_MAX;
    pFreeListNode bestBlock = NULL;
    pFreeListNode bestPrevBlock = NULL;
    pFreeListNode it = allocator->list;
    pFreeListNode itPrev = NULL;
    while (it != NULL) {
        const size_t currentDiff = it->blockSize - size;
        if (it->blockSize >= size && currentDiff < smallestDiff) {
            bestBlock = it;
            bestPrevBlock = itPrev;
            smallestDiff = currentDiff;
        }
        itPrev = it;
        it = it->next;
    }
    *previousNode = bestPrevBlock;
    *foundNode = bestBlock;
}

static inline void
FreeListAllocatorFindBlock(pFreeListAllocator allocator,
                           const size_t size,
                           pFreeListNode* previousNode,
                           pFreeListNode* foundNode)
{
    switch (allocator->policy) {
        case PlacementPolicy_First:
            FreeListAllocatorFindFirst(
              allocator, size, previousNode, foundNode);
            break;
        case PlacementPolicy_Best:
            FreeListAllocatorFindBest(allocator, size, previousNode, foundNode);
            break;
        default:
            break;
    }
}

static inline void*
FreeListAllocatorAllocate(pFreeListAllocator allocator,
                          const size_t requestedSize)
{
    size_t size = MAX(requestedSize, ALLOCATOR_ALIGNMENT);
    size += ALLOCATOR_ALIGNMENT - (size % ALLOCATOR_ALIGNMENT);
    const size_t allocateSize = size + sizeof(FreeListNode);

#if LOG_ALLOCATOR
    printf("/*Allocating %zu bytes. (requested %zu bytes) %zu bytes used. %zu "
           "block(s)*/\n",
           allocateSize,
           requestedSize,
           allocator->used,
           FreeListAllocatorBlockCount(allocator));
#endif

    FreeListNode* affectedNode = NULL;
    FreeListNode* previousNode = NULL;

    FreeListAllocatorFindBlock(
      allocator, allocateSize, &previousNode, &affectedNode);
    if (affectedNode == NULL) {
        freeListAllocatorOutOfMemory(allocator, allocateSize);
        return NULL;
    }

    const size_t rest = affectedNode->blockSize - allocateSize;

    if (rest > 0) {
        pFreeListNode newFreeNode =
          (pFreeListNode)((size_t)affectedNode + allocateSize);
        newFreeNode->blockSize = rest;
        newFreeNode->next = NULL;
        FreeListNodeInsert(&allocator->list, affectedNode, newFreeNode);
#if LOG_ALLOCATOR
        printf("/*Insert new chunk: %zu. %zu block(s)*/\n",
               rest,
               FreeListAllocatorBlockCount(allocator));
#endif
    }

    FreeListNodeRemove(&allocator->list, previousNode, affectedNode);
    affectedNode->blockSize = allocateSize;
    affectedNode->next = NULL;

    allocator->used += allocateSize;

#if LOG_ALLOCATOR
    printf("/*Allocated %zu bytes. %zu bytes used. %zu block(s)*/\n",
           allocateSize,
           allocator->used,
           FreeListAllocatorBlockCount(allocator));
#endif

    const size_t dataAddress = (size_t)affectedNode + sizeof(FreeListNode);
    void* ptr = (void*)dataAddress;
    memset(ptr, 0, size);
    return ptr;
}

static inline void*
FreeListAllocatorReallocate(pFreeListAllocator allocator,
                            void* oldPtr,
                            size_t size)
{
    size = MAX(size, ALLOCATOR_ALIGNMENT);
    size += ALLOCATOR_ALIGNMENT - (size % ALLOCATOR_ALIGNMENT);

    if (oldPtr == NULL) {
        return FreeListAllocatorAllocate(allocator, size);
    }

    const size_t currentAddress = (size_t)oldPtr;
    const size_t nodeAddress = currentAddress - sizeof(FreeListNode);

    pFreeListNode node = (pFreeListNode)nodeAddress;

    const size_t oldSize = node->blockSize - sizeof(FreeListNode);
#if LOG_ALLOCATOR
    if (node->blockSize == 0) {
        fprintf(stderr,
                "Node (%p) with 0 block size was found during reallocation\n",
                node);
    }
#endif
    if (size <= oldSize) {
        return oldPtr;
    }

#if LOG_ALLOCATOR
    printf("/*Reallocating from %zu to %zu*/\n", oldSize, size);
    if ((oldSize % ALLOCATOR_ALIGNMENT) != 0) {
        fprintf(stderr, "Failed to get accurate old size\n");
    }
#endif

    {
        const size_t target = nodeAddress + node->blockSize;
        pFreeListNode it = allocator->list;
        pFreeListNode prev = NULL;
        while (it != NULL) {
            if ((size_t)it != target) {
                prev = it;
                it = it->next;
                continue;
            }
            // Extend current block size if possible
            const size_t combinedSize = node->blockSize + it->blockSize;
            const size_t newBlockSize = size + sizeof(FreeListNode);
            if (combinedSize == newBlockSize) {
                allocator->used -= node->blockSize;
                allocator->used += newBlockSize;
                node->blockSize = newBlockSize;
                FreeListNodeRemove(&allocator->list, prev, it);
                return oldPtr;
            } else if (newBlockSize < combinedSize) {
                allocator->used -= node->blockSize;
                allocator->used += newBlockSize;
                node->blockSize = newBlockSize;
                pFreeListNode newNode =
                  (pFreeListNode)((size_t)node + newBlockSize);
                newNode->blockSize = combinedSize - newBlockSize;
                newNode->next = NULL;
                FreeListNodeRemove(&allocator->list, prev, it);
                FreeListNodeInsert(&allocator->list, prev, newNode);
                return oldPtr;
            }
            break;
        }
    }

    void* newPtr = FreeListAllocatorAllocate(allocator, size);
    memcpy(newPtr, oldPtr, oldSize);
    FreeListAllocatorFree(allocator, oldPtr);
    return newPtr;
}

#define MAKE_FREE_LIST_ALLOCATOR(T)                                            \
    static FreeListAllocator T = { 0 };                                        \
    static inline void* T##_alloc(const size_t size)                           \
    {                                                                          \
        return FreeListAllocatorAllocate(&T, size);                            \
    }                                                                          \
    static inline void* T##_realloc(void* buffer, const size_t newSize)        \
    {                                                                          \
        return FreeListAllocatorReallocate(&T, buffer, newSize);               \
    }                                                                          \
    static inline void T##_free(void* buffer)                                  \
    {                                                                          \
        FreeListAllocatorFree(&T, buffer);                                     \
    }                                                                          \
    static inline size_t T##_used() { return T.used; }                         \
    static inline size_t T##_totalSize() { return T.totalSize; }               \
    static inline Allocator make_##T##_allocator()                             \
    {                                                                          \
        Allocator a = { 0 };                                                   \
        a.allocate = T##_alloc;                                                \
        a.reallocate = T##_realloc;                                            \
        a.free = T##_free;                                                     \
        a.used = T##_used;                                                     \
        a.totalSize = T##_totalSize;                                           \
        return a;                                                              \
    }