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
  
  //the piece at a given location
  _Piece *get_element(int file, int rank);
  
  //returns memory for a move structure for a piece
  //(remember to free this later)
  _Move *make_move(_Piece *p, int to_file, int to_rank);
  
  //a vector of random moves that can be done by the piece in question
  vector<_Move*> legal_moves(Piece p);
};

#endif

