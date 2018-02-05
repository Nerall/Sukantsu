#include "core/detect.h"
#include "core/hand.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void print_histo(struct histogram *histo) {
	printf("----------------------------------\n");

	printf("| Index      |");
	for (int i = 1; i < 10; ++i)
		printf(" %d", i);
	printf(" |\n");

	printf("----------------------------------\n");

	printf("| Dot    (p) |");
	for (int i = 0; i < 9; ++i)
		printf(" %d", histo->cells[i]);
	printf(" |\n");

	printf("| Bamboo (s) |");
	for (int i = 9; i < 18; ++i)
		printf(" %d", histo->cells[i]);
	printf(" |\n");

	printf("| Cracks (m) |");
	for (int i = 18; i < 27; ++i)
		printf(" %d", histo->cells[i]);
	printf(" |\n");

	printf("| Honor  (z) |");
	for (int i = 27; i < 34; ++i)
		printf(" %d", histo->cells[i]);
	printf("     |\n");

	printf("----------------------------------\n");
}

static void print_groups(struct group *groups) {
	printf("Groups:\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		switch (groups[i].type) {
			case PAIR:
				printf("Pair (%d, %d)\n", groups[i].tile, groups[i].tile);
				break;
			case SEQUENCE:
				printf("Sequence (%d, %d, %d)\n", groups[i].tile,
				       groups[i].tile + 1, groups[i].tile + 2);
				break;
			case TRIPLET:
				printf("Triplet (%d, %d, %d)\n", groups[i].tile, groups[i].tile,
				       groups[i].tile);
				break;
			case QUAD:
				printf("Quad (%d, %d, %d, %d)\n", groups[i].tile,
				       groups[i].tile, groups[i].tile, groups[i].tile);
				break;
			default:
				fprintf(stderr, "print_groups: enum type not recongized: %d\n",
				        groups[i].type);
				break;
		}
	}
	printf("\n");
}

static void print_victory(struct hand *hand, struct grouplist *grouplist) {
	struct histogram histo;
	groups_to_histo(hand, &histo);

	print_histo(&histo);
	if (iskokushi(hand))
		printf("WOW, Thirteen orphans!!\n\n");
	else {
		if (ischiitoi(hand))
			printf("WOW, Seven pairs!\n\n");
		for (int i = 0; i < grouplist->nb_groups; ++i)
			print_groups(grouplist->groups[i]);
	}
}

static int opponent_discard(struct hand *hand, struct grouplist *grouplist,
                            struct histogram *wall, unsigned char player) {
	if (wall->nb_tiles > 14) {
		histo_index_t discard = random_pop_histogram(wall);
		char *players[] = {"Est", "South", "West", "North"};
		printf("%s's discard: %u\n\n", players[player], discard);
		if (get_histobit(&hand->wintiles, discard)) {
			puts("RON!\n");
			add_tile_hand(hand, discard);
			makegroups(hand, grouplist);
			print_victory(hand, grouplist);
			return 1;
		}
	}
	return 0;
}

/*
void clear_stream(FILE *in) {
    int ch;
    clearerr(in);
    do {
        ch = getc(in);
    } while (ch != '\n' && ch != EOF);
    clearerr(in);
}
*/

// Get the next input
// action:
// - 'r': riichi
// - 't': tsumo
// - 'd': discard
// returned int:
// - corresponding tile index
static histo_index_t get_input(struct histogram *histo, char *action) {
	while (1) {
		histo_index_t index;
		char family, number;

		printf("> ");
		fflush(stdout);

		char c = getchar();
		if (c == 't') {
			// Tsumo action
			*action = 't';
			while (getchar() != '\n')
				;
			return NO_TILE_INDEX;
		}

		if (c == 'r' || c == 'd') {
			// Riichi or Discard (explicit)

			// In this line, family is only use to pass unnecessary chars
			while ((family = getchar()) != ' ' && family != '\n')
				;

			*action = c;
			while ((family = getchar()) == ' ' || family == '\n')
				;
		} else {
			// Discard (implicit)
			*action = 'd';
			family = c;
		}

		while ((number = getchar()) == ' ' || number == '\n')
			;

		while (getchar() != '\n')
			;

		// Tile selection
		if (family >= '1' && family <= '9') {
			char tmp = family;
			family = number;
			number = tmp;
		}

		// Familty to index
		if (family == 'p')
			index = 0;
		else if (family == 's')
			index = 9;
		else if (family == 'm')
			index = 18;
		else if (family == 'z')
			index = 27;
		else
			continue;

		// Convert number
		if (number < '1' || number > '9')
			continue;

		index += number - '1';

		if (!is_valid_index(index) || !histo->cells[index])
			continue;

		return index;
	}
}

