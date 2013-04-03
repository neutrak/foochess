#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <cstdlib>
#include "Board.h"
using namespace std;

enum algorithm
{
  USER, //let the user actually play
  RANDOM, //random
  ID_DLMM, //iterative-deepening depth-limited minimax
  TL_AB_ID_DLMM, //time-limited alpha-beta pruned iterative-deepening depth-limited minimax
  
  ALGO_MAX
};

///The class implementing gameplay logic.
class AI: public BaseAI
{
private:
  //a master copy of the board that persists between moves
  Board *master;
public:
  AI(Connection* c);
  virtual const char* username();
  virtual const char* password();
  virtual void init();
  Board *board_from_master();
  _Move *user_move(Board *board);
  //make a move depending on the algorithm in use and the time left
  _Move *ai_move(Board *board, algorithm algo, double time_remaining);
  virtual bool run();
  virtual void end();
};

#endif
