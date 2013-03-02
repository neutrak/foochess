#ifndef BOARD_H
#define BOARD_H

#include "Piece.h"
#include <vector>
using namespace std;

class Board
{
private:
  static const int width=8;
  static const int height=8;
  _Piece *state[width*height];
  bool have_checked[width*height];
  
  //parent
  Board *p;
  vector<Board*> children;
  
public:
  //constructor, makes board internal structures based off of the data I'm given
  Board(vector<Piece> pieces, Board *parent);
  
  //destructor, cleans up nicely
  ~Board();
  
  //display whatever the current state of the board is
  void output_board();
  
  //remember we checked this piece for moves
  //so we don't have to again
  void set_checked(int file, int rank);
  
  //determine whether we need to check the piece
  //returns true if we've already checked it; otherwise false
  bool checked(int file, int rank);
  
  //the piece at a given location
  _Piece *get_element(int file, int rank);
  
  //returns memory for a move structure for a piece
  //(remember to free this later)
  _Move *make_move(_Piece *p, int to_file, int to_rank);
  
  //a transformation to get to the next direction
  //returns 0 when there are none left
  int next_direction(int direction);
  
  //generate the legal moves for different pieces
  //these should be subsets of what's returned by legal_moves
  
  vector<_Move*> pawn_moves(_Piece *piece);
  vector<_Move*> rook_moves(_Piece *piece);
  vector<_Move*> knight_moves(_Piece *piece);
  vector<_Move*> bishop_moves(_Piece *piece);
  vector<_Move*> queen_moves(_Piece *piece);
  vector<_Move*> king_moves(_Piece *piece);
  
  //a vector of random moves that can be done by the piece in question
  vector<_Move*> legal_moves(Piece p);
};

#endif

