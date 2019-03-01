#ifndef TCPP_HASHMAP_H
#define TCPP_HASHMAP_H

typedef struct HashMap {
    unsigned int size;
    unsigned int seed;

    void **map;
} HashMap;

HashMap *new_hash_map(unsigned int size, unsigned int seed);

void delete_hash_map(HashMap *hash_map);

void hash_map_insert_key(HashMap *hash_map, const char *key, void *value);

void hash_map_delete_key(HashMap *hash_map, const char *key);

void *hash_map_get_key(HashMap *hash_map, const char *key);

#endif //TCPP_HASHMAP_H
