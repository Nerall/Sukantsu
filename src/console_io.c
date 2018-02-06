#include "console_io.h"
#include "debug.h"
#include <stdio.h>

// Convert a tile index to a family and a number characters
histo_index_t char_to_index(char family, char number) {
	ASSERT_BACKTRACE(number >= '1' && number <= '9');
	ASSERT_BACKTRACE(family != 'z' || number <= '7');

	histo_index_t index;

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
		ASSERT_BACKTRACE(0 && "family not recognized");

	// Convert number
	return index + (number - '1');
}

// Convert a family and a number characters to a tile index
void index_to_char(histo_index_t index, char *family, char *number) {
	ASSERT_BACKTRACE(family);
	ASSERT_BACKTRACE(number);
	ASSERT_BACKTRACE(is_valid_index(index));

	int div = index / 9;
	if (div == 0)
		*family = 'p';
	else if (div == 1)
		*family = 's';
	else if (div == 2)
		*family = 'm';
	else
		*family = 'z';

	*number = '1' + (index % 9);
}

// Pretty print an histogram
void print_histo(struct histogram *histo) {
	ASSERT_BACKTRACE(histo);

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

// Print all possible groups
void print_groups(struct group *groups) {
	ASSERT_BACKTRACE(groups);

	char f, n;

	printf("Groups:\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		index_to_char(groups[i].tile, &f, &n);
		switch (groups[i].type) {
			case PAIR:
				printf("Pair (%c%c, %c%c)\n", f, n, f, n);
				break;
			case SEQUENCE:
				printf("Sequence (%c%c, %c%c, %c%c)\n", f, n, f, n + 1, f,
				       n + 2);
				break;
			case TRIPLET:
				printf("Triplet (%c%c, %c%c, %c%c)\n", f, n, f, n, f, n);
				break;
			case QUAD:
				printf("Quad (%c%c, %c%c, %c%c, %c%c)\n", f, n, f, n, f, n, f,
				       n);
				break;
			default:
				fprintf(stderr, "print_groups: enum type not recognized: %d\n",
				        groups[i].type);
				break;
		}
	}
	printf("\n");
}

void print_victory(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

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

// Get the next input
// Overwrite action and return the corresponding tile index
histo_index_t get_input(struct histogram *histo, enum action *action) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(action);

	while (1) {
		printf("> ");
		fflush(stdout);

		char c;
		while ((c = getchar()) == ' ' || c == '\n')
				;
		if (c == 't') {
			// Tsumo action
			*action = ACTION_TSUMO;
			while (getchar() != '\n')
				;
			return NO_TILE_INDEX;
		}

		if (c == 'p') {
			// Pass or Pon

			while ((c = getchar()) == ' ' || c == '\n')
				;

			if (c == 'a') {
				// Pass
				*action = ACTION_PASS;
			} else {
				// Pon
				*action = ACTION_PON;
			}

			while (getchar() != '\n')
				;
			return NO_TILE_INDEX;
		}

		char family, number;
		if (c == 'd') {
			// Discard (explicit)

			// In this line, family is only use to pass unnecessary chars
			while ((family = getchar()) != ' ' && family != '\n')
				;

			*action = ACTION_DISCARD;
			while ((family = getchar()) == ' ' || family == '\n')
				;
		} else if (c == 'r') {
			// Riichi or Ron

			while ((c = getchar()) == ' ' || c == '\n')
				;

			if (c == 'i') {
				// Riichi
				*action = ACTION_RIICHI;
			} else if (c == 'o') {
				// Ron
				*action = ACTION_RON;
			} else {
				// Error
				while (getchar() != '\n')
					;
				continue;
			}

			// In this line, family is only use to pass unnecessary chars
			while ((family = getchar()) != ' ' && family != '\n')
				;

			while ((family = getchar()) == ' ' || family == '\n')
				;
		} else {
			// Discard (implicit)
			*action = ACTION_DISCARD;
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

		if (family != 'p' && family != 's' && family != 'm' && family != 'z')
			continue;

		if (number < '1' || number > '9')
			continue;

		histo_index_t index = char_to_index(family, number);

		if (!histo->cells[index])
			continue;

		return index;
	}
}
