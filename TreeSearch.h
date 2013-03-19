#ifndef TREESEARCH_H
#define TREESEARCH_H
#include "Board.h"
#include "structures.h"

//returns a list of valid moves and creates associated children in board->children as a side-effect
vector <_Move*> generate_moves(Board *board, int player_id);

//make a random [legal] move
_Move *random_move(Board *board, int player_id);

//helper functions for depth-limited minimax
double dl_maxV(Board *node, int depth_limit, int player_id);
double dl_minV(Board *node, int depth_limit, int player_id);

//depth-limited minimax
_Move *dl_minimax(Board *root, int depth_limit, int player_id);

//TODO: iterative deepening depth-limited minimax

#endif

