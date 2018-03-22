#define _POSIX_C_SOURCE 199309L
#include "AI/detect.h"
#include "console_io.h"
#include "core/riichi_engine.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

#include "network/net_server.h"
#include "network/net_client.h"

#define NETWORK_TEST 0

void wait_for_players(struct riichi_engine *engine) {
	const unsigned short port_min = 5000, port_max = 10000;
	const struct timespec delay = {tv_sec : 0, tv_nsec : 250 * 1000000};
	const time_t timeout = 5;

	struct net_server *server = &engine->server;
	if (listen_net_server(&engine->server, port_min, port_max)) {
		return;
	}

	wprintf(L"Wait for network players ? (Y/N)\n> ");
	char c = getchar();
	while (c == 'Y' || c == 'y') {
		if (engine->server.nb_clients >= 3) {
			wprintf(L"No more room for other clients\n");
			break;
		}
		time_t t1 = time(NULL);
		do {
			if (check_new_connection_net_server(server)) {
				wprintf(L"New connection!\n");
				for (int p = 1; p < 4; ++p) {
					struct player *player = &engine->players[p];
					if (player->player_type != PLAYER_AI)
						continue;

					player->net_id = server->nb_clients - 1;
					player->player_type = PLAYER_CLIENT;
					break;
				}
				break;
			}
			nanosleep(&delay, NULL);
		} while (time(NULL) - t1 < timeout);
		wprintf(L"Continue waiting for network players ? (Y/N)\n> ");
		while (getchar() != '\n');
		c = getchar();
	}

	stop_listen_net_server(&engine->server);
}

int main() {
	setlocale(LC_ALL, "");
	srand(time(NULL));

	if (NETWORK_TEST) {
		client_test();
		return 0;
	}

	struct riichi_engine engine;
	enum player_type ptype = AI_MODE ? PLAYER_AI : PLAYER_HOST;
	init_riichi_engine(&engine, ptype, PLAYER_AI, PLAYER_AI, PLAYER_AI);

	if (!AI_MODE)
		wait_for_players(&engine);

	char c = 0;
	do {
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

	wprintf(L"\nYou played %d game(s).\n", engine.nb_games);
}
