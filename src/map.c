#include "map.h"

#include <string.h>
#include <stdlib.h>

#define HASH_INIT 0x38a95be9


static void createNode(MapNode *restrict, map_hash_t, const void *restrict, size_t, const void *restrict, size_t);
static void destroyNode(MapNode*);

static bool keyEqual(const Map *restrict, const MapNode *restrict, map_hash_t, const void *restrict, size_t);

static MapNode** findInsertionPoint(const Map *restrict, map_hash_t);
static MapNode** findNodeRef(const Map *restrict, map_hash_t, const void *restrict, size_t);


void map_create(Map *this, MapHash hash, MapEq eq) {
	memcpy((void*)&this->hash, &hash, sizeof(hash));
	memcpy((void*)&this->eq, &eq, sizeof(eq));

	this->maxLoad = MAP_DEFAULT_MAX_LOAD;
	this->buckets = MAP_DEFAULT_BUCKETS;

	this->data = calloc(this->buckets, sizeof(MapNode*));
	this->size = 0;
};

void map_destroy(Map *this) {
	map_clear(this);

	free(this->data);
};



void map_swap(Map *this, Map *other) {
	if(!this || !other) return;

	Map temp = *this;
	memcpy(this, other, sizeof(*this));
	memcpy(other, &temp, sizeof(*this));
};



void map_clear(Map *this) {
	for(size_t i = 0; i < this->buckets; ++i) {
		MapNode *node = this->data[i];

		while(node) {
			MapNode *next = node->next;

			destroyNode(node);
			free(node);

			node = next;
		};

		this->data[i] = NULL;
	};

	this->size = 0;
};

void map_rehash(Map *this, size_t buckets) {
	MapNode **data = this->data;
	this->data = calloc(buckets, sizeof(MapNode*));

	size_t oldBuckets = this->buckets;
	this->buckets = buckets;
	for(size_t i = 0; i < oldBuckets; ++i) {
		MapNode *node = data[i];

		while(node) {
			MapNode *next = node->next;

			MapNode **insRef = findInsertionPoint(this, node->hash);
			node->next = NULL;
			(*insRef) = node;

			node = next;
		};
	};

	free(data);
};



void map_insert(Map *restrict this, const void *restrict key, size_t keySize, const void *restrict value, size_t valueSize) {
	map_hash_t hash = this->hash(key, keySize);

	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);
	if(nodeRef) {
		// Update existing entry
		MapNode *next = (*nodeRef)->next;

		destroyNode(*nodeRef);
		createNode(*nodeRef, hash, key, keySize, value, valueSize);

		(*nodeRef)->next = next;
	} else {
		// Create new entry
		nodeRef = findInsertionPoint(this, hash);

		*nodeRef = malloc(sizeof(MapNode));
		createNode(*nodeRef, hash, key, keySize, value, valueSize);

		++this->size;
		if(this->size > this->buckets * this->maxLoad) map_rehash(this, this->buckets << 1);
	};
};

void map_erase(Map *restrict this, const void *restrict key, size_t keySize) {
	map_hash_t hash = this->hash(key, keySize);

	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);
	if(!nodeRef) return;

	// Remove node from linked list
	MapNode *node = *nodeRef;
	*nodeRef = node->next;

	destroyNode(node);
	free(node);

	--this->size;
};

void* map_allocate(Map *restrict this, const void *restrict key, size_t keySize, size_t valueSize) {
	map_hash_t hash = this->hash(key, keySize);

	MapNode *node;
	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);
	if(nodeRef) {
		if((*nodeRef)->valueSize != valueSize) {
			// Update existing entry
			MapNode *next = (*nodeRef)->next;

			destroyNode(*nodeRef);
			createNode(*nodeRef, hash, key, keySize, NULL, valueSize);

			(*nodeRef)->next = next;
		}

		node = *nodeRef;
	} else {
		// Create new entry
		nodeRef = findInsertionPoint(this, hash);

		*nodeRef = malloc(sizeof(MapNode));
		createNode(*nodeRef, hash, key, keySize, NULL, valueSize);

		// Save pointer in case we need to rehash
		node = *nodeRef;

		++this->size;
		if(this->size > this->buckets * this->maxLoad) map_rehash(this, this->buckets << 1);
	};

	return node->value;
};



