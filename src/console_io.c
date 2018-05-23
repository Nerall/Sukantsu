#include "console_io.h"
#include "AI/detect.h"
#include "core/hand.h"
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
void print_histo(const struct histogram *histo, histo_index_t last_tile) {
	ASSERT_BACKTRACE(histo);
	ASSERT_BACKTRACE(last_tile == NO_TILE_INDEX ||
	                 (is_valid_index(last_tile) && histo->cells[last_tile]));

	for (int i = 0; i < 33; ++i) {
		for (histo_cell_t j = histo->cells[i] - (last_tile == i); j > 0; --j) {
			wprintf(L"%lc ", tileslist[i]);
		}
	}
	for (histo_cell_t j = histo->cells[33] - (last_tile == 33); j > 0; --j) {
		wprintf(L"%lc", tileslist[33]);
	}

	if (last_tile != NO_TILE_INDEX)
		wprintf(L" %lc", tileslist[last_tile]);

	wprintf(L"\n");

	char PSMZ[] = {0, 0, 0, 0};
	for (int i = 0; i < 34; ++i) {
		for (histo_cell_t j = histo->cells[i] - (last_tile == i); j > 0; --j) {
			wprintf(L"%d", 1 + i % 9);
		}

		if (histo->cells[i] - (last_tile == i))
			PSMZ[i / 9] = 1;

		if ((i == 33 || i % 9 == 8) && PSMZ[i / 9])
			wprintf(L"%lc ", L"psmz"[i / 9]);
	}

	if (last_tile != NO_TILE_INDEX) {
		wprintf(L" %d%lc", 1 + last_tile % 9, L"psmz"[last_tile / 9]);
	}

	wprintf(L"\n\n");
}

