#include "console_io.h"
#include "AI/detect.h"
#include "core/histogram.h"
#include "core/riichi_engine_s.h"
#include "core/hand.h"
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
		for (int i = 0; i < grouplist->nb_groups; ++i)
			print_groups(grouplist->groups[i]);
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
			// engine->grouplist must contain the victory grouplist
			wprintf(L"TSUMO!\n");
			print_victory(player_hand, &engine->grouplist);
			break;
		}

		case PHASE_CLAIM: {
			//wprintf(L"Do you want to claim? (todo)\n");
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

void display(const struct riichi_engine *engine, int current_player) {
	// Init window
	sfRenderWindow* window;
	sfVideoMode mode = {800, 600, 32};

	// Init Textures
	sfTexture* textureslist[35];
	for (int i = 0; i < 35; ++i) {
		char path[20];
		sprintf(path, "src/tiles/%d.png", i);
		textureslist[i] = sfTexture_createFromFile(path, NULL);
	}

	// Create window
	window = sfRenderWindow_create(mode, "Sukantsu", sfResize | sfClose, NULL);
  while (sfRenderWindow_isOpen(window)) {
		sfEvent event;
		sfVector2i mouseposition;
		while (sfRenderWindow_pollEvent(window, &event)) {
				/*if (event.type == sfEvtClosed) {
					sfRenderWindow_close(window);
				}*/
				/*case sfEvtMouseButtonReleased:
					mouseposition = sfMouse_getPositionRenderWindow(window);
					if (mouseposition.y > 500) {
						sfRenderWindow_close(window);
					}
					break;
*/
				if (event.type == sfEvtKeyPressed) {
					for (int i = 0; i < 35; ++i) {
						sfTexture_destroy(textureslist[i]);
					}
					sfRenderWindow_close(window);
					sfRenderWindow_destroy(window);
					return ;
				}
		}
		// Add color
		sfColor background;
		background = sfColor_fromRGB(0, 128, 255);
		sfRenderWindow_clear(window, background);

		// Display hand
		struct hand handcopy;
		copy_hand(&engine->players[current_player].hand, &handcopy);
		sfSprite* spriteslist[14];
		// Position of each tile
		sfVector2f scale;
		scale.x = 0.20;
		scale.y = 0.20;
		sfVector2f bordersize;
		sfRectangleShape* border;
		bordersize.x = 45;
		bordersize.y = 61;
		for (int i = 0; i < 14; ++i) {
				sfVector2f tileposition;
				tileposition.x = 50 + 46 * i + (i == 13) * 9;
				tileposition.y = 500;

				border = sfRectangleShape_create();
				sfRectangleShape_setFillColor(border, sfTransparent);
				sfRectangleShape_setOutlineColor(border, sfBlack);
				sfRectangleShape_setOutlineThickness(border, 1.0);
				sfRectangleShape_setPosition(border, tileposition);
				sfRectangleShape_setSize(border, bordersize);
				sfRenderWindow_drawRectangleShape(window, border, NULL);
				sfRectangleShape_destroy(border);

				spriteslist[i] = sfSprite_create();
				for (histo_cell_t j = 0; j < 34; ++j) {
					if (handcopy.histo.cells[j] - (handcopy.last_tile == j) > 0) {
						--handcopy.histo.cells[j];
						sfSprite_setTexture(spriteslist[i], textureslist[j], 1);
						break;
					}
				}
				if (i == 13 && handcopy.last_tile != NO_TILE_INDEX) {
					--handcopy.histo.cells[handcopy.last_tile];
					sfSprite_setTexture(spriteslist[i],
						textureslist[handcopy.last_tile], 1);
				}
				sfSprite_setPosition(spriteslist[i], tileposition);
				sfSprite_setScale(spriteslist[i], scale);
				sfRenderWindow_drawSprite(window, spriteslist[i], NULL);
				sfSprite_destroy(spriteslist[i]);
			}

		// Display center
		sfRectangleShape* center;
		sfVector2f centerposition;
		centerposition.x = 320;
		centerposition.y = 220;
		sfVector2f centersize;
		centersize.x = 160;
		centersize.y = 160;
		center = sfRectangleShape_create();
		sfRectangleShape_setFillColor(center, sfGreen);
		sfRectangleShape_setOutlineColor(center, sfBlack);
		sfRectangleShape_setOutlineThickness(center, 1.0);
		sfRectangleShape_setPosition(center, centerposition);
		sfRectangleShape_setSize(center, centersize);
		sfRenderWindow_drawRectangleShape(window, center, NULL);
		sfRectangleShape_destroy(center);

		// Display discards player 1
		scale.x = 0.11;
		scale.y = 0.11;
		bordersize.x = 25;
		bordersize.y = 34;
		sfSprite* spritediscard1[handcopy.discardlist.nb_discards];
		for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
			sfVector2f tileposition;
			if (i < 18) {
			tileposition.x = 322 + 26 * (i % 6);
			tileposition.y = 381 + 35 * (i / 6);
			}
			else {
				tileposition.x = 322 + 26 * i;
				tileposition.y = 451;
			}
			spritediscard1[i] = sfSprite_create();
			sfSprite_setTexture(spritediscard1[i],
				textureslist[handcopy.discardlist.discards[i]], 1);
			sfSprite_setPosition(spritediscard1[i], tileposition);
			sfSprite_setScale(spritediscard1[i], scale);
			sfRenderWindow_drawSprite(window, spritediscard1[i], NULL);
			sfSprite_destroy(spritediscard1[i]);
			border = sfRectangleShape_create();

			sfRectangleShape_setFillColor(border, sfTransparent);
			sfRectangleShape_setOutlineColor(border, sfBlack);
			sfRectangleShape_setOutlineThickness(border, 1.0);
			sfRectangleShape_setPosition(border, tileposition);
			sfRectangleShape_setSize(border, bordersize);
			sfRenderWindow_drawRectangleShape(window, border, NULL);
			sfRectangleShape_destroy(border);
		}

		// Display discards player 2 (code duplicated)
		copy_hand(&engine->players[(current_player + 1) % 4].hand, &handcopy);

		sfSprite* spritediscard2[handcopy.discardlist.nb_discards];
		for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
			sfVector2f tileposition;
			if (i < 18) {
			tileposition.x = 481 + 35 * (i / 6);
			tileposition.y = 378 - 26 * (i % 6);
			}
			else {
				tileposition.x = 551;
				tileposition.y = 378 - 26 * i;
			}
			spritediscard2[i] = sfSprite_create();
			sfSprite_setTexture(spritediscard2[i],
				textureslist[handcopy.discardlist.discards[i]], 1);
			sfSprite_setPosition(spritediscard2[i], tileposition);
			sfSprite_setRotation(spritediscard2[i], 270);
			sfSprite_setScale(spritediscard2[i], scale);
			sfRenderWindow_drawSprite(window, spritediscard2[i], NULL);
			sfSprite_destroy(spritediscard2[i]);
			border = sfRectangleShape_create();

			sfRectangleShape_setFillColor(border, sfTransparent);
			sfRectangleShape_setOutlineColor(border, sfBlack);
			sfRectangleShape_setOutlineThickness(border, 1.0);
			sfRectangleShape_setPosition(border, tileposition);
			sfRectangleShape_setRotation(border, 270);
			sfRectangleShape_setSize(border, bordersize);
			sfRenderWindow_drawRectangleShape(window, border, NULL);
			sfRectangleShape_destroy(border);
		}

		// Display discards player 3 (code duplicated)
		copy_hand(&engine->players[(current_player + 2) % 4].hand, &handcopy);

		sfSprite* spritediscard3[handcopy.discardlist.nb_discards];
		for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
			sfVector2f tileposition;
			if (i < 18) {
			tileposition.x = 478 - 26 * (i % 6);
			tileposition.y = 219 - 35 * (i / 6);
			}
			else {
				tileposition.x = 478 - 26 * i;
				tileposition.y = 149;
			}
			spritediscard3[i] = sfSprite_create();
			sfSprite_setTexture(spritediscard3[i],
				textureslist[handcopy.discardlist.discards[i]], 1);
			sfSprite_setPosition(spritediscard3[i], tileposition);
			sfSprite_setRotation(spritediscard3[i], 180);
			sfSprite_setScale(spritediscard3[i], scale);
			sfRenderWindow_drawSprite(window, spritediscard3[i], NULL);
			sfSprite_destroy(spritediscard3[i]);
			border = sfRectangleShape_create();

			sfRectangleShape_setFillColor(border, sfTransparent);
			sfRectangleShape_setOutlineColor(border, sfBlack);
			sfRectangleShape_setOutlineThickness(border, 1.0);
			sfRectangleShape_setPosition(border, tileposition);
			sfRectangleShape_setRotation(border, 180);
			sfRectangleShape_setSize(border, bordersize);
			sfRenderWindow_drawRectangleShape(window, border, NULL);
			sfRectangleShape_destroy(border);
		}

		// Display discards player 4 (code duplicated)
		copy_hand(&engine->players[(current_player + 3) % 4].hand, &handcopy);

		sfSprite* spritediscard4[handcopy.discardlist.nb_discards];
		for (int i = 0; i < handcopy.discardlist.nb_discards; ++i) {
			sfVector2f tileposition;
			if (i < 18) {
			tileposition.x = 319 - 35 * (i / 6);
			tileposition.y = 222 + 26 * (i % 6);
			}
			else {
				tileposition.x = 478 - 26 * i;
				tileposition.y = 149;
			}
			spritediscard4[i] = sfSprite_create();
			sfSprite_setTexture(spritediscard4[i],
				textureslist[handcopy.discardlist.discards[i]], 1);
			sfSprite_setPosition(spritediscard4[i], tileposition);
			sfSprite_setRotation(spritediscard4[i], 90);
			sfSprite_setScale(spritediscard4[i], scale);
			sfRenderWindow_drawSprite(window, spritediscard4[i], NULL);
			sfSprite_destroy(spritediscard4[i]);
			border = sfRectangleShape_create();

			sfRectangleShape_setFillColor(border, sfTransparent);
			sfRectangleShape_setOutlineColor(border, sfBlack);
			sfRectangleShape_setOutlineThickness(border, 1.0);
			sfRectangleShape_setPosition(border, tileposition);
			sfRectangleShape_setRotation(border, 90);
			sfRectangleShape_setSize(border, bordersize);
			sfRenderWindow_drawRectangleShape(window, border, NULL);
			sfRectangleShape_destroy(border);
		}

		sfRenderWindow_display(window);
	}
}

