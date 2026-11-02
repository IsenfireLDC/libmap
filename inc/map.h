#ifndef _MAP_H_
#define _MAP_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#define MAP_DEFAULT_MAX_LOAD 0.75f
#define MAP_DEFAULT_BUCKETS 8

typedef uint32_t map_hash_t;
typedef map_hash_t MapHash(const void*, size_t);
typedef bool MapEq(const void *restrict, size_t, const void *restrict, size_t);

typedef struct MapNode {
	struct MapNode *next;

	const void *key;
	size_t keySize;

	void *value;
	size_t valueSize;

	map_hash_t hash;
} MapNode;

typedef struct Map {
	MapHash *const hash;
	MapEq *const eq;

	float maxLoad;
	size_t buckets;

	MapNode **data;
	size_t size;
} Map;

void map_create(Map*, MapHash, MapEq);
void map_destroy(Map*);

void map_swap(Map*, Map*);

void map_clear(Map*);
void map_rehash(Map*, size_t);

void map_insert(Map *restrict, const void *restrict, size_t, const void *restrict, size_t);
void map_erase(Map *restrict, const void *restrict, size_t);
void* map_allocate(Map *restrict, const void *restrict, size_t, size_t);

bool map_contains(const Map *restrict, const void *restrict, size_t);

size_t map_sizeOf(const Map *restrict, const void *restrict, size_t);
bool map_get(const Map *restrict, const void *restrict, size_t, void *restrict, size_t);
void* map_getref(Map *restrict, const void *restrict, size_t);

void map_foreach(
	Map *restrict,
	void(*)(void *restrict, const void *restrict, size_t, void *restrict, size_t),
	void *restrict
);


map_hash_t map_hash(const void*, size_t);
map_hash_t map_hashNext(map_hash_t, const void*, size_t);
bool map_eq(const void *restrict, size_t, const void *restrict, size_t);


#include <string.h>
#define map_dString(s) (s), (strlen(s)+1)
#define map_dGeneral(g) &(g), sizeof(g)
#define map_dNULL NULL, 0

#endif
