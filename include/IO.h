#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

typedef enum MapFileType
{
    MapFileType_Read,
    MapFileType_Write
} MapFileType,
  *pMapFileType;

static inline bool
mapFile(const char* filename, int* fd, char** ptr, size_t* size, MapFileType t)
{
    switch (t) {
        case MapFileType_Write:
            *fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            break;
        default:
            *fd = open(filename, O_RDONLY);
            break;
    }
    if (*fd < 0) {
        return false;
    }

    struct stat buf;
    if (fstat(*fd, &buf)) {
        return false;
    }

    switch (t) {
        case MapFileType_Write:
            *ptr = mmap(NULL, buf.st_size, PROT_WRITE, MAP_SHARED, *fd, 0);
            break;
        default:
            *ptr = mmap(NULL, buf.st_size, PROT_READ, MAP_SHARED, *fd, 0);
            break;
    }
    *size = buf.st_size;
    return true;
}

static inline void
unmapFile(int fd, char* ptr, const size_t size)
{
    if (fd >= 0) {
        close(fd);
    }
    if (ptr != NULL) {
        munmap(ptr, size);
    }
}