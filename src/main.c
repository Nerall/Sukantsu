#include "wall.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main() {
	srand(time(NULL));

	puts("Init wall:");
	struct wall wall;
	init_wall(&wall);
	puts("OK");

	puts("Pop random wall:");
	while (wall.nb_tiles) {
		int randi = random_pop_wall(&wall);
		printf("%d ", randi);
	}
	puts("\nOK");
}
