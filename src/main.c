#include "core/detect.h"
#include "core/hand.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wchar.h>
#include <locale.h>

// DEBUG FUNCTION
// Will print an histogram to stdout
static void print_histo(struct histogram *histo) {
  printf("--- 0 1 2 3 4 5 6 7 8 - Indexes\n");
  printf(" |  Dots (p)\n 0 ");
  for (int i = 0; i < 9; ++i)
    printf(" %d", histo->cells[i]);
  printf("\n |  Bamboos (s)\n 9 ");
  for (int i = 9; i < 18; ++i)
    printf(" %d", histo->cells[i]);
  printf("\n |  Cracks (m)\n 18");
  for (int i = 18; i < 27; ++i)
    printf(" %d", histo->cells[i]);
  printf("\n |  Honor tiles (z)\n 27");
  for (int i = 27; i < 34; ++i)
    printf(" %d", histo->cells[i]);
  printf("\n");
}

// DEBUG FUNCTION
// Will print hand groups to stdout
static void print_groups(struct group *groups) {
  printf("Group\n");
  for (int i = 0; i < HAND_NB_GROUPS; ++i) {
    switch(groups[i].type) {
      case PAIR:
        printf("Pair (%d, %d)\n", groups[i].tile, groups[i].tile);
      case SEQUENCE:
        printf("Sequence (%d, %d, %d)\n", groups[i].tile,
        groups[i].tile + 1, groups[i].tile + 2);
      case TRIPLET:
        printf("Triplet (%d, %d, %d)\n", groups[i].tile,
        groups[i].tile, groups[i].tile);
      case QUAD:
        printf("Quad (%d, %d, %d, %d)\n", groups[i].tile,
        groups[i].tile, groups[i].tile, groups[i].tile);
    }
  }
}

static void print_victory(struct hand *hand, struct grouplist *grouplist) {
  for (int i = 0; i < grouplist->nb_groups; ++i) {
    struct hand handcopy;
    copy_hand(hand, &handcopy);
    struct histogram histo = groups_to_histo(&handcopy);
    print_histo(&histo);
    if (iskokushi(hand))
      printf("WOW, Thirteen orphans!!");
    else {
      if (ischiitoi(hand))
        printf("WOW, Seven pairs!");
      print_groups(grouplist->groups[i]);
    }
  }
}

void clear_stream(FILE *in) {
  int ch;
  clearerr(in);
  do {
    ch = getc(in);
  } while (ch != '\n' && ch != EOF);
  clearerr(in);
}

int main() {
  setlocale (LC_ALL, "");
  srand(time(NULL));
  setbuf(stdout, NULL); // To flush automatically stdout

  printf("Sizeof structures:");
  printf("\thistogram : %lu\n", sizeof(struct histogram));
  printf("\thistobit  : %lu\n", sizeof(struct histobit));
  printf("\tgroup     : %lu\n", sizeof(struct group));
  printf("\thand      : %lu\n", sizeof(struct hand));

  // Initialization
  struct histogram wall;
  init_histogram(&wall, 4);
  struct hand hand;
  init_hand(&hand);
  struct grouplist grouplist;
  //histo_cell_t starthand[] = { 3, 4, 4, 5, 6, 6, 7, 8, 8, 8, 10, 11, 12, 22 };

  // Give 13 tiles to each player
  for (int i = 0; i < 13; ++i) {
    //add_tile_hand(&hand, starthand[i]);
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
    printf("\nTile drawn: %u\n", randi);
    printf("Draws remaining: %u\n\n", (wall.nb_tiles - 14) / 4);

    if(get_histobit(&hand.wintiles, randi)) {
      puts("TSUMO!");
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
    }

    // Ask for tile discard
    unsigned int index = NO_TILE_INDEX;
    while (!is_valid_index(index) || hand.histo.cells[index] == 0) {
      while (scanf("%u", &index) != 1) {
        clear_stream(stdin);
        fflush(stdout);
      }
    }
    remove_tile_hand(&hand, index);

    // Show winning tiles
    if (index != (unsigned int)hand.last_tile) {
      tenpailist(&hand, &grouplist);
      if (get_histobit(&hand.riichitiles, index)) {
        printf("You win if you get:\n");
        for (int w = 0; w < 34; ++w) {
          if(get_histobit(&hand.wintiles, w))
            printf("%u\n", w);
        }
      }
    }

    // Give one tile to each other player
    // Same code is copied twice
    histo_index_t discard = random_pop_histogram(&wall);
    printf("South's discard: %u\n", discard);
    if (get_histobit(&hand.wintiles, discard)) {
      puts("RON!");
      add_tile_hand(&hand, discard);
      makegroups(&hand, &grouplist);
      print_victory(&hand, &grouplist);
      return 1;
    }
    discard = random_pop_histogram(&wall);
    printf("West's discard: %u\n", discard);
    if (get_histobit(&hand.wintiles, discard)) {
      puts("RON!");
      add_tile_hand(&hand, discard);
      makegroups(&hand, &grouplist);
      print_victory(&hand, &grouplist);
      return 1;
    }
    discard = random_pop_histogram(&wall);
    printf("Norths discard: %u\n", discard);
    if (get_histobit(&hand.wintiles, discard)) {
      puts("RON!");
      add_tile_hand(&hand, discard);
      makegroups(&hand, &grouplist);
      print_victory(&hand, &grouplist);
      return 1;
    }
  }
  printf("End of the game.\n");
  return 0;
}
