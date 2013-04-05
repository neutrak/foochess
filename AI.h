#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "Board.h"
using namespace std;

enum algorithm
{
  USER, //let the user actually play
  RANDOM, //random
  ID_DLMM, //iterative-deepening depth-limited minimax
  TL_AB_ID_DLMM, //time-limited alpha-beta pruned iterative-deepening depth-limited minimax
  
  ALGO_COUNT
};

enum heuristic
{
  INFORMED_ATTACK, //informed_points base, weight capture of enemy pieces above preservation of own pieces
  INFORMED_DEFEND, //informed_points base, weight preservation of own pieces above capture of enemy pieces
  NAIVE_ATTACK, //naive_points base, weight capture more
  NAIVE_DEFEND, //naive_points base, weight preservation more
  
  HEURISTIC_COUNT
};

///The class implementing gameplay logic.
class AI: public BaseAI
{
private:
  //a master copy of the board that persists between moves
  Board *master;
  //the algorithm to use
  algorithm algo;
  //the heuristic to use (for the algorithms which are heuristic-based)
  heuristic heur;
public:
  AI(Connection* c);
  virtual const char* username();
  virtual const char* password();
  //time-limited input (timeout is in seconds)
  void tl_input(char *buffer, int buffer_size, int timeout);
  virtual void init();
  Board *board_from_master();
  _Move *user_move(Board *board);
  //make a move depending on the algorithm in use and the time left
  _Move *ai_move(Board *board, double time_remaining);
  virtual bool run();
  virtual void end();
};

#endif
