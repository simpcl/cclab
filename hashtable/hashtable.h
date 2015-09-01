#ifndef _HASHTABLE_H_
#define _HASHTABLE_H_

enum HT_ERROR_CODE {
    HT_OK = 0,
    HT_INVALID_PARAM,
    HT_INVALID_HASHTABLE,
    HT_EXISTS,
    HT_NOT_FOUND,
    HT_OUT_OF_MEMORY
};

typedef void * (*HT_ALLOC_FUNC)(size_t n);
typedef void (*HT_FREE_FUNC)(void* p);
    
typedef struct _HashItem {
    struct _HashItem *next;
    struct _HashItem *prev;
    struct _HashItem *h_next;    /* hash chain next */
    int access;
    int key_length;
    int key_offset;
    void * end[];
} HashItem;

typedef struct _HashTable {
    unsigned int hash_items;
    HashItem** primary_hashtable;
    HashItem* head;
    HashItem* tail;
    HT_ALLOC_FUNC alloc_func;
    HT_FREE_FUNC free_func;
} HashTable;

typedef int (*HT_APPLY_FUNC)(HashTable* ht, HashItem* hi);

typedef unsigned long int  ub4;   /* unsigned 4-byte quantities */
typedef unsigned      char ub1;   /* unsigned 1-byte quantities */

#define hashsize(n) ((ub4)1<<(n))
#define hashmask(n) (hashsize(n)-1)

#define HASHITEM_KEY(hi)  ((char*) &((hi)->end[0]) + (hi)->key_offset)

inline void* hashitem_alloc(HashTable* ht, size_t n);
inline void hashitem_free(HashTable* ht, void* p);
inline void hashitem_fill(HashItem* hi, size_t offset, const char* key, size_t nkey);
int hashtable_init(HashTable* ht, HT_ALLOC_FUNC alloc_func, HT_FREE_FUNC free_func);
int hashtable_clear(HashTable* ht);
int hashtable_destroy(HashTable* ht);
HashItem* hashtable_find(HashTable* ht, const char *key, const size_t nkey);
int hashtable_insert(HashTable* ht, HashItem* hi);
int hashtable_remove(HashTable* ht, const char *key, const size_t nkey, HashItem** item);
int hashtable_apply(HashTable* ht, HT_APPLY_FUNC apply_func);
int hashtable_apply2(HashTable* ht, int n, HT_APPLY_FUNC apply_func);

#endif //_HASHTABLE_H_
