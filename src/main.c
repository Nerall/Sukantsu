#include "core/detect.h"
#include "core/hand.h"
#include "core/detect.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
	printf("\t");
	for (int i = 0; i < HISTO_INDEX_MAX; ++i) {
		printf("%d ", histo->cells[i]);
	}
	printf("\n");
}

// DEBUG FUNCTION
// Will print hand groups to stdout
static void print_groups(struct group *groups) {
	printf("\t-> (hidden, type, tile)\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		printf("\t(%d, %d, %d)\n", groups[i].hidden, groups[i].type,
		       groups[i].tile);
	}
}

int main() {
	srand(time(NULL));
	setbuf(stdout, NULL); // To flush automatically stdout

	puts("Sizeof structures:");
	printf("\thistogram : %lu\n", sizeof(struct histogram));
	printf("\thistobit  : %lu\n", sizeof(struct histobit));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\thand      : %lu\n", sizeof(struct hand));

	
	
}
