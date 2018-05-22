#define _POSIX_C_SOURCE 199309L
#include "core/player.h"
#include "core/riichi_engine.h"
#include "core/riichi_engine_s.h"
#include "network/net_client.h"
#include "network/net_server.h"
#include "network/net_packets.h"
#include <SFML/Graphics.h>
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>

static void wait_for_players(struct riichi_engine *engine) {
	const unsigned short port_min = 5000, port_max = 10000;
	const struct timespec delay = {tv_sec : 0, tv_nsec : 250 * 1000000};
	const time_t timeout = 10;

	struct net_server *server = &engine->server;
	if (listen_net_server(&engine->server, port_min, port_max)) {
		return;
	}

	wprintf(L"Wait for network players ? (y/n)\n> ");
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
		wprintf(L"Continue waiting for network players ? (y/n)\n> ");
		while (getchar() != '\n')
			;
		c = getchar();
	}

	stop_listen_net_server(&engine->server);
}

static void rotate_players(struct riichi_engine *engine) {
	++engine->nb_rounds;
	for (int i = 0; i < 4; ++i) {
		enum table_pos *pos = &engine->players[i].player_pos;
		switch (*pos) {
			case EAST:
				*pos = NORTH;
				break;
			case SOUTH:
				*pos = EAST;
				break;
			case WEST:
				*pos = SOUTH;
				break;
			case NORTH:
				*pos = WEST;
				break;
		}
	}
}

static void send_end_game(struct riichi_engine *engine) {
	struct net_packet_init packet = {
		packet_type : PACKET_INIT,
		end_game : 1,
	};

	struct net_server *server = &engine->server;

	for (int i = 0; i < 4; ++i) {
		switch (engine->players[i].player_pos) {
			case EAST:
				packet.score_east = engine->players[i].player_score;
				break;

			case SOUTH:
				packet.score_south = engine->players[i].player_score;
				break;

			case WEST:
				packet.score_west = engine->players[i].player_score;
				break;

			case NORTH:
				packet.score_north = engine->players[i].player_score;
				break;
		}
	}

	// [SERVER] Send tiles to all clients
	for (int client_index = 0; client_index < NB_PLAYERS; ++client_index) {
		struct player *player = &engine->players[client_index];

		if (player->player_type != PLAYER_CLIENT)
			continue;

		packet.player_pos = player->player_pos;
		packet.histo = player->hand.histo;

		int s = send_data_to_client(server, player->net_id, &packet,
		                            sizeof(struct net_packet_init));
		player->net_status = !s;
		if (s) {
			fprintf(stderr,
			        "[ERROR][SERVER] Error while sending"
			        " init data to player %d\n",
			        client_index);
		}
	}
}

void host_main() {
	struct riichi_engine engine;
	enum player_type ptype = AI_MODE ? PLAYER_AI : PLAYER_HOST;
	init_riichi_engine(&engine, ptype, PLAYER_AI, PLAYER_AI, PLAYER_AI);

	wait_for_players(&engine);

	engine.gameGUI.window = sfRenderWindow_create(
	    engine.gameGUI.mode, "Sukantsu (host)", sfResize | sfClose, NULL);

	engine.server.selector = sfSocketSelector_create();

	char c = 0;
	int nb_won_games = 0;
	time_t t_init = time(NULL);

	do {
		int round = engine.nb_rounds;
		int win_pos = play_riichi_game(&engine);
		char *pnames[] = {"Thibaut", "Nicolas", "Manuel", "Gabriel"};
		if (win_pos == -1) {
			wprintf(L"Result: Draw\n\n");
		} else {
			wprintf(L"Result: %s has won!\n\n", pnames[(win_pos + round) % 4]);
			++nb_won_games;
		}

		for (int i = 0; i < 4; ++i) {
			wprintf(L"%s has %d points.\n", pnames[i],
			        engine.players[i].player_score);
		}

		// Rotate if winner != EAST
		if (win_pos != 0 ||
		    (win_pos == -1 && engine.players[round % 4].hand.tenpai))
			rotate_players(&engine);

		if (AI_MODE) {
			if (engine.nb_rounds > 3)
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

	send_end_game(&engine);

	wprintf(L"\nYou played %d seconds.", time(NULL) - t_init);
	wprintf(L"\nYou played %d game%s.\n", engine.nb_games,
	        (engine.nb_games > 1) ? "s" : "");
	wprintf(L"%d game%s won.\n", nb_won_games,
	        (nb_won_games > 1) ? "s were" : " was");

	// Destroy GUI
	destroy_gameGUI(&engine.gameGUI);
	sfRenderWindow_destroy(engine.gameGUI.window);

	sfSocketSelector_destroy(engine.server.selector);
	clean_net_server(&engine.server);
}

void client_main() {
	struct net_client client;

	char server[64];
	unsigned short port;
	do {
		wprintf(L"Type a server to connect to\n> ");
		fflush(stdout);
		for (char *c = server; c < server + 63; ++c) {
			*c = getchar();
			if (*c == '\n') {
				*c = '\0';
				break;
			}
		}

		wprintf(L"Type a port to connect to\n> ");
		fflush(stdout);
		while (!scanf("%hu", &port))
			;
		while (getchar() != '\n')
			;
	} while (!connect_to_server(&client, server, port));

	client_main_loop(&client);

	disconnect_from_server(&client);
}

int main() {
	setlocale(LC_ALL, "");
	srand(time(NULL));
	wprintf(L"Host or Client ? (h/c)\n> ");
	fflush(stdout);
	char c;
	while ((c = getchar()) != 'h' && c != 'c') {
		if (c == '\n') {
			wprintf(L"> ");
			fflush(stdout);
		}
	}
	while (getchar() != '\n')
		;
	if (c == 'h') {
		host_main();
	} else {
		client_main();
	}
}
