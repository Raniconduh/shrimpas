#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdint.h>
#include <stddef.h>


// collision list
typedef struct col_list {
	struct col_list * next;
	struct col_list * prev;

	unsigned long hash;
	char * key;
	void * val;
} col_list;


typedef struct {
	col_list * entries;
	col_list * last;
} bucket;


typedef struct {
	void (*destroyer)(void*);
	bucket ** buckets;
	size_t len; // total entries
	size_t max;
} hashmap;


typedef struct {
	size_t curbuck;
	col_list * col;

	char * key;
	void * val;
} hashmap_walk_state;


hashmap * new_hashmap(void (*destroyer)(void*));
void hashmap_insert(hashmap *, char *, void *);
void * hashmap_get(hashmap *, char *);
void hashmap_remove(hashmap *, char *);
int hashmap_walk(hashmap *, hashmap_walk_state *);
void destroy_hashmap(hashmap *);

#endif /* HASHMAP_H */
