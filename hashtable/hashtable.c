#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hashtable.h"
#include "hash.h"

#define HASHTABLE_DEBUG 0

/* how many powers of 2's worth of buckets we use */
static unsigned int hashpower = 16;

static HashItem** _hashitem_before(HashTable* ht, const char *key, const size_t nkey)
{
    HashItem **pos;
    uint32_t hv = hash(key, nkey, 0);
    unsigned int bucket = hv & hashmask(hashpower);

    pos = &(ht->primary_hashtable[bucket]);

    while (*pos && ((nkey != (*pos)->key_length) || memcmp(key, HASHITEM_KEY(*pos), nkey))) {
        pos = &(*pos)->h_next;
    }
    return pos;
}

inline void* hashitem_alloc(HashTable* ht, size_t n)
{
    void * p;
    if (!ht || !ht->alloc_func || n <= 0)
        return NULL;
    p = ht->alloc_func(n);
    if (p)
        memset(p, 0, n);
    return p;
}

inline void hashitem_free(HashTable* ht, void* p)
{
    if (ht && ht->free_func && p) {
        ht->free_func(p);
    }
}

inline void hashitem_fill(HashItem* hi, size_t offset, const char* key, size_t nkey)
{
    hi->key_offset = offset;
    sprintf(HASHITEM_KEY(hi), "%s", key);
    hi->key_length = nkey;
}

int hashtable_init(HashTable* ht, HT_ALLOC_FUNC alloc_func, HT_FREE_FUNC free_func)
{
    if (!ht)
        return HT_INVALID_PARAM;

    ht->hash_items = 0;
    ht->head = ht->tail = NULL;
    ht->primary_hashtable = calloc(hashsize(hashpower), sizeof(void *));
    if (!ht->primary_hashtable) {
        fprintf(stderr, "Failed to init hashtable.\n");
        return HT_OUT_OF_MEMORY;
    }

    if (!alloc_func) alloc_func = malloc;
    ht->alloc_func = alloc_func;

    if (!free_func) free_func = free;
    ht->free_func = free_func;

    return HT_OK;
}

int hashtable_clear(HashTable* ht)
{
#if HASHTABLE_DEBUG
    printf("hashtable_clear => begin\n");
#endif
    HashItem *hi, *next;

    if (!ht || !ht->primary_hashtable || !ht->head || !ht->tail) {
#if HASHTABLE_DEBUG
        printf("hashtable_clear => invalid hash table\n");
#endif
        return -1;
    }
    hi = ht->head;
    while (hi) {
        next = hi->next;
        if (hi->prev) hi->prev->next = hi->next;
        if (hi->next) hi->next->prev = hi->prev;
        hi->prev = hi->next = NULL;
        ht->free_func(hi);
        ht->hash_items--;
        hi = next;
    }
    ht->head = ht->tail = NULL;
    memset(ht->primary_hashtable, 0, hashsize(hashpower) * sizeof(void *));
    return 0;
}

int hashtable_destroy(HashTable* ht)
{
    if (hashtable_clear(ht) == 0) {

        free(ht->primary_hashtable);

        ht->primary_hashtable = NULL;

        return 0;
    }

    return -1;
}

HashItem* hashtable_find(HashTable* ht, const char *key, const size_t nkey)
{
    HashItem *hi;
    uint32_t hv;
    unsigned int bucket;

    if (!ht || !key || nkey <= 0)
        return NULL;
 
    hv = hash(key, nkey, 0);
    bucket = hv & hashmask(hashpower);
    hi = ht->primary_hashtable[bucket];

    while (hi && ((nkey != hi->key_length) || memcmp(key, HASHITEM_KEY(hi), nkey))) {
        hi = hi->h_next;
    }
    return hi;
}

int hashtable_insert(HashTable* ht, HashItem* hi)
{
    HashItem* pos;
    uint32_t hv;
    unsigned int bucket;
    char* key;
    size_t nkey;

    if (!ht || !hi)
        return HT_INVALID_PARAM;
    
    key = HASHITEM_KEY(hi);
    nkey = hi->key_length;
    hv = hash(key, nkey, 0);
    bucket = hv & hashmask(hashpower);
    pos = ht->primary_hashtable[bucket];
#if HASHTABLE_DEBUG
    printf("hashtable_insert => key:[%s] hv:[%u] bucket:[%u]\n", key, hv, bucket);
#endif
    while (pos) {
        if ((nkey == pos->key_length) && memcmp(key, HASHITEM_KEY(pos), nkey) == 0)
            return HT_EXISTS;
        pos = pos->h_next;
    }

    hi->h_next = ht->primary_hashtable[bucket];
    ht->primary_hashtable[bucket] = hi;

    hi->next = NULL;
    hi->prev = ht->tail;
    if (ht->tail) ht->tail->next = hi;
    ht->tail = hi;
    if (ht->head == NULL) ht->head = hi;

    ht->hash_items++;

    return HT_OK;
}

int hashtable_remove(HashTable* ht, const char *key, const size_t nkey, HashItem** item)
{
    HashItem** before;
    HashItem* nxt;

    if (!ht || !key || nkey <= 0 || !item)
        return HT_INVALID_PARAM;

    before = _hashitem_before(ht, key, nkey);
    if (*before) {
#if HASHTABLE_DEBUG
        printf("hashtable_remove => key:[%s]\n", key);
#endif
        *item = *before;
        nxt = (*before)->h_next;
        *before = nxt;

#if HASHTABLE_DEBUG
        printf("hashtable_remove => key:[%s]\n", HASHITEM_KEY(*item));
#endif
        (*item)->h_next = NULL;

        if ((*item) == ht->head) ht->head = (*item)->next;
        if ((*item) == ht->tail) ht->tail = (*item)->prev;

        if ((*item)->prev) (*item)->prev->next = (*item)->next;
        if ((*item)->next) (*item)->next->prev = (*item)->prev;
        (*item)->prev = (*item)->next = NULL;

        ht->hash_items--;
        return HT_OK;
    }
    return HT_NOT_FOUND;
}

int hashtable_apply(HashTable* ht, HT_APPLY_FUNC apply_func)
{
    HashItem *hi, *next;

    if (!ht || !apply_func)
        return HT_INVALID_PARAM;

    hi = ht->head;
    while (hi) {
#if HASHTABLE_DEBUG
        printf("hashtable_apply => before func hi:[%p]\n", hi);
#endif
        next = hi->next;
        apply_func(ht, hi);
        hi = next;
    }

    return HT_OK;
}

int hashtable_apply2(HashTable* ht, int n, HT_APPLY_FUNC apply_func)
{
    HashItem *hi, *next;
    int i;

    if (!ht || !apply_func || n <= 0)
        return HT_INVALID_PARAM;

    hi = ht->head;
    i = 0;
    while (hi) {
#if HASHTABLE_DEBUG
        printf("hashtable_apply2 => before func hi:[%p]\n", hi);
#endif
        if (i >= n)
            break;
        next = hi->next;
        if (apply_func(ht, hi))
            i++;
        hi = next;
    }
    if (hi && ht->head && ht->tail) {
        // it is circle
        ht->tail->next = ht->head;
        ht->head->prev = ht->tail;
        // change head and tail
        ht->head = hi;
        ht->tail = hi->prev;
        // line again
        ht->head->prev = NULL;
        ht->tail->next = NULL;
    }
    return HT_OK;
}

