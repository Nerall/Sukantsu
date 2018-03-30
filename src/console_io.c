#include "console_io.h"
#include "AI/detect.h"
#include "core/histogram.h"
#include "core/riichi_engine_s.h"
#include "debug.h"
#include <stdio.h>
#include <wchar.h>

static wchar_t tileslist[] = L"🀙🀚🀛🀜🀝🀞🀟🀠🀡🀐🀑🀒🀓🀔🀕🀖🀗🀘🀇🀈🀉🀊🀋🀌🀍🀎🀏🀀🀁🀂🀃🀆🀅🀄";

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
void print_histo(struct histogram *histo, histo_index_t last_tile) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(last_tile == NO_TILE_INDEX ||
	                 (is_valid_index(last_tile) && histo->cells[last_tile]));

	// "Remove" the last_tile to print at the side
	if (last_tile != NO_TILE_INDEX)
		histo->cells[last_tile]--;

	for (int i = 0; i < 33; ++i) {
		for (histo_cell_t j = histo->cells[i]; j > 0; --j) {
			wprintf(L"%lc ", tileslist[i]);
		}
	}
	for (histo_cell_t j = histo->cells[33]; j > 0; --j) {
		wprintf(L"%lc", tileslist[33]);
	}

	if (last_tile != NO_TILE_INDEX)
		wprintf(L" %lc", tileslist[last_tile]);

	wprintf(L"\n");

	char PSMZ[] = {0, 0, 0, 0};
	for (int i = 0; i < 34; ++i) {
		for (histo_cell_t j = histo->cells[i]; j > 0; --j) {
			wprintf(L"%d", 1 + i % 9);
		}

		if (histo->cells[i])
			PSMZ[i / 9] = 1;

		if ((i == 33 || i % 9 == 8) && PSMZ[i / 9])
			wprintf(L"%lc ", L"psmz"[i / 9]);
	}

	if (last_tile != NO_TILE_INDEX) {
		wprintf(L" %d%lc", 1 + last_tile % 9, L"psmz"[last_tile / 9]);
		// "Re-Add" the last_tile
		histo->cells[last_tile]++;
	}

	wprintf(L"\n\n");
}

// Print all possible groups
void print_groups(struct group *groups) {
	ASSERT_BACKTRACE(groups);

	char f, n;

	wprintf(L"Groups:\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		index_to_char(groups[i].tile, &f, &n);
		wchar_t c_tile = tileslist[groups[i].tile];
		switch (groups[i].type) {
			case PAIR:
				wprintf(L"Pair (%c%c%c)  %lc %lc\n", n, n, f, c_tile, c_tile);
				break;
			case SEQUENCE:
				wprintf(L"Sequence (%c%c%c%c)  %lc %lc %lc\n", n, n + 1, n + 2,
				        f, c_tile, tileslist[groups[i].tile + 1],
				        tileslist[groups[i].tile + 2]);
				break;
			case TRIPLET:
				wprintf(L"Triplet (%c%c%c%c)  %lc %lc %lc\n", n, n, n, f,
				        c_tile, c_tile, c_tile);
				break;
			case QUAD:
				wprintf(L"Quad (%c%c%c%c%c)  %lc %lc %lc %lc\n", n, n, n, n, f,
				        c_tile, c_tile, c_tile, c_tile);
				break;
			default:
				fprintf(stderr, "print_groups: enum type not recognized: %d\n",
				        groups[i].type);
				break;
		}
	}
	wprintf(L"\n");
}

// TODO: histo[last_tile] == 0 sometimes
void print_victory(struct hand *hand, struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	struct histogram histo;
	groups_to_histo(hand, &histo);

	print_histo(&histo, hand->last_tile);
	if (iskokushi(hand))
		wprintf(L"WOW, Thirteen orphans!!\n");
	else {
		if (ischiitoi(hand))
			wprintf(L"WOW, Seven pairs!\n");
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

// Display informations based on the structure
// Current game phase can be obtained via the enum engine->phase
// Current player should not be needed in the future (but still useful now)
void display_riichi(struct riichi_engine *engine, int current_player) {
	ASSERT_BACKTRACE(engine);

	struct player *player = &engine->players[current_player];
	struct hand *player_hand = &player->hand;

	char *pos[] = {"NORTH", "EAST", "SOUTH", "WEST"};

	switch (engine->phase) {
		case PHASE_INIT: {
			wprintf(L"\n# Game %u:\n", engine->nb_games);
			break;
		}

		case PHASE_DRAW: {
			wprintf(L"--------------------------------\n\n");
			wprintf(L"Remaining tiles: %u\n\n", (engine->wall.nb_tiles - 14));

			print_histo(&player_hand->histo, player_hand->last_tile);

			// Show best discards (hints)
			if (player_hand->tenpai) {
				wprintf(L"You are tenpai if you discard:\n");
				for (int r = 0; r < 34; ++r) {
					if (get_histobit(&player_hand->riichitiles, r)) {
						char f, n;
						index_to_char(r, &f, &n);
						wprintf(L"%c%c %lc\n", n, f, tileslist[r]);
					}
				}
				wprintf(L"\n");
			}
			break;
		}

		case PHASE_GETINPUT: {
			char f, n;
			index_to_char(player_hand->last_discard, &f, &n);
			wprintf(L"Player %-5s has discarded: %c%c\n",
			        pos[player->player_pos], n, f);
			break;
		}

		case PHASE_TSUMO: {
			wprintf(L"TSUMO!\n");
			print_victory(player_hand, &engine->grouplist);
			break;
		}

		case PHASE_WAIT: {
			if (player_hand->tenpai) {
				wprintf(L"You win if you get:\n");
				for (int w = 0; w < 34; ++w) {
					if (get_histobit(&player_hand->wintiles, w)) {
						char f, n;
						index_to_char(w, &f, &n);
						wprintf(L"%c%c %lc\n", n, f, tileslist[w]);
					}
				}
				wprintf(L"\n");
			}
			break;
		}

		default:
			ASSERT_BACKTRACE(0 && "Phase not recognized");
	}
}
