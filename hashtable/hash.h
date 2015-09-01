#ifndef HASH_H
#define HASH_H

#ifdef    __cplusplus
extern "C" {
#endif

#define ENDIAN_LITTLE 1
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
uint32_t hash(const void *key, size_t length, const uint32_t initval);

#ifdef    __cplusplus
}
#endif

#endif    /* HASH_H */

