#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include "SuperPiece.h"
#include <vector>
using namespace std;

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
  //this will determine a few things, like whether castling is a valid move
  bool white_check;
  bool black_check;
  
  //last moved piece, needed to tell if an en passant is legal
  _SuperPiece *last_moved;
  //and the last move applied to this board (initially NULL)
  _Move *last_move_made;
  
  //parent
  Board *p;
  vector<Board*> children;
  
public:
  //constructor, makes board internal structures based off of the data I'm given
  Board(vector<Piece> pieces, Board *parent);
  
  //copy constructor
  Board(Board *board);
  
  //equality check (just checks type, owner, position of pieces, not history or anything)
  bool equals(Board *board);
  
  //destructor, cleans up nicely
  ~Board();
  
  //accessors
  vector<Board*> get_children(){ return children; }
  _Move *get_last_move_made(){ return last_move_made; }
  bool get_check(int player_id){ return (player_id==WHITE) ? white_check : black_check; }
  
  //deal with other nodes in the structure
  void add_child(Board *board);
  void remove_child(Board *board);
  void clear_children();
  
  //display whatever the current state of the board is
  void output_board();
  
  //finds the king
  _SuperPiece *find_king(int player_id);
  
  //checks whether a given player is in check at a given position in the current board
  bool in_check(int file, int rank, int player_id);
  
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
  
  //just point values as commonly defined
  int naive_points(int player_id);
  
  //point values with position taken into account, etc.
  int informed_points(int player_id);
};

#endif

