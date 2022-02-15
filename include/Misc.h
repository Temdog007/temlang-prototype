#pragma once

#define TEMLANG_MAJOR_VERSION 0
#define TEMLANG_MINOR_VERSION 1
#define TEMLANG_REVISION 1

#define KB(X) (X * 1024)
#define MB(X) (KB(X) * 1024)
#define GB(X) (MB(X) * 1024)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))
#define CLAMP(x, min, max) (MIN(MAX(min, x), max))

#define MILLI_TO_MICRO(X) (X * 1000)
#define SECONDS_TO_MILLI(X) (X * 1000)
