#pragma once

#include <List.h>
#include <ProcessTokensArgs.h>

#include <Includes.h>

#define MAX_FILES 32

MAKE_FREE_LIST_ALLOCATOR(globalFreeListAllocator);
MAKE_ARENA_ALLOCATOR(globalArenaAllocator);

extern bool replPrintIsComment;

typedef enum CompilerMode
{
    CompilerMode_Basic,
    CompilerMode_Dynamic,
    CompilerMode_Repl,
} CompilerMode,
  *pCompilerMode;

typedef enum AllocatorType
{
    AllocatorType_Default,
    AllocatorType_Arena,
    AllocatorType_FreeListFirst,
    AllocatorType_FreeListBest
} AllocatorType,
  *pAllocatorType;

typedef struct CompilerArgs
{
    size_t allocatorSize;
    AllocatorType allocatorType;
    CompilerMode mode;
    ProcessTokensArgs processTokenArgs;
    CString preInitFile;
    CString initFile;
    CString structName;
    CString outputFile;
    CString structOutput;
    CString files[MAX_FILES];
    size_t fileCount;
    bool handleOutOfMemory;
    bool printCompilerArgs;
    bool useTempAllocator;
    bool useCGLM;
} CompilerArgs, *pCompilerArgs;

static inline CompilerArgs
parseCompilerArgs(int argc, char** argv)
{
    CompilerArgs args = { .allocatorSize = MB(32),
                          .allocatorType = AllocatorType_Default,
                          .mode = CompilerMode_Basic,
                          .processTokenArgs = { .printInstructions = false,
                                                .printTokens = false },
                          .preInitFile = NULL,
                          .initFile = "init.tem",
                          .structName = NULL,
                          .outputFile = "output.tem",
                          .structOutput = NULL,
                          .files = { 0 },
                          .fileCount = 0,
                          .handleOutOfMemory = false,
                          .printCompilerArgs = false,
                          .useTempAllocator = false,
                          .useCGLM = false };
    int i = 1;
    while (i < argc) {
        const char* c = argv[i];
        const size_t len = strlen(c);
        STR_EQUALS(c, "--print-tokens", len, {
            args.processTokenArgs.printTokens = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "-PT", len, {
            args.processTokenArgs.printTokens = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "--print-instructions", len, {
            args.processTokenArgs.printInstructions = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "-PI", len, {
            args.processTokenArgs.printInstructions = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "--print-arguments", len, {
            args.printCompilerArgs = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "-PA", len, {
            args.printCompilerArgs = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "--use-cglm", len, {
            args.useCGLM = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "-G", len, {
            args.useCGLM = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "--handle-out-of-memory", len, {
            args.handleOutOfMemory = true;
            i += 1;
            continue;
        });
        STR_EQUALS(c, "-H", len, {
            args.handleOutOfMemory = true;
            i += 1;
            continue;
        });
        if (i + 1 >= argc) {
            break;
        }
        STR_EQUALS(c, "--allocator", len, {
            char* end = NULL;
            args.allocatorSize = strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-A", len, {
            char* end = NULL;
            args.allocatorSize = strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--allocator-type", len, {
            char* end = NULL;
            args.allocatorType = (AllocatorType)strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-AT", len, {
            char* end = NULL;
            args.allocatorType = (AllocatorType)strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--mode", len, {
            char* end = NULL;
            args.mode = (CompilerMode)strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-M", len, {
            char* end = NULL;
            args.mode = (CompilerMode)strtoull(argv[i + 1], &end, 10);
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--pre-init", len, {
            args.preInitFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-P", len, {
            args.preInitFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--init", len, {
            args.initFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-I", len, {
            args.initFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--output", len, {
            args.outputFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-O", len, {
            args.outputFile = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--struct-name", len, {
            args.structName = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-S", len, {
            args.structName = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "--struct-output", len, {
            args.structName = argv[i + 1];
            i += 2;
            continue;
        });
        STR_EQUALS(c, "-SO", len, {
            args.structOutput = argv[i + 1];
            i += 2;
            continue;
        });
        break;
    }
    while (i < argc && args.fileCount < MAX_FILES) {
        args.files[args.fileCount] = argv[i];
        ++args.fileCount;
        ++i;
    }
    return args;
}