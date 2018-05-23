#ifndef _CONSOLE_INPUT_H_
#define _CONSOLE_INPUT_H_

#include "definitions.h"
#include <SFML/Graphics.h>

struct hand;
struct histogram;
struct group;
struct grouplist;
struct riichi_engine;

struct tilesGUI {
	sfSprite *tilesprite[24]; // max size of "hand" (discards' max length)
	int rotation;
	enum typeGUI typeGUI;
	sfVector2f scale;
	sfVector2f tileposition;
	sfVector2f tileincrement;
	sfRectangleShape *border;
	sfVector2f bordersize;
};

struct gameGUI {
	sfRenderWindow *window;
	sfVideoMode mode;
	sfColor background;
	sfTexture *textureslist[35];
	struct tilesGUI player1hand;
	struct tilesGUI player1handopen;
	struct tilesGUI player2hand;
	struct tilesGUI player2handopen;
	struct tilesGUI player3hand;
	struct tilesGUI player3handopen;
	struct tilesGUI player4hand;
	struct tilesGUI player4handopen;
	struct tilesGUI player1discards;
	struct tilesGUI player2discards;
	struct tilesGUI player3discards;
	struct tilesGUI player4discards;
	struct tilesGUI doras;
	sfRectangleShape *center;
	sfVector2f centerposition;
	sfVector2f centersize;
	sfColor centercolor;
	sfRectangleShape *suppx;
	sfRectangleShape *suppy;
};

histo_index_t char_to_index(char family, char number);

void index_to_char(histo_index_t index, char *family, char *number);

void print_histo(const struct histogram *histo, histo_index_t last_tile);

void print_groups(const struct group *groups);

void print_victory(const struct hand *hand, const struct grouplist *grouplist);

histo_index_t get_input(const struct histogram *histo, enum action *action);

void display_riichi(const struct riichi_engine *engine, int current_player);

void init_tilesGUI(struct tilesGUI *tilesGUI, enum typeGUI typeGUI,
                   int current_player);

void init_gameGUI(struct gameGUI *gameGUI);

void destroy_gameGUI(struct gameGUI *gameGUI);

// To remove
void display(struct riichi_engine *engine, int current_player);

void display_GUI(struct riichi_engine *engine);

void get_player_click(struct riichi_engine *engine, struct action_input *input);

#endif // _CONSOLE_INPUT_H_