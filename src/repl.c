#define LOG_ALLOCATOR 0
#define HAS_ARENA_OUT_OF_MEMORY 0
#define HAS_FREE_LIST_OUT_OF_MEMORY 0

#if __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

#include "compiler.h"
#include "repl.h"

extern void
onReplOutput(const char*, const char*, bool);

extern void
onReplError(const char*);

extern void
onReplReset();

extern void
onCompileDone(const char*);

extern void
onExampleAcquired(const char*);

extern void
setTextColor(const char*);

State state = { 0 };
const char* currentColor = "black";

const char* EMSCRIPTEN_KEEPALIVE
getVersionString()
{
    static char buffer[32];
    snprintf(buffer,
             sizeof(buffer),
             "TemLang %d.%d.%d",
             TEMLANG_MAJOR_VERSION,
             TEMLANG_MINOR_VERSION,
             TEMLANG_REVISION);
    return buffer;
}

void EMSCRIPTEN_KEEPALIVE
reset()
{
    static Allocator allocator = { 0 };
    allocator = makeDefaultAllocator();
    StateFree(&state);
    state.atoms.allocator = &allocator;
    onReplReset();
}

void EMSCRIPTEN_KEEPALIVE
initialize()
{
    prepareCompiler();
    parseCompilerArgs(0, NULL);
    reset();
}

int
REPL_print(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[KB(128)];
    const int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    onReplOutput(buffer, currentColor, false);
    return result;
}

int
TemLangError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    char buffer[KB(128)];
    int result = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    result += snprintf(buffer + result, sizeof(buffer), "\n");
    onReplError(buffer);
    return result;
}

void
printValue(const Value* value, const Allocator* allocator)
{
    TemLangString s = ValueToString(value, allocator);
    onReplOutput(s.buffer, currentColor, true);
    TemLangStringFree(&s);
}

void EMSCRIPTEN_KEEPALIVE
replExecute(const char* content)
{
    // Newline character from textarea input
    printf("replExecute: %s", content);

    const Allocator* allocator = state.atoms.allocator;
    TokenList tokens =
      performLex(allocator, content, strlen(content), 1, "<User Input>");
    printf("Got %u tokens\n", tokens.used);
    InstructionList instructions = TokensToInstructions(&tokens, allocator);
    printf("Got %u instructions\n", instructions.used);

    currentColor = "darkcyan";
    REPL_print(">>> %s", content);

    Expression e = { 0 };
    Value value = { 0 };
    if (instructions.used == 0) {
        // Try to evaluate as an expression
        if (TokensToExpression(tokens.buffer, tokens.used, &e, allocator) &&
            EvaluateExpression(&e, &state, &value, allocator)) {
            if (value.type != ValueType_Null) {
                currentColor = "black";
                printValue(&value, allocator);
            }
            goto end;
        } else {
            TemLangError("Cannot parse into instructions or tokens: '%s'\n",
                         content);
        }
    }

    for (size_t i = 0; i < instructions.used; ++i) {
        ValueFree(&value);
        const Instruction* instruction = &instructions.buffer[i];
        if (StateProcessInstruction(&state, instruction, allocator, &value)) {
            if (value.type != ValueType_Null) {
                currentColor = "black";
                printValue(&value, allocator);
            }
        } else {
            InstructionError(instruction);
        }
    }

end:
    ExpressionFree(&e);
    ValueFree(&value);
    InstructionListFree(&instructions);
    TokenListFree(&tokens);
}

bool EMSCRIPTEN_KEEPALIVE
compile(const char* content)
{
    ProcessTokensArgs args = { .printInstructions = false,
                               .printTokens = false };
    TemLangString string = { .allocator = state.atoms.allocator };
    const bool result = compileContents(content,
                                        strlen(content),
                                        "<User Input>",
                                        args,
                                        &string,
                                        string.allocator);
    if (result) {
        onCompileDone(string.buffer);
    } else {
        TemLangError("Compilation failed...");
    }
    TemLangStringFree(&string);
    return result;
}

bool EMSCRIPTEN_KEEPALIVE
getExample(const char* filename)
{
    FILE* f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, "File '%s' not found: %s\n", filename, strerror(errno));
        return false;
    }

    TemLangString s = { .allocator = state.atoms.allocator };
    while (true) {
        const char c = (char)fgetc(f);
        if (feof(f)) {
            break;
        }
        TemLangStringAppendChar(&s, c);
    }
    fclose(f);

    onExampleAcquired(s.buffer);
    TemLangStringFree(&s);
    return true;
}