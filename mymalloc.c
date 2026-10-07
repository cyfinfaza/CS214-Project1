#include <stddef.h>
#include <stdio.h>

#define MEMLENGTH 4096

static union {
  char bytes[MEMLENGTH];
  double not_used;
} heap;

typedef struct metadata {
    char occupied;
    struct metadata* next_chunk;
    size_t size;
} metadata;

static void * find_next_free_chunk(metadata *chunk){
    while(chunk->occupied) {
        if (chunk->next_chunk) chunk = chunk->next_chunk;
        else return NULL;
    }
    return chunk;
}

static void coalesce(metadata * c1, metadata * c2) {
    c1->size += sizeof(metadata) + c2->size;
    c1->next_chunk = c2->next_chunk;
}

static void bite(metadata * chunk, size_t amount) {
    chunk->occupied = 1;
    if (chunk->size > amount) {
        if ((chunk->size - amount) < sizeof(metadata)) {
            if (chunk->next_chunk && !chunk->next_chunk->occupied)
                coalesce(chunk, chunk->next_chunk);
            else return;
        }
        metadata * new_chunk = (metadata *)((char *)(chunk + 1) + amount);
        new_chunk->size = chunk->size - sizeof(metadata) - amount;
        new_chunk->next_chunk = chunk->next_chunk;
        new_chunk-> occupied = 0;
        chunk->next_chunk = new_chunk;
        chunk->size = amount;
    }
}

static char region_large_enough(metadata * chunk, size_t size) {
    metadata* iter_chunk = chunk;
    while (size > chunk->size) {
        if ((iter_chunk = iter_chunk->next_chunk) && !iter_chunk->occupied){
            coalesce(chunk, iter_chunk);
        }
        else return 0;
    }
    return 1;
}

void * mymalloc(size_t size, char *file, int line) {
    if (size == 0) return NULL;
    metadata* chunk = (metadata*)heap.bytes;
    if (!(chunk->size)) chunk->size = MEMLENGTH - sizeof(metadata);
    while (chunk = find_next_free_chunk(chunk)) {
        if (region_large_enough(chunk, size)) {
            bite(chunk, size);
            return chunk + 1;
        }
        if (chunk->next_chunk) chunk = chunk->next_chunk;
        else return NULL;
    }
    return NULL;
}

void myfree (void *ptr, char *file, int line) {
    if (!ptr) return;
    ((metadata*)ptr - 1)->occupied = 0;
}