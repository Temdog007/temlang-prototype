#pragma once
#include "compilerArgs.h"

#include <Compiler.h>
#include <Lexer.h>

#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

static bool
compileFile(const char*,
            const ProcessTokensArgs,
            pTemLangString,
            const Allocator*);

static bool
compileContents(const char*,
                const size_t,
                const char*,
                const ProcessTokensArgs,
                pTemLangString,
                const Allocator*);

static inline int
runCompiler(CompilerArgs args, const Allocator* allocator)
{
    int returnValue = 0;

    // Read from standard input first
    size_t compiled = 0;
    TemLangString s = { .allocator = allocator };
    {
        struct pollfd pfds;
        pfds.fd = STDIN_FILENO;
        pfds.events = POLLIN;
        if (poll(&pfds, 1, 1) > 0 && (pfds.revents & POLLIN) != 0) {
            char buffer[8196] = { 0 };
            ssize_t r = read(STDIN_FILENO, buffer, sizeof(buffer));
            while (r > 0) {
                buffer[r] = '\0';
                TemLangStringAppendChars(&s, buffer);
                r = read(STDIN_FILENO, buffer, sizeof(buffer));
            }
            s.used = 0;
            if (compileContents(s.buffer,
                                s.used,
                                "<Standard Input>",
                                args.processTokenArgs,
                                &s,
                                allocator)) {
                printf("%s\n", s.buffer);
            } else {
                --returnValue;
            }
            ++compiled;
        }
    }

    for (size_t i = 0; i < args.fileCount; ++i) {
        s.used = 0;
        if (compileFile(args.files[i], args.processTokenArgs, &s, allocator)) {
            printf("%s\n", s.buffer);
        } else {
            TemLangError("Failed to compile file: %s", args.files[i]);
            --returnValue;
        }
        ++compiled;
    }
    TemLangStringFree(&s);

    if (compiled == 0) {
        TemLangError("No input to compile");
        returnValue = EXIT_FAILURE;
    }

    return returnValue;
}

bool
compileContents(const char* contents,
                const size_t contentSize,
                const char* source,
                const ProcessTokensArgs args,
                pTemLangString output,
                const Allocator* allocator)
{
    TokenList tokens = performLex(allocator, contents, contentSize, 1, source);
    if (args.printTokens) {
        for (size_t i = 0; i < tokens.used; ++i) {
            TemLangString s = TokenToString(&tokens.buffer[i], allocator);
            REPL_print("%s\n", s.buffer);
            TemLangStringFree(&s);
        }
    }
    InstructionList instructions = TokensToInstructions(&tokens, allocator);

    // Reorder instructions so that definitions are first and dependencies are
    // met
    {
        bool needsCheck;
        do {
            needsCheck = false;
            for (size_t i = 0; !needsCheck && i < instructions.used; ++i) {
                Instruction* a = &instructions.buffer[i];
                if (a->type != InstructionType_DefineStruct) {
                    continue;
                }
                for (size_t j = i + 1; !needsCheck && j < instructions.used;
                     ++j) {
                    Instruction* b = &instructions.buffer[j];
                    const TemLangString* typeName = NULL;
                    switch (b->type) {
                        case InstructionType_DefineStruct:
                            typeName = &b->defineStruct.name;
                            break;
                        case InstructionType_DefineEnum:
                            typeName = &b->defineEnum.name;
                            break;
                        case InstructionType_DefineRange:
                            typeName = &b->defineRange.name;
                            break;
                        default:
                            continue;
                    }
                    if (StructMemberListFindIf(
                          &a->defineStruct.definition.members,
                          (StructMemberListFindFunc)
                            StructMemberTypeNameEqualsToString,
                          typeName,
                          NULL,
                          NULL)) {
                        const Instruction temp = *a;
                        *a = *b;
                        *b = temp;
                        needsCheck = true;
                        break;
                    }
                }
            }
        } while (needsCheck);
    }

    bool success = false;
    {

        VariableTarget t = { 0 };
        State state = { 0 };
        state.atoms.allocator = allocator;
        success =
          CompileInstructions(&instructions, allocator, t, &state, output);
        COMPILE_STATE_CLEANUP(state, (*output));
    }
    TokenListFree(&tokens);
    InstructionListFree(&instructions);
    return success;
}

bool
compileFile(const char* filename,
            const ProcessTokensArgs args,
            pTemLangString output,
            const Allocator* allocator)
{
    bool result = false;
    char* ptr = NULL;
    size_t size = 0UL;
    int fd = -1;

    if (mapFile(filename, &fd, &ptr, &size, MapFileType_Read)) {
        result = compileContents(ptr, size, filename, args, output, allocator);
    } else {
        result = false;
        TemLangError("Failed to open file '%s': %s", filename, strerror(errno));
    }

    unmapFile(fd, ptr, size);
    return result;
}
