#pragma once

#include <stdio.h>

int
REPL_print(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int result = vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stdout);
    return result;
}

int
TemLangError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int result = vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    return result;
}