int play() {
	// Initialization
	struct histogram wall;
	init_histogram(&wall, 4);
	struct hand hand;
	init_hand(&hand);
	struct grouplist grouplist;
	// Next line is for tests
	// histo_cell_t starthand[] = { 0, 0, 9, 9, 18, 18, 27, 27, 29, 29, 31, 32,
	// 33 };

	// Give 13 tiles to each player
	for (int i = 0; i < 13; ++i) {
		// add_tile_hand(&hand, starthand[i]);
		add_tile_hand(&hand, random_pop_histogram(&wall));
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
		random_pop_histogram(&wall);
	}

	// To initialize the waits
	tenpailist(&hand, &grouplist);
	// Main loop
	while (wall.nb_tiles > 14) {
		// Give one tile to player
		histo_index_t randi = random_pop_histogram(&wall);
		add_tile_hand(&hand, randi);
		hand.last_tile = randi;
		printf("\n-------------------------------\n\n");
		printf("Remaining tiles: %u\n", (wall.nb_tiles - 14));
		printf("Tile drawn: %u\n\n", randi);

		if (get_histobit(&hand.wintiles, randi)) {
			puts("TSUMO!\n");
			makegroups(&hand, &grouplist);
			print_victory(&hand, &grouplist);
			return 1;
		}

		print_histo(&hand.histo);

		// Show best discards
		tilestodiscard(&hand, &grouplist);
		if (hand.tenpai) {
			printf("You are tenpai if you discard:\n");
			for (int r = 0; r < 34; ++r) {
				if (get_histobit(&hand.riichitiles, r))
					printf("%u\n", r);
			}
			printf("\n");
		}

		// Ask for tile discard
		char action;
		histo_index_t index = get_input(&hand.histo, &action);

		if (action == 'd') {
			// Discard
			remove_tile_hand(&hand, index);
		} else if (action == 'r') {
			// Riichi
			printf("action -> riichi\n");
		} else if (action == 't') {
			// Tsumo
			printf("action -> tsumo\n");
		} else {
			// Something went wrong...
			fprintf(stderr, "Well, someone did not do his job\n");
		}

		printf("\n");

		// Show winning tiles
		tenpailist(&hand, &grouplist);
		if (hand.tenpai) {
			printf("You win if you get:\n");
			for (int w = 0; w < 34; ++w) {
				if (get_histobit(&hand.wintiles, w))
					printf("%u\n", w);
			}
			printf("\n");
		}

		// Give one tile to each other player
		if (opponent_discard(&hand, &grouplist, &wall, 1))
			return 1;
		if (opponent_discard(&hand, &grouplist, &wall, 2))
			return 1;
		if (opponent_discard(&hand, &grouplist, &wall, 3))
			return 1;
	}
	printf("End of the game.\n");
	return 0;
}

int main() {
	srand(time(NULL));

	printf("Sizeof structures:\n");
	printf("\thistogram : %lu\n", sizeof(struct histogram));
	printf("\thistobit  : %lu\n", sizeof(struct histobit));
	printf("\tgroup     : %lu\n", sizeof(struct group));
	printf("\thand      : %lu\n", sizeof(struct hand));

	char c;
	do {
		play();
		printf("Do you want to continue (y/n)\n");
		do {
			c = getchar();
		} while (c != 'y' && c != 'n');
	} while (c != 'n');
}
