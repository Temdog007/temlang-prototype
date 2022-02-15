#pragma once

#include "compilerArgs.h"

#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>

#include <Allocator.h>
#include <Expression.h>
#include <IO.h>
#include <Instruction.h>
#include <Lexer.h>
#include <State.h>
#include <StateCommand.h>

static inline int
runRepl(const CompilerArgs args, const Allocator* allocator)
{
    State state = { 0 };
    state.atoms.allocator = allocator;

    for (size_t i = 0; i < args.fileCount; ++i) {
        printf("Opening file '%s'...\n", args.files[i]);
        int fd = -1;
        char* ptr = NULL;
        size_t size = 0UL;
        if (mapFile(args.files[i], &fd, &ptr, &size, MapFileType_Read)) {
            TokenList list = performLex(allocator, ptr, size, 1, args.files[i]);
            Value v = StateProcessTokens(
              &state, &list, allocator, args.processTokenArgs);
            ValueFree(&v);
            TokenListFree(&list);
            printf(STRING(GREEN_TEXT, "Loaded file\n"));
        } else {
            fprintf(stderr,
                    STRING(RED_TEXT, "Failed to open file '%s': %s\n"),
                    args.files[i],
                    strerror(errno));
        }

        unmapFile(fd, ptr, size);
    }

    struct pollfd pfds;
    pfds.fd = STDIN_FILENO;
    pfds.events = POLLIN;
    char buffer[1024] = { 0 };
    while (true) {
        if (poll(&pfds, 1, 1000) == 0 || (pfds.revents & POLLIN) == 0) {
            continue;
        }

        // Last character is always a '\n'. So subtract one
        const ssize_t r = read(STDIN_FILENO, buffer, sizeof(buffer)) - 1L;
        if (r <= 0) {
            continue;
        }

        switch (r) {
            case 1:
                switch (buffer[0]) {
                    case 'q':
                        goto cleanup;
                    default:
                        break;
                }
                break;
            case 4:
                if (memcmp("quit", buffer, 4) == 0) {
                    goto cleanup;
                }
                if (memcmp("exit", buffer, 4) == 0) {
                    goto cleanup;
                }
                break;
            default:
                break;
        }
        buffer[r] = '\0';
        if (buffer[0] == '$') {
            switch (StateCommandFromCaseInsensitiveString(&buffer[1], r - 1)) {
                case StateCommand_Print: {
                    TemLangString s = StateToString(&state, allocator);
                    printf(STRING(CYAN_TEXT, "%s\n"), s.buffer);
                    TemLangStringFree(&s);
                } break;
                default:
                    fprintf(stderr,
                            STRING(RED_TEXT, "'%s' is not a valid command\n"),
                            &buffer[1]);
                    break;
            }
            continue;
        }

        TokenList list = performLex(allocator, buffer, r, 1, "<User Input>");
        if (list.buffer[0].type == TokenType_InstructionStarter) {
            Value value = StateProcessTokens(
              &state, &list, allocator, args.processTokenArgs);
            if (value.type != ValueType_Null) {
                TemLangString s = ValueToString(&value, allocator);
                printf(STRING(CYAN_TEXT, "%s\n"), s.buffer);
                TemLangStringFree(&s);
                ValueFree(&value);
            }
        } else {
            Expression e = { 0 };
            if (TokensToExpression(list.buffer, list.used, &e, allocator)) {
                Value value = { 0 };
                if (EvaluateExpression(&e, &state, &value, allocator)) {
                    TemLangString s = ValueToString(&value, allocator);
                    printf(STRING(CYAN_TEXT, "%s\n"), s.buffer);
                    TemLangStringFree(&s);
                    ValueFree(&value);
                }
                ValueFree(&value);
            } else {
                TemLangError("Failed to parse '%s' to an expression", buffer);
            }
            ExpressionFree(&e);
        }
        TokenListFree(&list);
    }

cleanup:
    StateFree(&state);
    return 0;
}