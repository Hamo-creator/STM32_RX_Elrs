/*
 * median_avg_filter.h
 *
 *  Created on: Apr 14, 2025
 *      Author: Khalil
 */

#ifndef MEDIAN_AVG_FILTER_H
#define MEDIAN_AVG_FILTER_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#define MEDIAN_AVG_FILTER_DEFINE(type, name, size) \
    typedef struct {                                \
        type data[size];                            \
        unsigned int counter;                       \
    } name;                                          \
                                                    \
    static inline void name##_init(name *f) {       \
        memset(f->data, 0, sizeof(f->data));        \
        f->counter = 0;                              \
    }                                               \
                                                    \
    static inline unsigned int name##_add(name *f, type item) { \
        f->data[f->counter] = item;                 \
        f->counter = (f->counter + 1) % size;       \
        return f->counter;                          \
    }                                               \
                                                    \
    static inline void name##_clear(name *f) {      \
        memset(f->data, 0, sizeof(f->data));        \
        f->counter = 0;                              \
    }                                               \
                                                    \
    static inline type name##_calc_scaled(const name *f) { \
        type minVal = f->data[0];                   \
        type maxVal = f->data[0];                   \
        type sum = f->data[0];                      \
        for (size_t i = 1; i < size; ++i) {         \
            type val = f->data[i];                  \
            sum += val;                             \
            if (val < minVal) minVal = val;         \
            if (val > maxVal) maxVal = val;         \
        }                                           \
        return sum - (minVal + maxVal);             \
    }                                               \
                                                    \
    static inline type name##_calc(const name *f) { \
        return name##_calc_scaled(f) / (size - 2);  \
    }

#endif // MEDIAN_AVG_FILTER_H
