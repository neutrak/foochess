#ifndef TREESEARCH_H
#define TREESEARCH_H
#include "Board.h"
#include "structures.h"

//returns a list of valid moves and creates associated children in board->children as a side-effect
vector <_Move*> generate_moves(Board *board, int player_id);

//make a random [legal] move
_Move *random_move(Board *board, int player_id);

//helper functions for depth-limited minimax

//a generalized function for code-reuse
//this serves the functions of dl_maxV, dl_minV, abdl_maxV, and abdl_minV
//those functions themselves just carefully choose the arguments to give to this
//max should be true to max, false to min
//prune should be true for pruning, false for not; alpha and beta are ignored when prune is false
double general_min_or_max_pruning(Board *node, int depth_limit, int player_id, bool max, bool prune, double alpha, double beta);

double dl_maxV(Board *node, int depth_limit, int player_id);
double dl_minV(Board *node, int depth_limit, int player_id);

//depth-limited minimax
_Move *dl_minimax(Board *root, int depth_limit, int player_id);

//TODO: iterative deepening depth-limited minimax

#endif

