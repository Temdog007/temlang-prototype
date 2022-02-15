#pragma once

#include <Includes.h>

#include <dlfcn.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

extern void*
initialize();

extern void
deinitialize(void*);

typedef bool (*UpdateFunction)(void*);

typedef struct HotReloadArgs
{
    struct stat updateStat;
    CString updateFile;
    CString updateFunctionName;
    void* updateHandle;
    UpdateFunction updateFunction;
    size_t copies;
} HotReloadArgs, *pHotReloadArgs;

static inline HotReloadArgs
parseForHotReload(int argc, const char** argv);

static inline void
HotReloadArgsFree(pHotReloadArgs args);

static inline bool
copyFile(const char* src, const char* dest);

static inline bool
fileChanged(const char*, struct stat);

static inline bool loadUpdateFunction(pHotReloadArgs);
static inline void unloadUpdateFunction(pHotReloadArgs);

static inline int
runHotReload(int argc, const char** argv)
{
    int result = EXIT_SUCCESS;
    HotReloadArgs args = parseForHotReload(argc, argv);

    if (!loadUpdateFunction(&args)) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    void* updateArgs = initialize();
    if (updateArgs == NULL) {
        result = EXIT_FAILURE;
        goto cleanup;
    }

    while (args.updateFunction(updateArgs)) {
        if (fileChanged(args.updateFile, args.updateStat)) {
            loadUpdateFunction(&args);
        }
    }

    deinitialize(updateArgs);

cleanup:
    HotReloadArgsFree(&args);
    return result;
}

static inline void
unloadUpdateFunction(pHotReloadArgs args)
{
    if (args->updateHandle == NULL) {
        return;
    }
    dlclose(args->updateHandle);
    args->updateHandle = NULL;
    args->updateFunction = NULL;
}

static inline bool
loadUpdateFunction(pHotReloadArgs args)
{
    void* updateHandle = NULL;

    char filename[256] = { 0 };
    snprintf(
      filename, sizeof(filename), "%sCopy%zu", args->updateFile, args->copies);
    ++args->copies;
    {
        static size_t failures = 0UL;
        if (!copyFile(args->updateFile, filename)) {
            ++failures;
            fprintf(stderr,
                    "Failed to copy new update file. Will try again later (%zu "
                    "failures)\n",
                    failures);
            return true;
        }
        failures = 0;
    }
    {
        struct stat sb = { 0 };
        if (stat(args->updateFile, &sb)) {
            perror("stat");
            goto cleanup;
        }
        args->updateStat = sb;
    }

    updateHandle = dlopen(filename, RTLD_LAZY);
    if (updateHandle == NULL) {
        fprintf(
          stderr, "Failed to open update file %s; %s\n", filename, dlerror());
        goto cleanup;
    }

    UpdateFunction updateFunction =
      (UpdateFunction)dlsym(updateHandle, args->updateFunctionName);
    if (updateFunction == NULL) {
        fprintf(stderr,
                "Failed to find update function %s; %s\n",
                args->updateFunctionName,
                dlerror());
        goto cleanup;
    }

    unloadUpdateFunction(args);
    args->updateHandle = updateHandle;
    args->updateFunction = updateFunction;
    return true;

cleanup:
    if (updateHandle != NULL) {
        dlclose(updateHandle);
    }
    return false;
}

static inline HotReloadArgs
parseForHotReload(int argc, const char** argv)
{
    HotReloadArgs args = { .updateFile = "",
                           .updateFunctionName = "update",
                           .updateHandle = NULL,
                           .copies = 0UL };
    int i = 1;
    while (i < argc) {
        const char* c = argv[i];
        const size_t len = strlen(c);
        STR_EQUALS(c, "-U", len, {
            args.updateFile = argv[i + 1];
            i += 2;
            continue;
        });
        fprintf(stderr, "Unknown argument: %s\n", argv[i]);
        ++i;
    }
    return args;
}

static inline void
HotReloadArgsFree(pHotReloadArgs args)
{
    unloadUpdateFunction(args);
}

static inline bool
copyFile(const char* src, const char* dest)
{
    bool result = true;
    int input = -1;
    int output = -1;

    input = open(src, O_RDONLY);
    if (input < 0) {
        fprintf(stderr,
                "Failed to open '%s' for copying: %s\n",
                src,
                strerror(errno));
        result = false;
        goto cleanup;
    }
    output = open(dest, O_CREAT | O_WRONLY, 0777);
    if (output < 0) {
        fprintf(stderr,
                "Failed to open '%s' for copying: %s\n",
                dest,
                strerror(errno));
        result = false;
        goto cleanup;
    }

    char buffer[8192] = { 0 };
    ssize_t totalBytesCopied = 0L;
    while (true) {
        const ssize_t bytesRead = read(input, buffer, sizeof(buffer));
        if (bytesRead == 0) {
            break;
        }
        if (bytesRead < 0) {
            fprintf(stderr,
                    "Read error while copying file '%s' to '%s': %s\n",
                    src,
                    dest,
                    strerror(errno));
            result = false;
            break;
        }
        const ssize_t bytesWritten = write(output, buffer, bytesRead);
        if (bytesWritten != bytesRead) {
            fprintf(stderr,
                    "Write error while copying file '%s' to '%s': %s\n",
                    src,
                    dest,
                    strerror(errno));
            result = false;
            break;
        }
        totalBytesCopied += bytesRead;
    }
    printf("Copied %zu bytes\n", totalBytesCopied);
    if (totalBytesCopied == 0) {
        return false;
    }

cleanup:
    if (input >= 0) {
        close(input);
    }
    if (output >= 0) {
        close(output);
    }
    return result;
}

static inline bool
fileChanged(const char* filename, struct stat sb)
{
    struct stat current = { 0 };
    if (stat(filename, &current)) {
        perror("stat");
        return true;
    }

    return current.st_mtime != sb.st_mtime;
}