bool map_contains(const Map *restrict this, const void *restrict key, size_t keySize) {
	map_hash_t hash = this->hash(key, keySize);
	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);

	return ! !nodeRef;
};


size_t map_sizeOf(const Map *restrict this, const void *restrict key, size_t keySize) {
	map_hash_t hash = this->hash(key, keySize);
	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);

	if(!nodeRef) return 0;

	return (*nodeRef)->valueSize;
};

bool map_get(const Map *restrict this, const void *restrict key, size_t keySize, void *restrict value, size_t valueSize) {
	map_hash_t hash = this->hash(key, keySize);
	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);
	if(!nodeRef) return false;

	size_t copySize = (*nodeRef)->valueSize;
	if(valueSize < copySize) copySize = valueSize;
	memcpy(value, (*nodeRef)->value, copySize);

	return true;
};

void* map_getref(Map *restrict this, const void *restrict key, size_t keySize) {
	map_hash_t hash = this->hash(key, keySize);
	MapNode **nodeRef = findNodeRef(this, hash, key, keySize);
	if(!nodeRef) return NULL;

	return (*nodeRef)->value;
};



void map_foreach(
	Map *restrict this,
	void(*fn)(void *restrict, const void *restrict, size_t, void *restrict, size_t),
	void *restrict context
) {
	for(size_t i = 0; i < this->buckets; ++i) {
		MapNode *const *bucketRef = &this->data[i];

		while(*bucketRef) {
			const MapNode *node = *bucketRef;

			fn(context, node->key, node->keySize, node->value, node->valueSize);

			bucketRef = &node->next;
		};
	};
};




map_hash_t map_hash(const void *data, size_t size) {
	return map_hashNext(HASH_INIT, data, size);
};

map_hash_t map_hashNext(map_hash_t hash, const void *data, size_t size) {
	const uint32_t *d32 = data;
	for(size_t i = 0; i < size; i += 4) {
		uint32_t chunk = 0;
		if(size - i < 4) {
			memcpy(&chunk, d32, size - i);
		} else {
			chunk = *d32++;
		};

		hash ^= chunk;
		hash += hash >> 7;
		hash ^= hash << 17;
	};

	return hash;
};

bool map_eq(const void *restrict a, size_t aSize, const void *restrict b, size_t bSize) {
	if(aSize != bSize) return false;

	return memcmp(a, b, aSize) == 0;
};





static void createNode(
	MapNode *restrict node,
	map_hash_t hash, const void *restrict key, size_t keySize, const void *restrict value, size_t valueSize
) {
	node->next = NULL;
	node->key = malloc(keySize);
	node->keySize = keySize;
	node->value = valueSize ? malloc(valueSize) : NULL;
	node->valueSize = valueSize;

	memcpy((void*)node->key, key, keySize);
	if(value) memcpy(node->value, value, valueSize);

	node->hash = hash;
};

static void destroyNode(MapNode *node) {
	free((void*)node->key);
	free(node->value);
};


static bool keyEqual(
	const Map *restrict this,
	const MapNode *restrict node, map_hash_t hash, const void *restrict key, size_t keySize
) {
	if(!node) return false;
	if(node->hash != hash) return false;

	// Pointers differ and ( one is null or contents differ )
	if(node->key != key && (!node->key || !key || !this->eq(node->key, node->keySize, key, keySize))) return false;

	return true;
};


static MapNode** findNodeRef(const Map *restrict this, map_hash_t hash, const void *restrict key, size_t keySize) {
	MapNode *const *bucketRef = &this->data[hash % this->buckets];

	while(*bucketRef) {
		const MapNode *node = *bucketRef;

		if(keyEqual(this, node, hash, key, keySize)) return (MapNode**)bucketRef;

		bucketRef = &node->next;
	};

	return NULL;
};


static MapNode** findInsertionPoint(const Map *this, map_hash_t hash) {
	MapNode *const *bucketRef = &this->data[hash % this->buckets];

	while(*bucketRef) bucketRef = &(*bucketRef)->next;

	return (MapNode**)bucketRef;
};
