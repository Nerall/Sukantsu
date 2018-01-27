# Sukantsu
Projet de S4

# DONE List
## Struct histogram
- struct histogram
  - histo_cells_t cells[HISTO_INDEX_MAX]
- functions
  - init_histogram(...)
  - add_histogram(...)
  - remove_histogram(...)
  
## Struct wall
- struct wall
  - struct histogram histo
  - int nb_tiles
- functions
  - random_pop_wall(...)
  
## Struct group
- struct group
  - unsigned char hidden
  - histo_index_t tiles[GROUP_NB_TILES]
- functions
  - init_group(...)
  
## Struct hand
- struct hand
  - struct histogram histo
  - struct group groups[HAND_NB_GROUPS]
  - unsigned char nb_groups
  - histo_index_t last_tile
- functions
  - init_hand(...)
  - add_group_hand(...)
  - add_tile_hand(...)
  - remove_tile_hand(...)

# TODO List
## Tout le reste
- voilà, voilà
