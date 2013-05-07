#ifndef BOARD_H
#define BOARD_H

#include "structures.h"
#include "SuperPiece.h"
#include "HistTable.h"
#include <vector>
using namespace std;

enum algorithm
{
  USER, //let the user actually play
  RANDOM, //random
  TREE_SEARCH, //any kind of tree search
  
  ALGO_COUNT
};

class HistTable;

//bounds for heuristic values; it's needed a few places
//9 is the value for a queen, 16 is the total number of pieces
//therefore naive point values can never exceed these bounds
#define HEURISTIC_MINIMUM (-(16*9))
#define HEURISTIC_MAXIMUM (16*9)

#define WHITE 0
#define BLACK 1

class Board
{
private:
  static const int width=8;
  static const int height=8;
  _SuperPiece *state[width*height];
  
  //whether or not the board is currently in check
  bool white_check;
  bool black_check;
  
  //last moved piece, needed to tell if an en passant is legal
  _SuperPiece *last_moved;
  //and the last move applied to this board (initially NULL)
  _Move *last_move_made;
  
  //store capture information because that's needed to identify things in the history table
  //what type of piece was captured last (can also check moves_since_capture to tell if this was last move)
  int last_capture_type;
  
  //the number of moves since the last capture, pawn advancement (respectively)
  //initialized to 0 in the normal constructor; carried in the copy constructor
  int moves_since_capture;
  int moves_since_advancement;
  
  //parent
  Board *p;
  vector<Board*> children;
  
  //value to sort by when we are a child of another board
  double sorting_value;
  
public:
  //constructor, makes board internal structures for a starting state
  Board();
  
  //place a piece on the board given some information about the piece
  void place_piece(int id, int owner, int file, int rank, int hasMoved, int type, bool haveChecked, int movements);
  
  //copy constructor
  Board(Board *board);
  
  //equality check (just checks type, owner, position of pieces, not history or anything)
  bool equals(Board *board);
  
  //destructor, cleans up nicely
  ~Board();
  
  //accessors
  vector<Board*> get_children(){ return children; }
  void resize_children(unsigned int new_size){ children.resize(new_size); }
  _Move *get_last_move_made(){ return last_move_made; }
  bool get_check(int player_id){ return (player_id==WHITE) ? white_check : black_check; }
  int get_moves_since_capture(){ return moves_since_capture; }
  int get_moves_since_advancement(){ return moves_since_advancement; }
  int get_last_capture_type(){ return last_capture_type; }
  double get_sorting_value(){ return sorting_value; }
  void set_sorting_value(double s){ sorting_value=s; }
  
  //deal with other nodes in the structure
  void add_child(Board *board);
  void remove_child(size_t index);
  void clear_children();
  void swap_children(size_t a, size_t b);
  //a helper function to randomize children (and by extension move choices)
  void shuffle_children();
  //order children by history table values, given a history table to use
  void history_order_children(HistTable *hist);
  //order children by heursitic values
  void heuristic_order_children(int player_id, bool max, bool heur_pawn_additions, bool heur_position_additions, double enemy_weight, double owned_weight);
  
  //an in-place quicksort implementation, sorting by sorting_value values of children (which can be set as anything)
  void quicksort_children(int lower_bound, int upper_bound);
  //quicksort helper
  int quicksort_partition_children(int lower_bound, int upper_bound, int pivot_index);
  
  //display whatever the current state of the board is
  void output_board();
  
  //finds the king
  _SuperPiece *find_king(int player_id);
  
  //checks whether a given player is in check at a given position in the current board
  bool in_check(int file, int rank, int player_id);
  //and helper functions
  bool in_check_diagonal(int file, int rank, int player_id);
  bool in_check_cardinal(int file, int rank, int player_id);
  bool in_check_fromknight(int file, int rank, int player_id);
  
  //the piece at a given location
  _SuperPiece *get_element(int file, int rank);
  
  //returns memory for a move structure for a piece
  _Move *make_move(_SuperPiece *p, int to_file, int to_rank, int promote_type);
  
  //copies a move
  //(remember to free this later)
  _Move *copy_move(_Move *move);
  
  //transforms the internal board to be
  //what it should be after a given move is applied
  void apply_move(_Move *move, bool update_check);
  
  //update an internal variable based on a board position
  void set_last_moved(int file, int rank);
  
  //a transformation to get to the next direction
  //returns 0 when there are none left
  int next_direction(int direction);
  
  //generate the legal moves for different pieces
  //these should be subsets of what's returned by legal_moves
  
  vector<_Move*> pawn_moves(_SuperPiece *piece);
  vector<_Move*> rook_moves(_SuperPiece *piece);
  vector<_Move*> knight_moves(_SuperPiece *piece);
  vector<_Move*> bishop_moves(_SuperPiece *piece);
  vector<_Move*> queen_moves(_SuperPiece *piece);
  vector<_Move*> king_moves(_SuperPiece *piece);
  
  //a vector of valid moves that can be done by the piece in question
  vector<_Move*> legal_moves(_SuperPiece *piece);
  
  //common heuristics and/or helpers for tree search
  
  //the value of a given type of piece
  double point_value(int type);
  
  //when informed is false just point values as commonly defined
  //when informed is true position is taken into account, etc.
  double points(int player_id, bool informed, bool attack_ability);
  
  //a general heuristic function to call, with parameters for heuristic options
  double heuristic_value(int player_id, bool max, bool heur_pawn_additions, bool heur_position_additions, double enemy_weight, double owned_weight);
  
  //this is a count of how many tiles on the board are attackable by the given player
  //it's something I'm playing with as part of heuristic calculation
  double board_ownership(int player_id);
  
  //returns true if this board state is "quiescent"; else false
  bool quiescent();
};

#endif

