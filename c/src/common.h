#ifndef COMMON_H
#define COMMON_H

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>

typedef uint8_t u8;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;
typedef ssize_t isize;

#define as(T, e) ((T)(e))
#define box(T) (T *)calloc(1, sizeof(T))
#define ARRAY_LEN(a) (sizeof(a) / sizeof(a[0]))

#ifndef LIST_INIT_CAP
#define LIST_INIT_CAP 256
#endif

#define Vec(T) struct { T *items; int count, capacity; }

#define list_reserve(l, expected_capacity)                                     \
    do {                                                                       \
        if ((expected_capacity) > (l)->capacity) {                             \
            if ((l)->capacity == 0) {                                          \
                (l)->capacity = LIST_INIT_CAP;                                 \
            }                                                                  \
            while ((expected_capacity) > (l)->capacity) {                      \
                (l)->capacity *= 2;                                            \
            }                                                                  \
            (l)->items =                                                       \
                realloc((l)->items, (l)->capacity * sizeof(*(l)->items));      \
            assert((l)->items != NULL);                                        \
        }                                                                      \
    } while (0)

#define list_append(l, item)                                                   \
    do {                                                                       \
        list_reserve((l), (l)->count + 1);                                     \
        (l)->items[(l)->count++] = (item);                                     \
    } while (0)

#define list_free(l) free((l).items)

#define list_remove_unordered(l, i)                                            \
    do {                                                                       \
        assert((i) < (l)->count);                                              \
        (l)->items[i] = (l)->items[--(l)->count];                              \
    } while (0)

#define US_PER_SEC ((i64)1000000LL)

static inline i64 time_us(void) {
    i64 t = clock();
    return (t * US_PER_SEC) / (i64)CLOCKS_PER_SEC;
}

// Forward declaration - app_cleanup defined in main.c
struct App;
void app_cleanup(struct App *app);

#define panic(app, ...)                                                        \
    do {                                                                       \
        fprintf(stderr, "[ERROR] Panicked at %s:%u\n -> ", __FILE__, __LINE__);\
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        if (app) app_cleanup(app);                                             \
        abort();                                                               \
    } while (0)

#define fatal(app, ...)                                                        \
    do {                                                                       \
        fprintf(stderr, "[ERROR] ");                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        if (app) app_cleanup(app);                                             \
        abort();                                                               \
    } while (0)

#endif // COMMON_H
