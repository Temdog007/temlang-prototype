#define LOG_ALLOCATOR 0
#define HAS_ARENA_OUT_OF_MEMORY 1
#define HAS_FREE_LIST_OUT_OF_MEMORY 1

#include "compiler.h"
#include "repl.h"

int
runDynamic(CompilerArgs, const Allocator* allocator);

bool replPrintIsComment = false;

int
main(int argc, char** argv)
{
    printf("// TemLang %d.%d.%d\n",
           TEMLANG_MAJOR_VERSION,
           TEMLANG_MINOR_VERSION,
           TEMLANG_REVISION);

    prepareCompiler();

    const CompilerArgs args = parseCompilerArgs(argc, argv);
    useCGLM = args.useCGLM;
    if (args.printCompilerArgs) {
        printf("/*Allocator size: %zu\nCompiler mode: %u\nPrint tokens: "
               "%s\nPrint instructions: %s\nPre-init file: %s\nInit file: "
               "%s\n",
               args.allocatorSize,
               (uint32_t)args.mode,
               args.processTokenArgs.printTokens ? "true" : "false",
               args.processTokenArgs.printInstructions ? "true" : "false",
               args.preInitFile,
               args.initFile);
        for (size_t i = 0; i < args.fileCount; ++i) {
            printf("#%zu: %s\n", i + 1, args.files[i]);
        }
        puts("*/");
    }

    Allocator allocator = { 0 };
    switch (args.allocatorType) {
        case AllocatorType_FreeListBest:
            globalFreeListAllocator = FreeListAllocatorCreate(
              "global", args.allocatorSize, PlacementPolicy_Best);
            allocator = make_globalFreeListAllocator_allocator();
            break;
        case AllocatorType_FreeListFirst:
            globalFreeListAllocator = FreeListAllocatorCreate(
              "global", args.allocatorSize, PlacementPolicy_First);
            allocator = make_globalFreeListAllocator_allocator();
            break;
        case AllocatorType_Arena:
            globalArenaAllocator.buffer = malloc(args.allocatorSize);
            globalArenaAllocator.totalSize = args.allocatorSize;
            allocator = make_globalArenaAllocator_allocator();
            break;
        default:
            allocator = makeDefaultAllocator();
            break;
    }

    int result = EXIT_FAILURE;
    switch (args.mode) {
        case CompilerMode_Basic:
            replPrintIsComment = true;
            result = runCompiler(args, &allocator);
            break;
        case CompilerMode_Dynamic:
            replPrintIsComment = true;
            result = runDynamic(args, &allocator);
            break;
        case CompilerMode_Repl:
            replPrintIsComment = false;
            result = runRepl(args, &allocator);
            break;
        default:
            TemLangError("Unknown compiler mode: %u\n", args.mode);
            break;
    }
    switch (args.allocatorType) {
        case AllocatorType_FreeListFirst:
        case AllocatorType_FreeListBest:
            FreeListAllocatorDelete(&globalFreeListAllocator);
            break;
        case AllocatorType_Arena:
            free(globalArenaAllocator.buffer);
            break;
        default:
            break;
    }
    return result;
}

