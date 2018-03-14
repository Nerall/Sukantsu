#include "console_io.h"
#include "debug.h"
#include <stdio.h>

static inline int lower_case(char c) {
	return (c < 'A' || c >= 'a' ? c : c - 'A' + 'a');
}

// Convert a tile index to a family and a number characters
histo_index_t char_to_index(char family, char number) {
	ASSERT_BACKTRACE(number >= '1' && number <= '9');
	ASSERT_BACKTRACE(family != 'z' || number <= '7');

	histo_index_t index = 0;

	// Family to index
	if (family == 'p')
		index = 0;
	else if (family == 's')
		index = 9;
	else if (family == 'm')
		index = 18;
	else if (family == 
  
'z')
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
void print_histo(struct histogram *histo, histo_index_t last_tile) {
	ASSERT_BACKTRACE(histo);
  
	for (int i = 0; i < 33; ++i) {
		for (histo_cell_t j = histo->cells[i];
    (i == last_tile) ? j > 1 : j > 0; --j) {
			wprintf(L"%lc ", tileslist[i]);
		}
	}
	for (histo_cell_t j = histo->cells[33];
  (33 == last_tile) ? j > 1 : j > 0; --j) {
    wprintf(L"%lc", tileslist[33]);
  }
  if (last_tile != NO_TILE_INDEX)
    wprintf(L" %lc", tileslist[last_tile]);
  
  wprintf(L"\n");

  char PSMZ[] = { 0, 0, 0, 0 };
  for (int i = 0; i < 34; ++i) {
    for (histo_cell_t j = histo->cells[i];
    (i == last_tile) ? j > 1 : j > 0; --j) {
      wprintf(L"%d", 1 + i % 9);
    }
    if ((i == last_tile && histo->cells[i] > 1) ||
    (i != last_tile && histo->cells[i]))
      PSMZ[i / 9] = 1;
    if ((i % 9 == 8 || i == 33) && PSMZ[i / 9])
      wprintf(L"%lc ", L"psmz"[i / 9]);
  }
  
  if (last_tile != NO_TILE_INDEX)
    wprintf(L" %d%lc", 1 + last_tile % 9, L"psmz"[last_tile / 9]);
  
  wprintf(L"\n\n");
	
  /*wprintf(L"-----------------------------------\n");

	wprintf(L"| Index       |");
	for (int i = 1; i < 10; ++i)
		wprintf(L" %d", i);
	wprintf(L" |\n");

	wprintf(L"-----------------------------------\n");

	wprintf(L"| Coin    (p) |");
	for (int i = 0; i < 9; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" |\n");

	wprintf(L"| Bamboo  (s) |");
	for (int i = 9; i < 18; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" |\n");

	wprintf(L"| Numeral (m) |");
	for (int i = 18; i < 27; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L" |\n");

	wprintf(L"| Honor   (z) |");
	for (int i = 27; i < 34; ++i)
		wprintf(L" %d", histo->cells[i]);
	wprintf(L"     |\n");

	wprintf(L"-----------------------------------\n\n");*/
}

// Print all possible groups
void print_groups(struct group *groups) {
	ASSERT_BACKTRACE(groups);

	char f, n;

	wprintf(L"Groups:\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		index_to_char(groups[i].tile, &f, &n);
		switch (groups[i].type) {
			case PAIR:
				wprintf(L"Pair (%c%c%c)  %lc %lc\n", n, n, f,
				        tileslist[groups[i].tile], tileslist[groups[i].tile]);
				break;
			case SEQUENCE:
				wprintf(L"Sequence (%c%c%c%c)  %lc %lc %lc\n", n,
				        n + 1, n + 2, f, tileslist[groups[i].tile],
				        tileslist[groups[i].tile + 1],
				        tileslist[groups[i].tile + 2]);
				break;
			case TRIPLET:
				wprintf(L"Triplet (%c%c%c%c)  %lc %lc %lc\n", n, n,
				        n, f, tileslist[groups[i].tile],
				        tileslist[groups[i].tile], tileslist[groups[i].tile]);
				break;
			case QUAD:
				wprintf(L"Quad (%c%c%c%c%c)  %lc %lc %lc %lc\n", n,
				        n, n, n, f, tileslist[groups[i].tile],
				        tileslist[groups[i].tile], tileslist[groups[i].tile],
				        tileslist[groups[i].tile]);
				break;
			default:
				fprintf(stderr, "print_groups: enum type not recognized: %d\n",
				        groups[i].type);
				break;
		}
	}
	wprintf(L"\n");
}

void print_victory(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	struct histogram histo;
	groups_to_histo(hand, &histo);

	print_histo(&histo, hand->last_tile);
	if (iskokushi(hand))
		wprintf(L"WOW, Thirteen orphans!!\n\n");
	else {
		if (ischiitoi(hand))
			wprintf(L"WOW, Seven pairs!\n\n");
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
		wprintf(L"> ");
		fflush(stdout);

		char c, family, number;
		while ((c = getchar()) == ' ' || c == '\n')
			;

		c = lower_case(c);
		if (c == 'k') {
			// Kan action

			while ((family = getchar()) != ' ' && family != '\n')
				;

			*action = ACTION_KAN;

			// Get family
			while ((family = getchar()) == ' ' || family == '\n')
				;
		}
    
    else if (c == 't') {
			// Tsumo action

			while (getchar() != '\n')
				;

			*action = ACTION_TSUMO;
			return NO_TILE_INDEX;
		}
    
    else if (c == 'd') {
			// Discard (explicit)

			// In this line, family is only use to pass unnecessary chars
			while ((family = getchar()) != ' ' && family != '\n')
				;

			*action = ACTION_DISCARD;
			// Get family
			while ((family = getchar()) == ' ' || family == '\n')
				;
		}
    
    else if (c == 'r') {
			// Riichi or Ron

			while ((c = getchar()) != ' ' && c != '\n')
				;

			*action = ACTION_RIICHI;
			// Get family
			while ((family = getchar()) == ' ' || family == '\n')
				;
		}
    
    else {
			// Discard (implicit)
			*action = ACTION_DISCARD;
			family = c;
		}

		// Get number
		while ((number = getchar()) == ' ' || number == '\n')
			;

		while (getchar() != '\n')
			;

		// Tile selection
		if (family >= '1' && family <= '9') {
			char tmp = family;
			family = number;
			number = tmp;
		} else if (number < '1' || number > '9') {
			continue;
		}

		family = lower_case(family);

		if (family != 'p' && family != 's' && family != 'm' && family != 'z')
			continue;

		histo_index_t index = char_to_index(family, number);

		if (!histo->cells[index])
			continue;

		return index;
	}
}
