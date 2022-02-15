#include <DefaultExternalFunctions.h>
#include <HotReload.h>
#include <TemLangString.h>
#include <stdio.h>
#include <stdlib.h>

static TemLangStringList strings = { 0 };
static Allocator allocator = { 0 };

MAKE_ARENA_ALLOCATOR(temp);

void*
initialize()
{
    printf("New called initialize\n");
    static uint8_t buffer[MB(16)] = { 0 };
    temp.buffer = buffer;
    temp.length = sizeof(buffer);
    allocator = make_temp_allocator();
    // allocator = makeDefaultAllocator();
    strings.allocator = &allocator;
    strings.buffer = NULL;
    strings.size = 0;
    strings.used = 0;
    return &strings;
}

void
deinitialize(void* data)
{
    pTemLangStringList list = (void*)data;
    printf("New called deinitialize\n");
    TemLangStringListFree(list);
}

int
main(int argc, const char** argv)
{
    return runHotReload(argc, argv);
}