void init_tilesGUI(struct tilesGUI *tilesGUI, enum typeGUI typeGUI, int current_player) {
	ASSERT_BACKTRACE(tilesGUI);
	tilesGUI->rotation = 360 - 90 * (typeGUI + 1);
	tilesGUI->typeGUI = typeGUI;
	sfVector2f scale;
	sfVector2f tileposition;
	sfVector2f tileincrement;
	sfVector2f bordersize;
	switch (typeGUI) {
		case PLAYERHAND:
			scale.x = 0.2;
			scale.y = 0.2;
			bordersize.x = 45;
			bordersize.y = 61;
			switch (current_player) {
				case 0:
					tileposition.x = 50;
					tileposition.y = 500;
					tileincrement.x = 46;
					tileincrement.y = 0;
					break;/*
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
		case PLAYERHANDOPEN:
			scale.x = 0.11;
			scale.y = 0.11;
			bordersize.x = 25;
			bordersize.y = 34;
			switch (current_player) {/*
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
					tileposition.x = 481;
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
					tileposition.x = 319;
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
			scale.y = 0.11;
			bordersize.x = 25;
			bordersize.y = 34;
			tileposition.x = 0;
			tileposition.y = 0;
			tileincrement.x = 0;
			tileincrement.y = 0;
			break;
		default:
			scale.x = 0.15;
			scale.y = 0.15;
			bordersize.x = 0;
			bordersize.y = 0;
			tileposition.x = 0;
			tileposition.y = 0;
			break;
	}
	tilesGUI->scale = scale;
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
	centerposition.x = 320;
	centerposition.y = 220;
	gameGUI->centerposition = centerposition;
	sfVector2f centersize;
	centersize.x = 160;
	centersize.y = 160;
	gameGUI->centersize = centersize;
}
