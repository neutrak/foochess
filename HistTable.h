#ifndef HISTTABLE_H
#define HISTTABLE_H
#include <map>
#include <stdlib.h>
#include <stdio.h>
#include "Board.h"
using namespace std;

class Board;
struct _HistMove;

//a structure to identify a move
//two moves are the same iff
//  start and end position are the same
//  captures or lack thereof are the same
//  whether they result in a check or not is the same
struct _HistMove
{
  int fromFile;
  int fromRank;
  int toFile;
  int toRank;
  
  bool capture;
  int capture_type;
  bool result_in_check;
  
  int history_value;
};

//a history table for ht_qs_tl_ab_id_dlmm, and anything else I want to use it for
class HistTable
{
private:
  multimap<int, _HistMove *> history;
  
  //an integer by which to index a move in the history table's format
  //NOTE: because this is a multimap hashes aren't necessarily unique
  //however, they should be distributed about evenly
  int hash(_HistMove* hist_move);
  
  //convert a move and board to a history table move
  //(a move alone is not enough, since it doesn't include e.g. capture information)
  //NOTE: b is the board AFTER the relevant move has been applied
  _HistMove *make_hist_move(Board *b);
  
  //returns true if two history moves are equal, else false
  bool hist_eq(_HistMove *a, _HistMove *b);
  
public:
  //constructor
  HistTable();
  //destructor (this will free everything in the history multimap)
  ~HistTable();
  
  //increment a history table value or add the move to the history table if it's not there
  //NOTE: b is the board AFTER the relevant move has been applied
  void increment_or_make(Board *b);
  
  //get the value of a history table entry
  //if this move isn't in the history table, return 0
  //NOTE: b is the board AFTER the relevant move has been applied
  int get_value(Board *b);
};

#endif

