

#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <Includes.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

bool
update(pTemLangStringList list)
{
    do {
        struct pollfd pfds;
        pfds.fd = STDIN_FILENO;
        pfds.events = POLLIN;
        if (poll(&pfds, 1, 1000) == 0 || (pfds.revents & POLLIN) == 0) {
            break;
        }

        char buffer[32] = { 0 };
        const ssize_t r = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (r <= 0) {
            break;
        }
        switch (buffer[0]) {
            case 'q':
                return false;
            case 'p':
                if (TemLangStringListIsEmpty(list)) {
                    printf("Empty list...\n");
                } else {
                    for (size_t i = 0; i < list->used; ++i) {
                        printf("#%zu: %s\n", i, list->buffer[i].buffer);
                    }
                }
                continue;
            case 'r':
                TemLangStringListRemove(list, 1, list->allocator);
                continue;
            default:
                break;
        }
        buffer[r - 1] = '\0';
        TemLangString s = { .allocator = list->allocator,
                            .buffer = NULL,
                            .size = 0UL,
                            .used = 0UL };
        TemLangStringAppendChars(&s, buffer);
        TemLangStringListAppend(list, &s);
        TemLangStringFree(&s);
        printf("Added: %s\n", buffer);
    } while (true);
    return true;
}
