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
  
public:
  //constructor, makes board internal structures based off of the data I'm given
  Board(vector<Piece> pieces);
  
  //destructor, cleans up nicely
  ~Board();
  
  //display whatever the current state of the board is
  void output_board();
  
  //the piece at a given location
  _Piece *get_element(int file, int rank);
};

#endif

