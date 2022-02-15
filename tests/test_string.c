#include <DefaultExternalFunctions.h>
#include <Serialize.h>
#include <TemLangString.h>
#include <stdio.h>
#include <stdlib.h>

static bool
testString(const Allocator*);

static bool
testIntList(const Allocator*);

MAKE_ARENA_ALLOCATOR(temp);

int
main(int argc, char** argv)
{
    temp.buffer = malloc(MB(1));
    temp.length = MB(1);
    temp.offset = 0;
    temp.previousAllocation = NULL;

    // Allocator allocator = makeDefaultAllocator();
    Allocator allocator = make_temp_allocator();
    printf("Testing string...\n");
    printf("Test string passed: %s\n", testString(&allocator) ? "Yes" : "No");

    const TemLangString s =
      TemLangStringCreate("This is a message", &allocator);
    const int8_t i = 53;
    printf("TemLangString and num: %s and %d\n", s.buffer, i);

    Bytes bytes = { .allocator = &allocator };
    StringSerialize(&s, &bytes, true);

    int8_tSerialize(&i, &bytes, true);

    TemLangString s2 = { 0 };
    const size_t offset = StringDeserialize(&s2, &bytes, 0, true);
    int8_t k = 0;
    int8_tDeserialize(&k, &bytes, offset, true);
    printf("Deserialized: %s and %d\n", s2.buffer, k);

    free(temp.buffer);
    return 0;
}

bool
testString(const Allocator* allocator)
{
    bool result = true;

    TemLangString hello = TemLangStringCreate("Hello", allocator);
    TemLangString world = TemLangStringCreate("World", allocator);
    TemLangString newS = TemLangStringCreate("New", allocator);
    printf("Test Create: %s, %s!\n", hello.buffer, world.buffer);

    TemLangString s = { 0 };
    s.allocator = allocator;
    TemLangStringAppend(&s, &hello);
    TemLangStringAppendChar(&s, ", ");
    TemLangStringAppend(&s, &world);
    TemLangStringAppendChar(&s, '!');
    printf("Test Append: %s\n", s.buffer);
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }

    if (!TemLangStringRemove(&s, 4)) {
        result = false;
        goto cleanup;
    }
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }
    printf("Test Remove: %s\n", s.buffer);
    if (TemLangStringPop(&s) != '!') {
        result = false;
        goto cleanup;
    }
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }
    printf("Test Pop: %s\n", s.buffer);

    TemLangStringInsertChar(&s, 'a', 1);
    printf("Test Insert Char: %s\n", s.buffer);
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }

    TemLangStringInsert(&s, &newS, 6);
    printf("Test insert: %s\n", s.buffer);
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }

    TemLangStringRemoveChunk(&s, 0, 6);
    printf("Test remove chunk: %s\n", s.buffer);
    if (strlen(s.buffer) != s.used) {
        result = false;
        goto cleanup;
    }

cleanup:
    TemLangStringFree(&s);
    TemLangStringFree(&newS);
    TemLangStringFree(&hello);
    TemLangStringFree(&world);
    return result;
}

bool
testIntList(const Allocator* a)
{
    return true;
}