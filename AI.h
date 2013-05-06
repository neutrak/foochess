#ifndef AI_H
#define AI_H

#include <iostream>
#include <cstdlib>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include "Board.h"

#define BUFFER_SIZE 1024

using namespace std;

///The class implementing gameplay logic.
class AI
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
  
  //a move history
  vector <_Move*> moves;
public:
  AI();
  ~AI();
  
  //time-limited input (timeout is in seconds)
  //returns true when input is recieved, false otherwise
  bool tl_input(char *buffer, int buffer_size, int timeout);
  
  //set the internal algorithm to be used for movements
  void set_algo(algorithm a) { algo=a; }
  
  void init();
  
  //store this move in the movement history
  void remember_move(_Move *m);
  
  _Move *user_move(Board *board, int player_id);
  
  //make a move depending on the algorithm in use and the time left
  _Move *ai_move(Board *board, int player_id, double time_remaining, double enemy_time_remaining);
  
  bool run(Board *board, int player_id);
  
  void end();
};

#endif
