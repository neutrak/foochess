#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "Board.h"
using namespace std;

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
  //the history table this AI is using (NULL for none)
  HistTable *hist;
public:
  AI(Connection* c);
  virtual const char* username();
  virtual const char* password();
  //time-limited input (timeout is in seconds)
  //returns true when input is recieved, false otherwise
  bool tl_input(char *buffer, int buffer_size, int timeout);
  virtual void init();
  Board *board_from_master();
  _Move *user_move(Board *board);
  //make a move depending on the algorithm in use and the time left
  _Move *ai_move(Board *board, double time_remaining, double enemy_time_remaining);
  virtual bool run();
  virtual void end();
};

#endif
