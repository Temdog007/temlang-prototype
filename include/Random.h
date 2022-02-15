#pragma once

#include <stdint.h>
#include <time.h>

// Source:
// http://www.math.sci.hiroshima-u.ac.jp/m-mat/MT/VERSIONS/C-LANG/mt19937-64.c

#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL         /* Least significant 31 bits */

typedef struct RandomState
{
    uint64_t mt[NN];
    uint64_t mag01[2];
    int32_t mti;
} RandomState, *pRandomState;

static inline uint64_t
random64(pRandomState s);

static inline RandomState
makeRandomStateWithSeed(const uint64_t seed)
{
    RandomState s = { 0 };
    s.mag01[0] = 0ULL;
    s.mag01[1] = MATRIX_A;
    s.mt[0] = seed;
    for (s.mti = 1; s.mti < NN; ++s.mti) {
        s.mt[s.mti] = (6364136223846793005ULL *
                         (s.mt[s.mti - 1] ^ (s.mt[s.mti - 1] >> 62ULL)) +
                       s.mti);
    }
    return s;
}

static inline RandomState
makeRandomState()
{
    return makeRandomStateWithSeed(time(NULL));
}

static inline uint64_t
random64(pRandomState s)
{
    uint64_t x;
    if (s->mti >= NN) { /* generate NN words at one time */
        int32_t i;

        for (i = 0; i < NN - MM; i++) {
            x = (s->mt[i] & UM) | (s->mt[i + 1] & LM);
            s->mt[i] = s->mt[i + MM] ^ (x >> 1) ^ s->mag01[(int32_t)(x & 1)];
        }
        for (; i < NN - 1; i++) {
            x = (s->mt[i] & UM) | (s->mt[i + 1] & LM);
            s->mt[i] =
              s->mt[i + (MM - NN)] ^ (x >> 1) ^ s->mag01[(int32_t)(x & 1)];
        }
        x = (s->mt[NN - 1] & UM) | (s->mt[0] & LM);
        s->mt[NN - 1] = s->mt[MM - 1] ^ (x >> 1) ^ s->mag01[(int32_t)(x & 1)];

        s->mti = 0;
    }

    x = s->mt[s->mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}