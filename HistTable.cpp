#include "HistTable.h"

//a history table for ht_qs_tl_ab_id_dlmm, and anything else I want to use it for


//an integer by which to index a move in the history table's format
//NOTE: because this is a multimap hashes aren't necessarily unique
//however, they should be distributed about evenly
int HistTable::hash(_HistMove* hist_move)
{
  int hash=0;
  
  //DEFENSIVE: if a good hash can't be computed at least return /something/
  if(hist_move==NULL)
  {
    return 0;
  }
  
  //the from postion gets added to the hash, bit shifts for more unique hashes
  hash+=(hist_move->fromFile);
  hash+=(hist_move->fromRank) << 3;
  
  //the to position, bit shifts here for same reason
  hash+=(hist_move->toFile) << 6;
  hash+=(hist_move->toRank) << 9;
  
  //captures result in multiplcation with the captured type
  if(hist_move->capture)
  {
    hash*=(hist_move->capture_type);
  }
  
  //checks result in multiplication by 2
  if(hist_move->result_in_check)
  {
    hash*=2;
  }
  
  //return an identifier for this move which is /close to/ unique
  return hash;
}

//make a history table entry based on the post-move board state b
_HistMove *HistTable::make_hist_move(Board *b)
{
  _HistMove *hist_move=(_HistMove*)(malloc(sizeof(_HistMove)));
  if(hist_move==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)\n");
    exit(1);
  }
  
  _Move *m=b->get_last_move_made();
  
  //initially the history value is 0 (it can be incremented later)
  //(history value is how many times this has been returned from a dl_minimax or general_min_or_max_pruning call)
  hist_move->history_value=0;
  
  hist_move->fromFile=m->fromFile;
  hist_move->toFile=m->toFile;
  
  hist_move->fromRank=m->fromRank;
  hist_move->toRank=m->toRank;
  
  //until proven otherwise there are no captures or checks
  hist_move->capture=false;
  hist_move->result_in_check=false;
  
  //if the last move was a capture, store that correctly
  if(b->get_moves_since_capture()==0)
  {
    hist_move->capture=true;
    hist_move->capture_type=(b->get_last_capture_type());
  }
  
  _SuperPiece *moved_piece=b->get_element(m->toFile,m->toRank);
  
  //DEFENSIVE: moved_piece should never be NULL, since it was just moved
  if(moved_piece!=NULL)
  {
    //if this move put the enemy in check
    if(b->get_check(!(moved_piece->owner)))
    {
      hist_move->result_in_check=true;
    }
  }
  
  return hist_move;
}

//returns true if two history moves are equal, else false
bool HistTable::hist_eq(_HistMove *a, _HistMove *b)
{
  //verfiy any captures made are still valid
  if(a->capture && b->capture)
  {
    if(a->capture_type!=b->capture_type)
    {
      return false;
    }
  }
  
  //if we didn't discount equality earlier, then check everything else
  return ((a->fromFile == b->fromFile) && (a->fromRank == b->fromRank) && (a->toFile == b->toFile) && (a->toRank == b->toRank) && (a->result_in_check == b->result_in_check));
}

//constructor
HistTable::HistTable()
{
  history.clear();
}

//destructor (this will free everything in the history multimap)
HistTable::~HistTable()
{
  //for every entry in "history", free() that memory
  for(multimap<int, _HistMove *>::iterator i=history.begin(); i!=history.end(); i++)
  {
    free((*i).second);
  }
}

//increment a history table value or add the move to the history table if it's not there
void HistTable::increment_or_make(Board *b)
{
  _HistMove *hist_move=make_hist_move(b);
  int current_hash=hash(hist_move);
  
  pair<multimap<int, _HistMove *>::iterator, multimap<int, _HistMove *>::iterator> key_eq_range;
  
  //look through all history table entries with the same hash
  key_eq_range=history.equal_range(current_hash);
  for(multimap<int, _HistMove *>::iterator i=key_eq_range.first; i!=key_eq_range.second; i++)
  {
    //if the history moves are equal, increment the relevant entry
    if(hist_eq((*i).second, hist_move))
    {
      free(hist_move);
      ((*i).second)->history_value++;
      //save some clock cycles and save some ifs outside this loop
      return;
    }
  }
  
  //we got here and didn't return, this move isn't in the history table yet, so put it there and give it a 1 value
  hist_move->history_value=1;
  history.insert(pair<int, _HistMove *>(current_hash, hist_move));
}

//get the value of a history table entry
//if this move isn't in the history table, return 0
//NOTE: b is the board AFTER the relevant move has been applied
int HistTable::get_value(Board *b)
{
  _HistMove *hist_move=make_hist_move(b);
  int current_hash=hash(hist_move);
  
  pair<multimap<int, _HistMove *>::iterator, multimap<int, _HistMove *>::iterator> key_eq_range;
  
  //look through all history table entries with the same hash
  key_eq_range=history.equal_range(current_hash);
  for(multimap<int, _HistMove *>::iterator i=key_eq_range.first; i!=key_eq_range.second; i++)
  {
    //if the history moves are equal, return the value associated with that entry
    if(hist_eq((*i).second, hist_move))
    {
      free(hist_move);
      return ((*i).second)->history_value;
    }
  }
  
  free(hist_move);
  
  //if the entry was not found in the history table, it couldn't have been incremented yet
  return 0;
}



