#include "AI/detect.h"
#include "console_io.h"
#include "core/riichi_engine.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

int main() {
	setlocale(LC_ALL, "");
	srand(time(NULL));

	struct riichi_engine engine;
	enum player_type ptype = AI_MODE ? PLAYER_AI : PLAYER_HUMAN;
	init_riichi_engine(&engine, ptype, PLAYER_HUMAN, PLAYER_AI, PLAYER_AI);

	char c = 0;
	do {
		//int index_win = play(++engine.nb_games);
		int index_win = play_riichi_game(&engine);
		if (index_win == -1) {
			wprintf(L"Result: Draw\n");
		} else {
			wprintf(L"Result: Player %d has won!\n", index_win + 1);
		}
		if (AI_MODE) {
			if (index_win == 0)
				break;
			else
				continue;
		}

		wprintf(L"Do you want to continue (y/n)\n> ");
		fflush(stdout);

		do {
			c = getchar();
			if (c >= 'a')
				c += 'A' - 'a';
		} while (c != 'Y' && c != 'N');
		while (getchar() != '\n')
			;
	} while (c != 'N');

	wprintf(L"\nYou played %d game%s.\n", engine.nb_games,
  (engine.nb_games > 1) ? "s" : "");
}
