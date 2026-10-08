#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#ifndef REALMALLOC
#include "mymalloc.h"
#endif

#define RUNS 50
#define OBJECTS 120

/*
 * Allocate objects of sizes 8 to 1024, then free them in reverse order
 */
void task1(void) {
  char *p8 = malloc(8);
  char *p16 = malloc(16);
  char *p32 = malloc(32);
  char *p64 = malloc(64);
  char *p128 = malloc(128);
  char *p512 = malloc(512);
  char *p1024 = malloc(1024);

  free(p1024);
  free(p512);
  free(p128);
  free(p64);
  free(p32);
  free(p16);
  free(p8);
}

/*
 * Allocate 120 1-byte objects, then free them in allocation order
 */
void task2(void) {
  char *obj[OBJECTS];
  int i;

  for (i = 0; i < OBJECTS; i++) {
    obj[i] = malloc(1);
  }

  for (i = 0; i < OBJECTS; i++) {
    free(obj[i]);
  }
}

/*
 * Randomly allocate 1-byte objects or free a random one until 120 allocations,
 * then free the rest
 */
void task3(void) {
  char *obj[OBJECTS];
  int allocated = 0;
  int count = 0;
  int i;

  while (allocated < OBJECTS) {
    if (count == 0 || rand() % 2 == 0) {
      obj[count] = malloc(1);
      count++;
      allocated++;
    } else {
      i = rand() % count;
      free(obj[i]);
      obj[i] = obj[count - 1];
      count--;
    }
  }

  for (i = 0; i < count; i++) {
    free(obj[i]);
  }
}

/*
 * Allocate objects of sizes 8 to 3000, freeing each one right after it is
 * allocated
 */
void task4(void) {
  char *p;

  p = malloc(8);
  free(p);
  p = malloc(16);
  free(p);
  p = malloc(32);
  free(p);
  p = malloc(64);
  free(p);
  p = malloc(128);
  free(p);
  p = malloc(256);
  free(p);
  p = malloc(512);
  free(p);
  p = malloc(1024);
  free(p);
  p = malloc(2048);
  free(p);
  p = malloc(3000);
  free(p);
}

/*
 * Allocate 50 40-byte objects, free every other one, fill the holes with
 * 16-byte objects, free everything, then allocate one 3000-byte object
 */
void task5(void) {
  char *big[50];
  char *small[25];
  char *p;
  int i;

  for (i = 0; i < 50; i++) {
    big[i] = malloc(40);
  }

  for (i = 0; i < 50; i += 2) {
    free(big[i]);
  }

  for (i = 0; i < 25; i++) {
    small[i] = malloc(16);
  }

  for (i = 1; i < 50; i += 2) {
    free(big[i]);
  }

  for (i = 0; i < 25; i++) {
    free(small[i]);
  }

  p = malloc(3000);
  free(p);
}

int main(int argc, char **argv) {
  struct timeval start, end;
  double elapsed;
  int run;

  gettimeofday(&start, NULL);

  for (run = 0; run < RUNS; run++) {
    task1();
    task2();
    task3();
    task4();
    task5();
  }

  gettimeofday(&end, NULL);

  elapsed =
      (end.tv_sec - start.tv_sec) * 1000000.0 + (end.tv_usec - start.tv_usec);
  printf("Average time per workload: %f microseconds\n", elapsed / RUNS);

  return EXIT_SUCCESS;
}
