#include <stdint.h>
#include <stdio.h>

static inline uint32_t xr(uint32_t x) {
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return x;
}

static uint32_t advance(uint32_t x, int n) {
    while (n--) x = xr(x);
    return x;
}

static uint32_t inverse_odd(uint32_t a) {
    uint32_t x = a;
    for (int i = 0; i < 6; ++i) x *= 2u - a * x;
    return x;
}

static uint32_t tab[4][256];

static inline uint32_t after_constructors(uint32_t x) {
    return tab[0][x & 255u]
         ^ tab[1][(x >> 8) & 255u]
         ^ tab[2][(x >> 16) & 255u]
         ^ tab[3][x >> 24];
}

static inline int row_sum(uint32_t *state, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        *state = xr(*state);
        sum += (int)(*state % 10u);
    }
    return sum;
}

int main(void) {
    for (int b = 0; b < 4; ++b)
        for (int v = 0; v < 256; ++v)
            tab[b][v] = advance((uint32_t)v << (8 * b), 72);

    const uint32_t mul = 2654435761u;
    const uint32_t inv = inverse_odd(mul);
    int found = 0;
    for (uint64_t i = 0; i < (1ull << 31); ++i) {
        uint32_t initial = (uint32_t)(2u * (uint32_t)i + 1u);
        uint32_t state = after_constructors(initial);
        int k = row_sum(&state, 6);
        if (row_sum(&state, 4) != k) continue;
        if (row_sum(&state, 6) != k) continue;
        if (row_sum(&state, 4) != k) continue;
        if (row_sum(&state, 5) != k) continue;
        if (row_sum(&state, 5) != k) continue;
        if (row_sum(&state, 6) != k) continue;

        uint32_t raw[2] = { initial, initial - 1u };
        for (int j = 0; j < 2; ++j) {
            uint32_t seed = (raw[j] - 12345u) * inv;
            uint32_t check = (seed * mul + 12345u) | 1u;
            if (seed >= 2u && check == initial) {
                printf("seed=%u initial=%08x sum=%d\n", seed, initial, k);
                ++found;
            }
        }
        if (found >= 4) break;
    }
    return found ? 0 : 1;
}