int
runDynamic(const CompilerArgs args, const Allocator* allocator)
{
    const char* structName =
      args.structName == NULL ? "ProgramState" : args.structName;

    int result = EXIT_FAILURE;
    State state = { 0 };
    state.atoms.allocator = allocator;
    TemLangString initOutput = TemLangStringCreate("#pragma once\n", allocator);
    TemLangString mainOutput = TemLangStringCreate("", allocator);
    if (args.handleOutOfMemory) {
        TemLangStringAppendChars(&mainOutput,
                                 "#define HAS_ARENA_OUT_OF_MEMORY\n");
    }
    const VariableTarget target = { .type = VariableTarget_None };

    {
        Instruction newI = { .type = InstructionType_InlineCHeaders };
        newI.source.lineNumber = 1;
        newI.source.source =
          TemLangStringCreate("<TemLang Compiler>", allocator);

        if (!CompileInstruction(
              &state, &newI, allocator, target, &mainOutput) ||
            !CompileInstruction(
              &state, &newI, allocator, target, &initOutput)) {
            TemLangError("Compiler error! Failed to include headers");
            goto cleanup;
        }
        TemLangStringAppendFormat(
          mainOutput,
          "#if HOT_RELOAD\n\n#include<HotReload.h>\n#endif\n\n#include "
          "<%s>\nstatic "
          "bool initiated = true;",
          args.outputFile);
    }

    if (args.preInitFile != NULL) {
        int fd = -1;
        char* ptr = NULL;
        size_t size = 0UL;

        bool success;
        if (mapFile(args.preInitFile, &fd, &ptr, &size, MapFileType_Read)) {
            const TokenList tokens =
              performLex(allocator, ptr, size, 1UL, args.preInitFile);
            const InstructionList instructions =
              TokensToInstructions(&tokens, allocator);
            success = CompileInstructions(
              &instructions, allocator, target, &state, &initOutput);
        } else {
            TemLangError("Failed to open file %s: %s\n",
                         args.preInitFile,
                         strerror(errno));
            success = false;
        }
        unmapFile(fd, ptr, size);
        if (!success) {
            TemLangError("Failed to compile pre-init file");
            goto cleanup;
        }
    }

    const Expression functionName = {
        .type = ExpressionType_UnaryValue,
        .value = { .type = ValueType_String,
                   .string = TemLangStringCreate("DoInitialize", allocator) }
    };
    const Expression functionArgs = { .type = ExpressionType_UnaryValue,
                                      .value = { .type = ValueType_String,
                                                 .string = TemLangStringCreate(
                                                   "()", allocator) } };
    InstructionList instructions = { 0 };
    {
        int fd = -1;
        char* ptr = NULL;
        size_t size = 0UL;

        if (!mapFile(args.initFile, &fd, &ptr, &size, MapFileType_Read)) {
            TemLangError(
              "Failed to open file %s: %s", args.initFile, strerror(errno));
            goto cleanup;
        }
        TokenList tokens = performLex(allocator, ptr, size, 1UL, args.initFile);
        instructions = TokensToInstructions(&tokens, allocator);
        unmapFile(fd, ptr, size);
    }

    {
        const size_t s = strlen(structName);
        const TemLangString structNameStr = {
            .allocator = NULL, .buffer = (char*)structName, .size = s, .used = s
        };
        StructDefinition sd = { 0 };
        if (!CompileCFunctionReturnValue(&state,
                                         allocator,
                                         &functionName,
                                         &functionArgs,
                                         &structNameStr,
                                         &instructions,
                                         &sd,
                                         &initOutput,
                                         &mainOutput,
                                         false)) {
            TemLangError("Failed to generate initialize function");
            goto cleanup;
        }
        if (args.structOutput != NULL) {
            int fd = -1;
            char* ptr = NULL;
            size_t size = 0UL;
            if (mapFile(
                  args.structOutput, &fd, &ptr, &size, MapFileType_Write)) {
                TemLangString s =
                  StructDefinitionToTemLang(&structNameStr, &sd, allocator);
                write(fd, s.buffer, s.used);
            }
            unmapFile(fd, ptr, size);
        }
    }

    TemLangStringAppendFormat(
      mainOutput,
      "void* initialize(){static %s value;value = "
      "DoInitialize(); return "
      "initiated ? &value : NULL;}\n#if HOT_RELOAD\nint "
      "main(int argc, const char** argv){ "
      "return runHotReload(argc, argv);}\n#endif\n",
      structName);
    TemLangStringAppendFormat(mainOutput,
                              "void deinitialize(void* "
                              "v){%s* arg =(%s*)v;",
                              structName,
                              structName);
    TemLangStringAppendFormat(mainOutput, "%sFree(arg);}", structName);
    {
        FILE* file = fopen(args.outputFile, "w");
        if (file == NULL) {
            TemLangError(
              "Failed to open file %s: %s", args.outputFile, strerror(errno));
            goto cleanup;
        }
        fwrite(initOutput.buffer, 1, initOutput.used, file);
        fclose(file);
    }

    puts(mainOutput.buffer);
    result = EXIT_SUCCESS;
cleanup:
    return result;
}

void
arenaAllocatorOutOfMemory(const ArenaAllocator* a, const size_t size)
{
    fprintf(stderr, RED_TEXT);
    defaultArenaAllocatorOutOfMemory(a, size);
    fprintf(stderr, RESET_TEXT);
    abort();
}

void
freeListAllocatorOutOfMemory(const FreeListAllocator* a, const size_t size)
{
    fprintf(stderr, RED_TEXT);
    defaultFreeListAllocatorOutOfMemory(a, size);
    fprintf(stderr, RESET_TEXT);
    abort();
}

int
REPL_print(const char* format, ...)
{
    int result = 0;
    if (replPrintIsComment) {
        result += fputs("// ", stdout);
        va_list args;
        va_start(args, format);
        result += vfprintf(stdout, format, args);
        va_end(args);
        result += fputs("\n", stdout);
    } else {
        result += fprintf(stdout, YELLOW_TEXT);
        va_list args;
        va_start(args, format);
        result += vfprintf(stdout, format, args);
        va_end(args);
        result += fprintf(stdout, "\n" RESET_TEXT);
    }
    return result;
}

int
TemLangError(const char* format, ...)
{
    const int a = fprintf(stderr, RED_TEXT);
    va_list args;
    va_start(args, format);
    const int b = vfprintf(stderr, format, args);
    va_end(args);
    const int c = fprintf(stderr, "\n" RESET_TEXT);
    return a + b + c;
}
