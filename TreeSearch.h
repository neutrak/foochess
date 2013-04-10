#ifndef TREESEARCH_H
#define TREESEARCH_H
#include <sys/time.h>
#include "AI.h"
#include "structures.h"

#define AVG(X,Y) ((X+Y)/2)
//an out-of-bounds return code to tell when a call ran out of time
#define OUT_OF_TIME (HEURISTIC_MINIMUM*2)

typedef enum
{
  PAWN,
  ROOK,
  KNIGHT,
  BISHOP,
  QUEEN,
  KING,
  
  PIECE_MAX
} piece_type;

class TreeSearch
{
private:
  //TODO: store history table here, PV, etc.; make getters so the user can get those
  
public:
  //returns a list of valid moves and creates associated children in board->children as a side-effect
  static vector <_Move*> generate_moves(Board *board, int player_id);
  
  //returns true if there is a stalemate caused by repeated moves
  //else false
  static bool stalemate_by_repeat(vector <_Move*> move_accumulator);
  
  //returns true if there is insufficient material to checkmate
  //otherwise false
  static bool insufficient_material(Board *board, int player_id);
  
  //free the memory referenced by a move accumulator vector
  static void free_move_acc(vector <_Move*> move_accumulator);
  
  //the heuristic we'll be using for minimax
  static double informed_attack_heuristic(Board *node, int player_id, bool max);
  static double informed_defend_heuristic(Board *node, int player_id, bool max);
  static double naive_attack_heuristic(Board *node, int player_id, bool max);
  static double naive_defend_heuristic(Board *node, int player_id, bool max);
  
  //how much time to allocate to this move given the board and how much time we have left
  static double time_for_this_move(Board *board, double time_remaining, int moves_made);
  
  //make a random [legal] move
  static _Move *random_move(Board *board, int player_id);
  
  //helper functions for depth-limited minimax
  //NOTE: the move_accumulator everywhere is for detecting a statelmate-by-repeat situation
  
  //a generalized function for code-reuse
  //this serves the functions of dl_maxV, dl_minV, abdl_maxV, and abdl_minV
  //those functions themselves just carefully choose the arguments to give to this
  //max should be true to max, false to min
  //prune should be true for pruning, false for not; alpha and beta are ignored when prune is false
  static double general_min_or_max_pruning(Board *node, int depth_limit, int qs_depth_limit, int player_id, bool max, heuristic heur, bool prune, double alpha, double beta, vector<_Move*> move_accumulator, bool time_limit, double time_for_move, double time_used);
  
  //depth-limited minimax
  static _Move *dl_minimax(Board *root, int depth_limit, int qs_depth_limit, int player_id, vector<_Move*> move_accumulator, heuristic heur, bool prune, bool time_limit, double time_for_move, double time_used);
  
  //NOTE: the way a non-quiescent search is done is to set the quiescent depth limit as 0
  //iterative deepening depth-limited minimax with an option to time-limit instead of using a given max depth
  static _Move *id_minimax(Board *root, int max_depth_limit, int qs_depth_limit, int player_id, vector<_Move*> move_accumulator, heuristic heur, bool prune, bool time_limit, double time_remaining);
};

#endif