// Print all possible groups
void print_groups(const struct group *groups) {
	ASSERT_BACKTRACE(groups);

	char f, n;

	wprintf(L"Groups:\n");
	for (int i = 0; i < HAND_NB_GROUPS; ++i) {
		index_to_char(groups[i].tile, &f, &n);
		wchar_t c_tile = tileslist[groups[i].tile];
		switch (groups[i].type) {
			case PAIR:
				wprintf(L"Pair %u (%c%c%c)  %lc %lc\n", groups[i].hidden, n, n, f, c_tile, c_tile);
				break;
			case SEQUENCE:
				wprintf(L"Sequence %u (%c%c%c%c)  %lc %lc %lc\n", groups[i].hidden, n, n + 1, n + 2,
				        f, c_tile, tileslist[groups[i].tile + 1],
				        tileslist[groups[i].tile + 2]);
				break;
			case TRIPLET:
				wprintf(L"Triplet %u (%c%c%c%c)  %lc %lc %lc\n", groups[i].hidden, n, n, n, f,
				        c_tile, c_tile, c_tile);
				break;
			case QUAD:
				wprintf(L"Quad %u (%c%c%c%c%c)  %lc %lc %lc %lc\n", groups[i].hidden, n, n, n, n, f,
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

void print_victory(const struct hand *hand, const struct grouplist *grouplist) {
	ASSERT_BACKTRACE(grouplist);

	struct histogram histo;
	groups_to_histo(hand, &histo);

	print_histo(&histo, hand->last_tile);
	if (iskokushi(hand))
		wprintf(L"WOW, Thirteen orphans!!\n");
	else {
		if (ischiitoi(hand))
			wprintf(L"WOW, Seven pairs!\n");
		for (int i = 0; i < grouplist->nb_groups; ++i) {
			print_groups(grouplist->groups[i]);
		}
	}
}

// Get the next input
// Overwrite action and return the corresponding tile index
histo_index_t get_input(const struct histogram *histo, enum action *action) {
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
void display_riichi(const struct riichi_engine *engine, int current_player) {
	ASSERT_BACKTRACE(engine);

	const struct player *player = &engine->players[current_player];
	const struct hand *player_hand = &player->hand;

	char *pos[] = {"EAST", "SOUTH", "WEST", "NORTH"};

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
			wprintf(L"Player %s has discarded: %c%c\n",
			        pos[player->player_pos], n, f);
			break;
		}

		case PHASE_TSUMO: {
			// engine->grouplist must contain the victory grouplist
			if (player_hand->last_tile != NO_TILE_INDEX)
				wprintf(L"TSUMO!\n");
			else
				wprintf(L"RON!\n");
			print_victory(player_hand, &engine->grouplist);
			break;
		}

		case PHASE_CLAIM: {
			// wprintf(L"Do you want to claim? (todo)\n");
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

void init_tilesGUI(struct tilesGUI *tilesGUI, enum typeGUI typeGUI,
                   int current_player) {
	ASSERT_BACKTRACE(tilesGUI);
	tilesGUI->rotation = 360 - 90 * (current_player);
	tilesGUI->typeGUI = typeGUI;
	sfVector2f scale;
	sfVector2f tileposition;
	sfVector2f tileincrement;
	sfVector2f bordersize;
	switch (typeGUI) {
		case PLAYERHAND:
			switch (current_player) {
				case 0:	
					scale.x = 0.15;
					scale.y = 0.15;
					bordersize.x = 34;
					bordersize.y = 46;
					tileposition.x = 150;
					tileposition.y = 525;
					tileincrement.x = 35;
					tileincrement.y = 0;
					break;
				 case 1:
					scale.x = 0.15;
					scale.y = 0.15;
					bordersize.x = 34;
					bordersize.y = 46;
				 	tileposition.x = 725;
					tileposition.y = 550;
					tileincrement.x = 0;
					tileincrement.y = -35;
					 break;
				 case 2:
					scale.x = 0.15;
					scale.y = 0.15;
					bordersize.x = 34;
					bordersize.y = 46;
				 	tileposition.x = 650;
					tileposition.y = 75;
					tileincrement.x = -35;
					tileincrement.y = 0;
					 break;
				 case 3:
					scale.x = 0.15;
					scale.y = 0.15;
					bordersize.x = 34;
					bordersize.y = 46;
				 	tileposition.x = 75;
					tileposition.y = 50;
					tileincrement.x = 0;
					tileincrement.y = 35;
					 break;
				default:
					scale.x = 0;
					scale.y = 0;
					tileposition.x = 0;
					tileposition.y = 0;
					tileincrement.x = 0;
					tileincrement.y = 0;
					break;
			}
			break;
		case PLAYERHANDOPEN:
			scale.x = 0.11;
			scale.y = 0.11;
			bordersize.x = 25;
			bordersize.y = 34;
			switch (current_player) { /*
				 case 0:
				     break;
				 case 1:
				     break;
				 case 2:
				     break;
				 case 3:
				     break;*/
				default:
					tileposition.x = 0;
					tileposition.y = 0;
					tileincrement.x = 0;
					tileincrement.y = 0;
					break;
			}
			break;
		case PLAYERDISCARDS:
			scale.x = 0.11;
			scale.y = 0.11;
			bordersize.x = 25;
			bordersize.y = 34;
			switch (current_player) {
				case 0:
					tileposition.x = 322;
					tileposition.y = 381;
					tileincrement.x = 26;
					tileincrement.y = 35;
					break;
				case 1:
					tileposition.x = 531;
					tileposition.y = 378;
					tileincrement.x = 35;
					tileincrement.y = -26;
					break;
				case 2:
					tileposition.x = 478;
					tileposition.y = 219;
					tileincrement.x = -26;
					tileincrement.y = -35;
					break;
				case 3:
					tileposition.x = 269;
					tileposition.y = 222;
					tileincrement.x = -35;
					tileincrement.y = 26;
					break;
				default:
					tileposition.x = 0;
					tileposition.y = 0;
					tileincrement.x = 0;
					tileincrement.y = 0;
					break;
			}
			break;
		case DORAS:
			scale.x = 0.11;
			bordersize.x = 25;
			bordersize.y = 34;
			tileposition.x = 335;
			tileposition.y = 265;
			tileincrement.x = 26;
			tileincrement.y = 35;
			break;
		default:
			scale.x = 1;
			scale.y = 1;
			bordersize.x = 0;
			bordersize.y = 0;
			tileposition.x = 0;
			tileposition.y = 0;
			break;
	}
	tilesGUI->scale = scale;
	sfSprite *tilesprite[24];
	for (int i = 0; i < 24; ++i) {
		tilesGUI->tilesprite[i] = tilesprite[i];
	}
	tilesGUI->tileposition = tileposition;
	tilesGUI->tileincrement = tileincrement;
	tilesGUI->border = 0;
	tilesGUI->bordersize = bordersize;
}

void init_gameGUI(struct gameGUI *gameGUI) {
	ASSERT_BACKTRACE(gameGUI);

	gameGUI->window = NULL;
	sfVideoMode mode = {800, 600, 32};
	gameGUI->mode = mode;
	sfColor background;
	background = sfColor_fromRGB(0, 128, 255);
	gameGUI->background = background;

	init_tilesGUI(&gameGUI->player1hand, PLAYERHAND, 0);
	init_tilesGUI(&gameGUI->player2hand, PLAYERHAND, 1);
	init_tilesGUI(&gameGUI->player3hand, PLAYERHAND, 2);
	init_tilesGUI(&gameGUI->player4hand, PLAYERHAND, 3);
	init_tilesGUI(&gameGUI->player1handopen, PLAYERHANDOPEN, 0);
	init_tilesGUI(&gameGUI->player2handopen, PLAYERHANDOPEN, 1);
	init_tilesGUI(&gameGUI->player3handopen, PLAYERHANDOPEN, 2);
	init_tilesGUI(&gameGUI->player4handopen, PLAYERHANDOPEN, 3);
	init_tilesGUI(&gameGUI->player1discards, PLAYERDISCARDS, 0);
	init_tilesGUI(&gameGUI->player2discards, PLAYERDISCARDS, 1);
	init_tilesGUI(&gameGUI->player3discards, PLAYERDISCARDS, 2);
	init_tilesGUI(&gameGUI->player4discards, PLAYERDISCARDS, 3);
	init_tilesGUI(&gameGUI->doras, DORAS, 0);

	gameGUI->center = NULL;
	sfVector2f centerposition;
	centerposition.x = 270;
	centerposition.y = 220;
	gameGUI->centerposition = centerposition;
	gameGUI->centercolor = sfGreen;
	sfVector2f centersize;
	centersize.x = 260;
	centersize.y = 160;
	gameGUI->centersize = centersize;

	gameGUI->suppx = NULL;
	gameGUI->suppy = NULL;

	// Moved from display_GUI
	for (int i = 0; i < 35; ++i) {
		char path[20];
		sprintf(path, "src/tiles/%d.png", i);
		gameGUI->textureslist[i] = sfTexture_createFromFile(path, NULL);
	}
	gameGUI->center = sfRectangleShape_create();

	gameGUI->player1hand.border = sfRectangleShape_create();
	gameGUI->player2hand.border = sfRectangleShape_create();
	gameGUI->player3hand.border = sfRectangleShape_create();
	gameGUI->player4hand.border = sfRectangleShape_create();

	for (int i = 0; i < 14; ++i) {
		gameGUI->player1hand.tilesprite[i] = sfSprite_create();
		gameGUI->player2hand.tilesprite[i] = sfSprite_create();
		gameGUI->player3hand.tilesprite[i] = sfSprite_create();
		gameGUI->player4hand.tilesprite[i] = sfSprite_create();
	}

	gameGUI->player1discards.border = sfRectangleShape_create();
	gameGUI->player2discards.border = sfRectangleShape_create();
	gameGUI->player3discards.border = sfRectangleShape_create();
	gameGUI->player4discards.border = sfRectangleShape_create();

	for (int i = 0; i < 24; ++i) {
		gameGUI->player1discards.tilesprite[i] = sfSprite_create();
		gameGUI->player2discards.tilesprite[i] = sfSprite_create();
		gameGUI->player3discards.tilesprite[i] = sfSprite_create();
		gameGUI->player4discards.tilesprite[i] = sfSprite_create();
	}

	gameGUI->doras.border = sfRectangleShape_create();

	for (int i = 0; i < 10; ++i) {
		gameGUI->doras.tilesprite[i] = sfSprite_create();
	}

	gameGUI->suppx = sfRectangleShape_create();
	gameGUI->suppy = sfRectangleShape_create();
}

void destroy_gameGUI(struct gameGUI *gameGUI) {
	ASSERT_BACKTRACE(gameGUI);

	for (int i = 0; i < 35; ++i) {
		sfTexture_destroy(gameGUI->textureslist[i]);
	}

	sfRectangleShape_destroy(gameGUI->player1hand.border);
	sfRectangleShape_destroy(gameGUI->player2hand.border);
	sfRectangleShape_destroy(gameGUI->player3hand.border);
	sfRectangleShape_destroy(gameGUI->player4hand.border);

	sfRectangleShape_destroy(gameGUI->center);
	for (int i = 0; i < 14; ++i) {
		sfSprite_destroy(gameGUI->player1hand.tilesprite[i]);
		sfSprite_destroy(gameGUI->player2hand.tilesprite[i]);
		sfSprite_destroy(gameGUI->player3hand.tilesprite[i]);
		sfSprite_destroy(gameGUI->player4hand.tilesprite[i]);

	}
	sfRectangleShape_destroy(gameGUI->player1discards.border);
	sfRectangleShape_destroy(gameGUI->player2discards.border);
	sfRectangleShape_destroy(gameGUI->player3discards.border);
	sfRectangleShape_destroy(gameGUI->player4discards.border);

	for (int i = 0; i < 24; ++i) {
		sfSprite_destroy(gameGUI->player1discards.tilesprite[i]);
		sfSprite_destroy(gameGUI->player2discards.tilesprite[i]);
		sfSprite_destroy(gameGUI->player3discards.tilesprite[i]);
		sfSprite_destroy(gameGUI->player4discards.tilesprite[i]);
	}

	sfRectangleShape_destroy(gameGUI->doras.border);

	for (int i = 0; i < 10; ++i) {
		sfSprite_destroy(gameGUI->doras.tilesprite[i]);
	}

	sfRectangleShape_destroy(gameGUI->suppx);
	sfRectangleShape_destroy(gameGUI->suppy);
}

void display_GUI(struct riichi_engine *engine) {
	struct gameGUI *gameGUI = &engine->gameGUI;
	sfRenderWindow_clear(gameGUI->window, gameGUI->background);

	/////////////////////////////////////////////////////////////////
	// Notice from Nico:                                           //
	// All that have been commented has been moved to init_gameGUI //
	// Creating a lot of things each time without destroying them  //
	//   is not so great for my computer's memory ;)               //
	/////////////////////////////////////////////////////////////////

	sfRectangleShape_setOutlineColor(gameGUI->center, sfBlack);
	sfRectangleShape_setOutlineThickness(gameGUI->center, 1.0);
	sfRectangleShape_setPosition(gameGUI->center, gameGUI->centerposition);
	sfRectangleShape_setSize(gameGUI->center, gameGUI->centersize);
	sfRectangleShape_setFillColor(gameGUI->center, gameGUI->centercolor);
	sfRenderWindow_drawRectangleShape(gameGUI->window, gameGUI->center,
	                                  NULL);

	sfVector2f position;

	struct tilesGUI *doras = &gameGUI->doras;
	for (int i = 0; i < 10; ++i) {
		position.x =
		    doras->tileposition.x + doras->tileincrement.x * (i % 5);
		position.y =
			doras->tileposition.y + doras->tileincrement.y * (i / 5);
		sfRectangleShape_setFillColor(doras->border, sfTransparent);
		sfRectangleShape_setOutlineColor(doras->border, sfBlack);
		sfRectangleShape_setOutlineThickness(doras->border, 1.0);
		sfRectangleShape_setPosition(doras->border, position);
		sfRectangleShape_setSize(doras->border, doras->bordersize);
		sfRenderWindow_drawRectangleShape(gameGUI->window, doras->border, NULL);
		if (i < engine->doralist.nb_revealed) {
			sfSprite_setTexture(doras->tilesprite[i],
			 gameGUI->textureslist[engine->doralist.tiles[i]], 1);
		}
		else
			sfSprite_setTexture(doras->tilesprite[i],
								 gameGUI->textureslist[34], 1);
		sfSprite_setPosition(doras->tilesprite[i], position);
		sfSprite_setScale(doras->tilesprite[i], doras->scale);
		sfRenderWindow_drawSprite(gameGUI->window, doras->tilesprite[i], NULL);
	}

	struct hand handcopy;

	copy_hand(&engine->players[0]
			  .hand, &handcopy);
	struct tilesGUI *P1 = &gameGUI->player1hand;
	struct tilesGUI *D1 = &gameGUI->player1discards;

	int n = 13;
	for (int i = 0; i < 14; ++i) {
		position.x =
		    P1->tileposition.x + P1->tileincrement.x * (i) + (i == 13) * 9;
		position.y = P1->tileposition.y;
		sfRectangleShape_setFillColor(P1->border, sfTransparent);
		sfRectangleShape_setOutlineColor(P1->border, sfBlack);
		sfRectangleShape_setOutlineThickness(P1->border, 1.0);
		sfRectangleShape_setPosition(P1->border, position);
		sfRectangleShape_setSize(P1->border, P1->bordersize);
		sfRenderWindow_drawRectangleShape(gameGUI->window, P1->border, NULL);
		if (i < n) {
			for (histo_cell_t j = 0; j < 34; ++j) {
				if (handcopy.histo.cells[j] - (handcopy.last_tile == j) > 0) {
					--handcopy.histo.cells[j];
					sfSprite_setTexture(P1->tilesprite[i],
										gameGUI->textureslist[j], 1);
					break;
				}
				if (j == 33) {
					sfSprite_setTexture(P1->tilesprite[i],
										gameGUI->textureslist[34], 1);
					if (n == 13)
						n = i;
				}
			}
		}
		else if (i == 13 && handcopy.last_tile != NO_TILE_INDEX) {
			--handcopy.histo.cells[handcopy.last_tile];
			sfSprite_setTexture(P1->tilesprite[i],
			                    gameGUI->textureslist[handcopy.last_tile], 1);
		}
		else {
			sfSprite_setTexture(P1->tilesprite[i],
			                    gameGUI->textureslist[34], 1);
		}
		sfSprite_setPosition(P1->tilesprite[i], position);
		sfSprite_setScale(P1->tilesprite[i], P1->scale);
		sfRenderWindow_drawSprite(gameGUI->window, P1->tilesprite[i], NULL);
	}

	for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
		position.x = D1->tileposition.x + D1->tileincrement.x * (i % 6)
			+ D1->tileincrement.x * (i > 17) * 6;
		position.y = D1->tileposition.y + D1->tileincrement.y * (i / 6)
			- D1->tileincrement.y * (i > 17);
		sfRectangleShape_setFillColor(D1->border, sfTransparent);
		sfRectangleShape_setOutlineColor(D1->border, sfBlack);
		sfRectangleShape_setOutlineThickness(D1->border, 1.0);
		sfRectangleShape_setPosition(D1->border, position);
		sfRectangleShape_setSize(D1->border, D1->bordersize);
		sfRenderWindow_drawRectangleShape(gameGUI->window, D1->border, NULL);

		sfSprite_setTexture(D1->tilesprite[i], gameGUI->textureslist[
			handcopy.discardlist.discards[i]],1);
		sfSprite_setPosition(D1->tilesprite[i], position);
		sfSprite_setScale(D1->tilesprite[i], D1->scale);
		sfRenderWindow_drawSprite(gameGUI->window, D1->tilesprite[i], NULL);

	}

	// Player 2
	copy_hand(&engine->players[1]
			   .hand, &handcopy);
	struct tilesGUI *P2 = &gameGUI->player2hand;
	struct tilesGUI *D2 = &gameGUI->player2discards;
	
	n = 13;
	for (int i = 0; i < 14; ++i) {
		position.x = P2->tileposition.x;
		position.y =
			P2->tileposition.y + P2->tileincrement.y * (i) - (i == 13) * 9;
		sfRectangleShape_setFillColor(P2->border, sfTransparent);
		sfRectangleShape_setOutlineColor(P2->border, sfBlack);
		sfRectangleShape_setOutlineThickness(P2->border, 1.0);
		sfRectangleShape_setPosition(P2->border, position);
		sfRectangleShape_setSize(P2->border, P2->bordersize);
		sfRectangleShape_setRotation(P2->border, P2->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, P2->border, NULL);
		if (i < n) {
			for (histo_cell_t j = 0; j < 34; ++j) {
				if (handcopy.histo.cells[j] - (handcopy.last_tile == j) > 0) {
					--handcopy.histo.cells[j];
					sfSprite_setTexture(P2->tilesprite[i],
										gameGUI->textureslist[j], 1);
					break;
				}
				if (j == 33) {
					sfSprite_setTexture(P2->tilesprite[i],
										gameGUI->textureslist[34], 1);
					if (n == 13)
						n = i;
				}
			}
		}
		else if (i == 13 && handcopy.last_tile != NO_TILE_INDEX) {
			--handcopy.histo.cells[handcopy.last_tile];
			sfSprite_setTexture(P2->tilesprite[i],
			                    gameGUI->textureslist[handcopy.last_tile], 1);
		}
		else {
			sfSprite_setTexture(P2->tilesprite[i],
			                    gameGUI->textureslist[34], 1);
		}
		sfSprite_setPosition(P2->tilesprite[i], position);
		sfSprite_setScale(P2->tilesprite[i], P2->scale);
		sfSprite_setRotation(P2->tilesprite[i], P2->rotation);

		sfRenderWindow_drawSprite(gameGUI->window, P2->tilesprite[i], NULL);
	}

	for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
		position.x = D2->tileposition.x + D2->tileincrement.x * (i / 6)
			- D2->tileincrement.x * (i > 17);
		position.y = D2->tileposition.y + D2->tileincrement.y * (i % 6)
			+ D2->tileincrement.y * (i > 17) * 6;
		sfRectangleShape_setFillColor(D2->border, sfTransparent);
		sfRectangleShape_setOutlineColor(D2->border, sfBlack);
		sfRectangleShape_setOutlineThickness(D2->border, 1.0);
		sfRectangleShape_setPosition(D2->border, position);
		sfRectangleShape_setSize(D2->border, D2->bordersize);
		sfRectangleShape_setRotation(D2->border, D2->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, D2->border, NULL);

		sfSprite_setTexture(D2->tilesprite[i], gameGUI->textureslist[
			handcopy.discardlist.discards[i]],1);
		sfSprite_setPosition(D2->tilesprite[i], position);
		sfSprite_setScale(D2->tilesprite[i], D2->scale);
		sfSprite_setRotation(D2->tilesprite[i], D2->rotation);
		sfRenderWindow_drawSprite(gameGUI->window, D2->tilesprite[i], NULL);

	}

	// Player 3
	copy_hand(&engine->players[2]
			   .hand, &handcopy);
	struct tilesGUI *P3 = &gameGUI->player3hand;
	struct tilesGUI *D3 = &gameGUI->player3discards;
	
	n = 13;
	for (int i = 0; i < 14; ++i) {
		position.x =
		    P3->tileposition.x + P3->tileincrement.x * (i) - (i == 13) * 9;
		position.y = P3->tileposition.y;
		sfRectangleShape_setFillColor(P3->border, sfTransparent);
		sfRectangleShape_setOutlineColor(P3->border, sfBlack);
		sfRectangleShape_setOutlineThickness(P3->border, 1.0);
		sfRectangleShape_setPosition(P3->border, position);
		sfRectangleShape_setSize(P3->border, P3->bordersize);
		sfRectangleShape_setRotation(P3->border, P3->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, P3->border, NULL);
		if (i < n) {
			for (histo_cell_t j = 0; j < 34; ++j) {
				if (handcopy.histo.cells[j] - (handcopy.last_tile == j) > 0) {
					--handcopy.histo.cells[j];
					sfSprite_setTexture(P3->tilesprite[i],
										gameGUI->textureslist[j], 1);
					break;
				}
				if (j == 33) {
					sfSprite_setTexture(P3->tilesprite[i],
										gameGUI->textureslist[34], 1);
					if (n == 13)
						n = i;
				}
			}
		}
		else if (i == 13 && handcopy.last_tile != NO_TILE_INDEX) {
			--handcopy.histo.cells[handcopy.last_tile];
			sfSprite_setTexture(P3->tilesprite[i],
			                    gameGUI->textureslist[handcopy.last_tile], 1);
		}
		else {
			sfSprite_setTexture(P3->tilesprite[i],
			                    gameGUI->textureslist[34], 1);
		}
		sfSprite_setPosition(P3->tilesprite[i], position);
		sfSprite_setScale(P3->tilesprite[i], P3->scale);
		sfSprite_setRotation(P3->tilesprite[i], P3->rotation);

		sfRenderWindow_drawSprite(gameGUI->window, P3->tilesprite[i], NULL);
	}

	for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
		position.x = D3->tileposition.x + D3->tileincrement.x * (i % 6)
			+ D3->tileincrement.x * (i > 17) * 6;
		position.y = D3->tileposition.y + D3->tileincrement.y * (i / 6)
			- D3->tileincrement.y * (i > 17);
		sfRectangleShape_setFillColor(D3->border, sfTransparent);
		sfRectangleShape_setOutlineColor(D3->border, sfBlack);
		sfRectangleShape_setOutlineThickness(D3->border, 1.0);
		sfRectangleShape_setPosition(D3->border, position);
		sfRectangleShape_setSize(D3->border, D3->bordersize);
		sfRectangleShape_setRotation(D3->border, D3->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, D3->border, NULL);

		sfSprite_setTexture(D3->tilesprite[i], gameGUI->textureslist[
			handcopy.discardlist.discards[i]],1);
		sfSprite_setPosition(D3->tilesprite[i], position);
		sfSprite_setScale(D3->tilesprite[i], D3->scale);
		sfSprite_setRotation(D3->tilesprite[i], D3->rotation);
		sfRenderWindow_drawSprite(gameGUI->window, D3->tilesprite[i], NULL);

	}

	// Player 4
	copy_hand(&engine->players[3]
			   .hand, &handcopy);
	struct tilesGUI *P4 = &gameGUI->player4hand;
	struct tilesGUI *D4 = &gameGUI->player4discards;
	
	n = 13;
	for (int i = 0; i < 14; ++i) {
		position.x = P4->tileposition.x;
		position.y =
			P4->tileposition.y + P4->tileincrement.y * (i) + (i == 13) * 9;
		sfRectangleShape_setFillColor(P4->border, sfTransparent);
		sfRectangleShape_setOutlineColor(P4->border, sfBlack);
		sfRectangleShape_setOutlineThickness(P4->border, 1.0);
		sfRectangleShape_setPosition(P4->border, position);
		sfRectangleShape_setSize(P4->border, P4->bordersize);
		sfRectangleShape_setRotation(P4->border, P4->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, P4->border, NULL);
		if (i < n) {
			for (histo_cell_t j = 0; j < 34; ++j) {
				if (handcopy.histo.cells[j] - (handcopy.last_tile == j) > 0) {
					--handcopy.histo.cells[j];
					sfSprite_setTexture(P4->tilesprite[i],
										gameGUI->textureslist[j], 1);
					break;
				}
				if (j == 33) {
					sfSprite_setTexture(P4->tilesprite[i],
										gameGUI->textureslist[34], 1);
					if (n == 13)
						n = i;
				}
			}
		}
		else if (i == 13 && handcopy.last_tile != NO_TILE_INDEX) {
			--handcopy.histo.cells[handcopy.last_tile];
			sfSprite_setTexture(P4->tilesprite[i],
			                    gameGUI->textureslist[handcopy.last_tile], 1);
		}
		else {
			sfSprite_setTexture(P4->tilesprite[i],
			                    gameGUI->textureslist[34], 1);
		}
		sfSprite_setPosition(P4->tilesprite[i], position);
		sfSprite_setScale(P4->tilesprite[i], P4->scale);
		sfSprite_setRotation(P4->tilesprite[i], P4->rotation);

		sfRenderWindow_drawSprite(gameGUI->window, P4->tilesprite[i], NULL);
	}

	for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
		position.x = D4->tileposition.x + D4->tileincrement.x * (i / 6)
			- D4->tileincrement.x * (i > 17);
		position.y = D4->tileposition.y + D4->tileincrement.y * (i % 6)
			+ D4->tileincrement.y * (i > 17) * 6;
		sfRectangleShape_setFillColor(D4->border, sfTransparent);
		sfRectangleShape_setOutlineColor(D4->border, sfBlack);
		sfRectangleShape_setOutlineThickness(D4->border, 1.0);
		sfRectangleShape_setPosition(D4->border, position);
		sfRectangleShape_setSize(D4->border, D4->bordersize);
		sfRectangleShape_setRotation(D4->border, D4->rotation);
		sfRenderWindow_drawRectangleShape(gameGUI->window, D4->border, NULL);

		sfSprite_setTexture(D4->tilesprite[i], gameGUI->textureslist[
			handcopy.discardlist.discards[i]],1);
		sfSprite_setPosition(D4->tilesprite[i], position);
		sfSprite_setScale(D4->tilesprite[i], D4->scale);
		sfSprite_setRotation(D4->tilesprite[i], D4->rotation);
		sfRenderWindow_drawSprite(gameGUI->window, D4->tilesprite[i], NULL);

	}
	sfRenderWindow_display(gameGUI->window);
}

void get_player_click(struct riichi_engine *engine, struct action_input *input) {
	sfRenderWindow* window = engine->gameGUI.window;
    sfEvent event;
    while(sfRenderWindow_isOpen(window)) {
		while (sfRenderWindow_pollEvent(window, &event))
	    {
	        switch (event.type) {
				case sfEvtClosed:
					return;
		       	case sfEvtResized:
	       			/*if (event.size.width > 800) {
	       				sfRectangleShape *suppx = engine->gameGUI.suppx;
	       				sfVector2f size;
	        			size.x = event.size.width - 800;
	        			size.y = event.size.height;
	        			sfVector2f position;
	        			position.x = 800;
	        			position.y = 0;
	        			sfRectangleShape_setFillColor(suppx, sfBlack);
	        			sfRectangleShape_setSize(suppx, size);
	        			sfRectangleShape_setPosition(suppx, position);
	        			sfRenderWindow_drawRectangleShape(window, suppx, NULL);
	          			}
	        		if (event.size.height > 600) {
	        			sfRectangleShape *suppy = engine->gameGUI.suppy;
	        			sfVector2f size;
	        			size.x = event.size.width;
	        			size.y = event.size.height - 600;
	        			sfVector2f position;
	        			position.x = 0;
	        			position.y = 600;
	        			sfRectangleShape_setFillColor(suppy, sfBlack);
	        			sfRectangleShape_setSize(suppy, size);
	        			sfRectangleShape_setPosition(suppy, position);
	        			sfRenderWindow_drawRectangleShape(window, suppy, NULL);
	        		}*/
	       			return;
	       		case sfEvtMouseButtonPressed:
	       			if (event.mouseButton.x > 148 && event.mouseButton.x < 650
	       			 && event.mouseButton.y > 523 && event.mouseButton.y < 573) {
	       				struct hand hand = engine->players[0].hand;
	       				if (event.mouseButton.x > 604) {
	       					if (event.mouseButton.x > 613 &&
	       						 hand.last_tile != NO_TILE_INDEX) {
	       						input->action = ACTION_DISCARD;
	       						input->tile = hand.last_tile;
								hand.last_tile = NO_TILE_INDEX;
	       					}
	       					return;
	       				}
	       				int i = (event.mouseButton.x - 149) / 35;
	       				int cpt = 0;
	       				input->action = ACTION_DISCARD;
	      				for (histo_index_t index = 0; index < 34; ++index) {
	        				cpt += hand.histo.cells[index]
	        				 -(index == hand.last_tile);
	        				if (cpt > i) {
	        					input->tile = index;
	       						break;
	        				}
	        			}
	        			hand.last_tile = NO_TILE_INDEX;
						return;
	        		}
	        		break;
	       		default:
	        		break;
	       	}
	    }
    }
}