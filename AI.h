#ifndef AI_H
#define AI_H

#include "BaseAI.h"
#include <iostream>
#include <cstdlib>
#include "Board.h"
using namespace std;

//if we have this number of seconds or fewer left on the clock just use random rather than timing out
#define RANDOM_FALLBACK_TIME 8.0

enum algorithm
{
  RANDOM,
  ID_DLMM,
  
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
  virtual bool run();
  virtual void end();
};

#endif
