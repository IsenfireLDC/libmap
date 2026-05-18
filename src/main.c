#include <map.h>

#include <stdio.h>
#include <stdlib.h>


static void mi_displayPairs(void *restrict, const void *restrict key, size_t keySize, void *restrict value, size_t valueSize) {
	const char *keyData = key;
	const char *valueData = value;

	putchar('[');
	for(size_t i = 0; i < keySize; ++i) { printf("%02hhx", keyData[i]); };
	printf("]: ");
	for(size_t i = 0; i < valueSize; ++i) { printf("%02hhx", valueData[i]); };
	printf("\n");
};

int main(void) {
	Map map;
	map_create(&map, map_hash, map_eq);

	int testV = 0;

	map_insert(&map, map_dGeneral(testV), map_dString("Hello, World!"));
	testV = 27;
	map_insert(&map, map_dString("String Key"), map_dGeneral(testV));

	if(!map_get(&map, map_dString("String Key"), map_dGeneral(testV))) fprintf(stderr, "Failed to read by string key\n");
	if(testV != 27) fprintf(stderr, "Got incorrect test value\n");

	testV = 0;
	size_t strLen = map_sizeOf(&map, map_dGeneral(testV));
	char *str = alloca(strLen);
	str[0] = 0;
	if(!map_get(&map, map_dGeneral(testV), str, strLen)) fprintf(stderr, "Failed to read by int key\n");
	printf("Loaded string value '%s' from map\n", str);

	printf("Triggering auto rehash\n");
	for(int i = 0; i < 8; ++i) {
		int iPP = i + 1;
		map_insert(&map, map_dGeneral(i), map_dGeneral(iPP));
	};

	printf("Manual rehash\n");
	map_rehash(&map, 32);
	printf("Full dump:\n");
	map_foreach(&map, mi_displayPairs, NULL);

	printf("Replace value\n");
	map_insert(&map, map_dGeneral(testV), map_dGeneral(strLen));
	printf("Allocation test\n");
	char *allocData = map_allocate(&map, map_dString("AllocTest"), 1024);
	for(int i = 0; i < 1024; ++i) {
		allocData[i] = i;
	};
	printf("Full dump:\n");
	map_foreach(&map, mi_displayPairs, NULL);

	printf("Clear\n");
	map_clear(&map);
	printf("Full dump:\n");
	map_foreach(&map, mi_displayPairs, NULL);

	map_destroy(&map);

	return 0;
};
