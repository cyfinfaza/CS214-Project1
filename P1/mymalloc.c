#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#define MEMLENGTH 4096

// heap definition
static union {
  char bytes[MEMLENGTH];
  double not_used;
} heap;

// memory chunk metadata
typedef struct metadata {
  char occupied;               // 1 if occupied, 0 if not
  struct metadata *next_chunk; // memory address of the next chunk, 0 if there
                               // is no next chunk
  size_t size;                 // capacity of the chunk for real data
} metadata;

/*
 * Finds the first free chunk in the memory starting at a particular chunk.
 * Returns the starting address of the current chunk if it is free, the next
 * free chunk if the current chunk is not free, or null if no free chunks
 * are found after this point.
 */
static void *find_next_free_chunk(metadata *chunk) {
  while (chunk->occupied) {
    if (chunk->next_chunk)
      chunk = chunk->next_chunk;
    else
      return NULL;
  }
  return chunk;
}

/*
 * Combines two adjacent chunks into one.
 * Both chunks should be free before doing this.
 */
static void coalesce(metadata *c1, metadata *c2) {
  c1->size += sizeof(metadata) + c2->size;
  c1->next_chunk = c2->next_chunk;
}

/*
 * Shrink the current chunk to the amount specified and create a new chunk after
 * it. If there is not enough space after the amount specified to fit a header,
 * the chunk will be coalesced with the next chunk if it is free, or otherwise
 * the chunk will not be shrunk, and a new chunk will not be created.
 */
static void bite(metadata *chunk, size_t amount) {
  chunk->occupied = 1;
  if (chunk->size > amount) {
    if ((chunk->size - amount) < sizeof(metadata)) {
      if (chunk->next_chunk && !chunk->next_chunk->occupied)
        coalesce(chunk, chunk->next_chunk);
      else
        return;
    }
    metadata *new_chunk = (metadata *)((char *)(chunk + 1) + amount);
    new_chunk->size = chunk->size - sizeof(metadata) - amount;
    new_chunk->next_chunk = chunk->next_chunk;
    new_chunk->occupied = 0;
    chunk->next_chunk = new_chunk;
    chunk->size = amount;
  }
}

/*
 * Check if a contiguous free memory region starting at the specified chunk has
 * enough space to fit the specified size, coalescing free chunks along the way.
 * Returns 1 if the region is large enough, 0 if not.
 */
static char region_large_enough(metadata *chunk, size_t size) {
  metadata *iter_chunk = chunk;
  while (size > chunk->size) {
    if ((iter_chunk = iter_chunk->next_chunk) && !iter_chunk->occupied) {
      coalesce(chunk, iter_chunk);
    } else
      return 0;
  }
  return 1;
}

/*
 * Runs at exit. Reports any chunks that are still allocated.
 */
static void leak_check(void) {
  size_t bytes = 0;
  int objects = 0;
  for (metadata *chunk = (metadata *)heap.bytes; chunk;
       chunk = chunk->next_chunk) {
    if (chunk->occupied) {
      bytes += chunk->size;
      objects++;
    }
  }
  if (objects)
    fprintf(stderr, "mymalloc: %zu bytes leaked in %d objects.\n", bytes,
            objects);
}

/*
 * Allocate memory for an object of the provided size.
 * Returns a pointer to the memory region, or NULL if the memory could not be
 * allocated.
 */
void *mymalloc(size_t size, char *file, int line) {
  if (size == 0 || size > MEMLENGTH) { // also prevents overflow when rounding
    fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size,
            file, line);
    return NULL;
  }
  size_t requested = size;
  if (size & 7)
    size = ((size >> 3) + 1) << 3; // make size divisible by 8
  metadata *chunk = (metadata *)heap.bytes;
  if (!(chunk->size)) {
    chunk->size = MEMLENGTH - sizeof(metadata);
    atexit(leak_check);
  }
  while ((chunk = find_next_free_chunk(chunk))) {
    if (region_large_enough(chunk, size)) {
      bite(chunk, size);
      return chunk + 1;
    }
    if (chunk->next_chunk)
      chunk = chunk->next_chunk;
    else
      break;
  }
  fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", requested,
          file, line);
  return NULL;
}

/*
 * Free a chunk of memory allocated by mymalloc, given a pointer to the start of
 * its contents. If a bad pointer is provided, the program exits with status 2.
 */
void myfree(void *ptr, char *file, int line) {
  if (!ptr)
    return;
  for (metadata *chunk = (metadata *)heap.bytes; chunk;
       chunk = chunk->next_chunk) {
    if ((void *)(chunk + 1) == ptr) {
      if (!chunk->occupied) {
        fprintf(stderr, "free: Inappropriate pointer (%s:%d): double free\n",
                file, line);
        exit(2);
      }
      chunk->occupied = 0;
      return;
    }
  }
  fprintf(stderr, "free: Inappropriate pointer (%s:%d)\n", file, line);
  exit(2);
}