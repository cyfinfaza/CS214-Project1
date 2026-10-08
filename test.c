#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mymalloc.h"

#define MEMSIZE 4096
#define HEADERSIZE 24
#define OBJECTS 64
#define OBJSIZE (MEMSIZE / OBJECTS - HEADERSIZE)

/*
 * Fill the heap with objects, write a distinct byte to each, and check none were overwritten
 * This checks whether mymalloc messes with the data (it should not)
 */
int
test_overlap (void)
{
	char *obj[OBJECTS];
	int i, j, errors = 0;

	for (i = 0; i < OBJECTS; i++) {
		obj[i] = malloc (OBJSIZE);
		if (obj[i] == NULL) return 0;
	}

	for (i = 0; i < OBJECTS; i++) {
		memset (obj[i], i, OBJSIZE);
	}

	for (i = 0; i < OBJECTS; i++) {
		for (j = 0; j < OBJSIZE; j++) {
			if (obj[i][j] != i) errors++;
		}
	}

	for (i = 0; i < OBJECTS; i++) {
		free (obj[i]);
	}

	return errors == 0;
}

/*
 * Fill the heap, check malloc fails, free one object, check malloc succeeds again
 * This checks whether free actually allows us to use the memory again.
 */
int
test_reuse (void)
{
	char *obj[OBJECTS];
	char *p;
	int i;

	for (i = 0; i < OBJECTS; i++) {
		obj[i] = malloc (OBJSIZE);
		if (obj[i] == NULL) return 0;
	}

	p = malloc (1);
	if (p != NULL) return 0;

	free (obj[10]);

	p = malloc (1);
	if (p == NULL) return 0;

	free (p);
	for (i = 0; i < OBJECTS; i++) {
		if (i != 10) free (obj[i]);
	}

	return 1;
}

/*
 * Fill the heap with small objects, free them all, then allocate one object the size of the whole heap
 * This checks that after we free a bunch of small chunks, they will all coalesce so we can use them for a large block.
 */
int
test_coalesce (void)
{
	char *obj[OBJECTS];
	char *p;
	int i;

	for (i = 0; i < OBJECTS; i++) {
		obj[i] = malloc (OBJSIZE);
		if (obj[i] == NULL) return 0;
	}

	for (i = 0; i < OBJECTS; i++) {
		free (obj[i]);
	}

	p = malloc (MEMSIZE - HEADERSIZE);
	if (p == NULL) return 0;

	free (p);
	return 1;
}

/*
 * Allocate objects of odd sizes and check every pointer is 8-byte aligned
 * This checks that alignment is implemented properly.
 */
int
test_align (void)
{
	int sizes[] = {1, 3, 7, 13, 21, 33};
	char *obj[6];
	int i, ok = 1;

	for (i = 0; i < 6; i++) {
		obj[i] = malloc (sizes[i]);
		if (obj[i] == NULL || (uintptr_t)obj[i] % 8 != 0) ok = 0;
	}

	for (i = 0; i < 6; i++) {
		free (obj[i]);
	}

	return ok;
}

/*
 * Free an address that did not come from malloc (should exit with status 2)
 * This checks that the correct error message is printed for this failure condition.
 */
int
test_notmalloc (void)
{
	printf ("this test passed if an error is shown below\n");
	fflush (stdout);
	int x;
	free (&x);
	return 0;
}

/*
 * Free an address that is not at the start of a chunk (should exit with status 2)
 * This checks that the correct error message is printed for this failure condition.
 */
int
test_offset (void)
{
	printf ("this test passed if an error is shown below\n");
	fflush (stdout);
	int *p = malloc (sizeof (int) * 2);
	free (p + 1);
	return 0;
}

/*
 * Free the same pointer twice (should exit with status 2)
 * This checks that the correct error message is printed for this failure condition.
 */
int
test_double (void)
{
	printf ("this test passed if an error is shown below\n");
	fflush (stdout);
	int *p = malloc (sizeof (int) * 100);
	int *q = p;
	free (p);
	free (q);
	return 0;
}

/*
 * Request more memory than the heap holds (should print an error and return NULL)
 * This checks that the correct error message is printed for this failure condition.
 */
int
test_toobig (void)
{
	printf ("this test passed if an error is shown below\n");
	fflush (stdout);
	char *p = malloc (5000);
	return p == NULL;
}

/*
 * Leak three objects (should report "mymalloc: 128 bytes leaked in 3 objects." at exit)
 * This checks that the correct error message is printed for this failure condition.
 */
int
test_leak (void)
{
	printf ("**ignore PASS** this test passed if leaks are shown below (but exit status 0)\n");
	fflush (stdout);
	malloc (8);
	malloc (16);
	malloc (100);
	return 1;
}

int
main (int argc, char **argv)
{
	int result;

	if (argc != 2) {
		printf ("usage: %s overlap|reuse|coalesce|align|notmalloc|offset|double|toobig|leak\n", argv[0]);
		return EXIT_FAILURE;
	}

	printf ("=== %s ===\n", argv[1]);
	fflush (stdout);

	if (strcmp (argv[1], "overlap") == 0) result = test_overlap ();
	else if (strcmp (argv[1], "reuse") == 0) result = test_reuse ();
	else if (strcmp (argv[1], "coalesce") == 0) result = test_coalesce ();
	else if (strcmp (argv[1], "align") == 0) result = test_align ();
	else if (strcmp (argv[1], "notmalloc") == 0) result = test_notmalloc ();
	else if (strcmp (argv[1], "offset") == 0) result = test_offset ();
	else if (strcmp (argv[1], "double") == 0) result = test_double ();
	else if (strcmp (argv[1], "toobig") == 0) result = test_toobig ();
	else if (strcmp (argv[1], "leak") == 0) result = test_leak ();
	else {
		printf ("unknown test: %s\n", argv[1]);
		return EXIT_FAILURE;
	}

	printf ("%s: %s\n", argv[1], result ? "PASS" : "FAIL");
	return result ? EXIT_SUCCESS : EXIT_FAILURE;
}
