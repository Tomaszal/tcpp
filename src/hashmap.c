#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hashmap.h"

/**
 * 'm' and 'r' are mixing constants generated offline.
 * Used for Murmur
 */
static const unsigned int m = 0x5bd1e995;
static const unsigned int r = 24;

/**
 * <a href="https://sites.google.com/site/murmurhash/">MurmurHash2</a>
 *
 * Generates a 32-bit non-cryptographic hash value for a given key.
 *
 * @author Austin Appleby
 *
 * @param key The key to be hashed.
 * @param length The length of a hash in bytes.
 * @param seed The hash seed.
 * @return Hash value of the given key.
 */
static unsigned int murmur_hash(const void *key, unsigned int length, unsigned int seed) {
    // Initialize the hash to a 'random' value
    unsigned int h = seed ^length;

    const unsigned char *data = (unsigned char *) key;

    // Mix 4 bytes at a time into the hash
    while (length >= 4) {
        unsigned int k = *(unsigned int *) data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        length -= 4;
    }

    // Handle * the last few bytes of the input array
    switch (length) {
        case 3:
            h ^= data[2] << 16u;
        case 2:
            h ^= data[1] << 8u;
        case 1:
            h ^= data[0];
            h *= m;
        default:;
    };

    // Do a few final mixes of the hash to ensure the last few bytes are well-incorporated
    h ^= h >> 13u;
    h *= m;
    h ^= h >> 15u;

    return h;
}

/**
 * Generates a new hash map.
 *
 * @param size Size of the hash map. Bigger hash maps have less collisions.
 * @param seed Hash map hash function seed.
 * @return A newly generated hash map.
 */
HashMap *new_hash_map(unsigned int size, unsigned int seed) {
    HashMap *hash_map = malloc(sizeof *hash_map);

    hash_map->size = size;
    hash_map->seed = seed;

    hash_map->map = calloc((size_t) size, sizeof *(hash_map->map));

    if (!hash_map->map) {
        free(hash_map);
        return NULL;
    }

    return hash_map;
}

/**
 * Deletes a hash map and frees the memory allocated to it.
 *
 * @param hash_map A hash map to delete.
 */
void delete_hash_map(HashMap *hash_map) {
    for (unsigned int i = 0; i < hash_map->size; i++) {
        if (hash_map->map[i]) {
            free(hash_map->map[i]);
        }
    }

    free(hash_map->map);
    free(hash_map);
}

/**
 * Generates a hash value of a given key in a given hash map.
 *
 * @param hash_map A hash map in which to generate the hash.
 * @param key A key for which to generate the hash.
 * @return The generated hash.
 */
static unsigned int hash_map_hash(HashMap *hash_map, const char *key) {
    return murmur_hash(key, (int) strlen(key), hash_map->seed) % hash_map->size;
}

/**
 * Inserts the value to a given key in a hash map.
 *
 * @param hash_map A hash map to insert the value in.
 * @param key A key of the value to be inserted.
 * @param value The value to insert.
 */
void hash_map_insert_key(HashMap *hash_map, const char *key, void *value) {
    hash_map->map[hash_map_hash(hash_map, key)] = value;
}

/**
 * Deletes the value of a given key in a hash map.
 *
 * @param hash_map A hash map from which to delete the value from.
 * @param key A key of the value to be deleted.
 */
void hash_map_delete_key(HashMap *hash_map, const char *key) {
    unsigned int index = hash_map_hash(hash_map, key);
    free(hash_map->map[index]);
    hash_map->map[index] = NULL;
}

/**
 * Retrieves the value of a given key in a hash map.
 *
 * @param hash_map A hash map from which to retrieve the value from.
 * @param key A key of the value to be retrieved.
 * @return The value of a given key.
 */
void *hash_map_get_key(HashMap *hash_map, const char *key) {
    return hash_map->map[hash_map_hash(hash_map, key)];
}
