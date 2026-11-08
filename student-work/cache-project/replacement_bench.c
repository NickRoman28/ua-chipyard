/* Replacement-policy benchmark for an 8 KiB, 4-way, 64-byte-line D-cache
 * (32 sets, so addresses 2 KiB apart map to the same set).
 *
 *   fit       3 lines compete per set: everything fits, so policy should not
 *             matter (control).
 *   hot_scan  3 reused lines plus 1 never-reused line per set: LRU keeps the
 *             reused lines, while Random sometimes evicts them.
 *   loop5     5 lines cycle through each 4-way set: LRU evicts each line just
 *             before its reuse, while Random keeps some by chance.
 *
 * Each phase starts from the same cache contents (see reset_cache), and every
 * config must print the same CHECKSUM line.
 */

#include <stdint.h>
#include <stdio.h>

#include "metrics.h"

#define KIB           1024u
#define LINE            64u
#define NSETS           32u
#define WAY_SPAN      (NSETS * LINE)              /* 2 KiB */

#define FIT_SIZE      (6u * KIB)                  /* 3 lines per set */
#define FIT_PASSES     100u

#define HOT_WAYS         3u                       /* 6 KiB */
#define HOT_SIZE      (HOT_WAYS * WAY_SPAN)
#define HOT_PASSES      64u
#define SCAN_SIZE     (HOT_PASSES * WAY_SPAN)     /* one new line per set, per pass */

#define LOOP5_SIZE    (10u * KIB)                 /* 5 lines per set */
#define LOOP5_PASSES    50u

#define RESET_SIZE    (16u * KIB)

volatile uint64_t sink;

static uint8_t fit_buf[FIT_SIZE]     __attribute__((aligned(WAY_SPAN)));
static uint8_t hot_buf[HOT_SIZE]     __attribute__((aligned(WAY_SPAN)));
static uint8_t scan_buf[SCAN_SIZE]   __attribute__((aligned(WAY_SPAN)));
static uint8_t loop5_buf[LOOP5_SIZE] __attribute__((aligned(WAY_SPAN)));
static uint8_t reset_buf[RESET_SIZE] __attribute__((aligned(WAY_SPAN)));

static uint64_t ck;

static inline uint64_t load_line(const uint8_t *p)
{
    return *(const volatile uint8_t *)p;
}

static void fill(uint8_t *buf, uint32_t size)
{
    uint32_t i;

    for (i = 0; i < size; i++)
        buf[i] = (uint8_t)(i ^ (i >> 8));
}

/* Untimed: leaves every phase starting from the same cache contents. */
static void reset_cache(void)
{
    uint32_t r, i;
    uint64_t acc = 0;

    for (r = 0; r < 8u; r++)
        for (i = 0; i < RESET_SIZE; i += LINE)
            acc += load_line(&reset_buf[i]);

    sink = acc;
}

static uint64_t walk(const uint8_t *buf, uint32_t size, uint32_t passes)
{
    uint64_t acc = 0;
    uint32_t r, i;

    for (r = 0; r < passes; r++)
        for (i = 0; i < size; i += LINE)
            acc += load_line(&buf[i]);

    return acc;
}

static uint64_t stage_hot_scan(void)
{
    uint64_t acc = 0;
    uint32_t p, s, w;
    uint32_t next = 0;

    for (p = 0; p < HOT_PASSES; p++) {
        for (s = 0; s < NSETS; s++) {
            for (w = 0; w < HOT_WAYS; w++)
                acc += load_line(&hot_buf[(w * WAY_SPAN) + (s * LINE)]);

            /* next advances once per set, so it stays in set s. */
            acc += load_line(&scan_buf[next]);
            next += LINE;
        }
    }

    return acc;
}

static void mix(uint64_t v)
{
    ck = (ck << 5) - ck + v;
    sink = v;
}

int main(void)
{
    printf("replacement_bench\n");

    hpm_setup();

    fill(fit_buf, FIT_SIZE);
    fill(hot_buf, HOT_SIZE);
    fill(scan_buf, SCAN_SIZE);
    fill(loop5_buf, LOOP5_SIZE);
    fill(reset_buf, RESET_SIZE);

    reset_cache();
    {
        uint64_t v;
        BENCH_BEGIN();
        v = walk(fit_buf, FIT_SIZE, FIT_PASSES);
        BENCH_END("fit");
        mix(v);
    }

    reset_cache();
    {
        uint64_t v;
        BENCH_BEGIN();
        v = stage_hot_scan();
        BENCH_END("hot_scan");
        mix(v);
    }

    reset_cache();
    {
        uint64_t v;
        BENCH_BEGIN();
        v = walk(loop5_buf, LOOP5_SIZE, LOOP5_PASSES);
        BENCH_END("loop5");
        mix(v);
    }

    printf("CHECKSUM,%lu\n", (unsigned long)ck);

    return 0;
}
