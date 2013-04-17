#include <stdio.h>
#include <stdlib.h>
#include "TreeSearch.h"

//returns a list of valid moves and creates associated children in board->children as a side-effect
vector <_Move*> TreeSearch::generate_moves(Board *board, int player_id)
{
  //the moves we'll return
  vector<_Move*> valid_moves;
  
  //now that we know where the king is, see if any enemy can attack it
  //file
  for(size_t f=1; f<=8; f++)
  {
    //rank
    for(size_t r=1; r<=8; r++)
    {
      //if this is an owned piece of any kind
      if(board->get_element(f,r)!=NULL && board->get_element(f,r)->owner==player_id)
      {
        //generate its moves
        _SuperPiece *p=board->get_element(f,r);
        vector<_Move*> our_moves=board->legal_moves(p);
        
        board->get_element(p->file, p->rank)->haveChecked=true;
        
        //add each move into the valid moves vector
        vector<_Move*>::iterator i;
        for(i=our_moves.begin(); i!=our_moves.end(); i++)
        {
          //verify that none end us in check, because in that case we can't make the move
          
          //copy the current board
          Board *post_move=new Board(board);
          
          //apply the given move
          post_move->apply_move(*i, true);
          
          //if the result of this move is our owner being in check
          if(post_move->get_check(p->owner))
          {
            //then it's not really a valid move
//            printf("generate_moves() Warn: We'll be in check after a move from (%i,%i) to (%i,%i)\n",(*i)->fromFile, (*i)->fromRank, (*i)->toFile, (*i)->toRank);
            
            //remove this move from the vector to return
            i=our_moves.erase(i);
            //the -- is so the ++ in the for loop kicks us back to the proper next element
            i--;
            
            //note the move *i is freed within the destructor for post_move (which got a reference from apply_move above)
            delete post_move;
          }
          else
          {
//            printf("generate_moves(); generated a move from File, Rank (%i,%i) to (%i,%i)\n",(*i)->fromFile, (*i)->fromRank, (*i)->toFile, (*i)->toRank);
            valid_moves.push_back(*i);
            
            //make a tree structure
            board->add_child(post_move);
          }
        }
      }
    }
  }
  
  return valid_moves;
}

//returns true if there is a stalemate caused by repeated moves
//else false
bool TreeSearch::stalemate_by_repeat(vector <_Move*> move_accumulator)
{
  //if there weren't enough moves to /possibly/ cause a problem
  if(move_accumulator.size()<8)
  {
    //there's no problem!
    return false;
  }
  
  for(int i=0; i<4; i++)
  {
    _Move *early_move=move_accumulator[move_accumulator.size()-5-i];
    _Move *later_move=move_accumulator[move_accumulator.size()-1-i];
    
    //if the moves didn't start in the same spot
    if( !((early_move->fromFile == later_move->fromFile) && (early_move->fromRank == later_move->fromRank)) )
    {
      return false;
    }
    //else if the moves didn't end in the same spot
    else if( !((early_move->toFile == later_move->toFile) && (early_move->toRank == later_move->toRank)) )
    {
      return false;
    }
    //else this pair of moves /is/ equal, try the next pair
  }
  
  //if we didn't find any move for which there was not a repeat
  //then this /is/ a stalemate by repeat
  return true;
}

//returns true if there is insufficient material to checkmate
//otherwise false
bool TreeSearch::insufficient_material(Board *board, int player_id)
{
  //a count of how many of each piece each player has
  int our_pieces[PIECE_MAX];
  int enemy_pieces[PIECE_MAX];
  
  //the color of the last bishop found for each player; white is 0, black is 1
  int enemy_bishop_color=0;
  int owned_bishop_color=0;
  
  //initialize to 0
  for(int i=0; i<PIECE_MAX; i++)
  {
    our_pieces[i]=0;
    enemy_pieces[i]=0;
  }
  
  int width=8;
  int height=8;
  
  //find pieces, make counts!
  for(int f=1; f<=width; f++)
  {
    for(int r=1; r<=height; r++)
    {
      if(board->get_element(f,r)!=NULL)
      {
        //where to index in to the chosen array (our's or the enemy's)
        int type_index=-1;
        
        //increment depending on type (the returns in here save us some clock cycles)
        switch(board->get_element(f,r)->type)
        {
          case 'P':
            //if either side has a pawn there is "sufficient material"
//            type_index=PAWN;
            return false;
            break;
          case 'R':
            //same for rooks; a rook and a king is enough to checkmate
//            type_index=ROOK;
            return false;
            break;
          case 'N':
            type_index=KNIGHT;
            break;
          case 'B':
            type_index=BISHOP;
            {
              //if the sum of file and rank is even, this is a white square; else this is a black square
              int color=(f+r)%2;
              if(board->get_element(f,r)->owner==player_id)
              {
                owned_bishop_color=color;
              }
              else
              {
                enemy_bishop_color=color;
              }
            }
            break;
          case 'Q':
            //same for queens; a queen and a king is enough to checkmate
//            type_index=QUEEN;
            return false;
            break;
          case 'K':
            type_index=KING;
            break;
        }
        
        if(board->get_element(f,r)->owner==player_id)
        {
          our_pieces[type_index]++;
        }
        else
        {
          enemy_pieces[type_index]++;
        }
      }
    }
  }
  
  //insufficient material conditions
  //only kings left
  if(our_pieces[BISHOP]==0 && our_pieces[KNIGHT]==0 && enemy_pieces[BISHOP]==0 && enemy_pieces[KNIGHT]==0)
  {
    return true;
  }
  //if we have nothing but a king and the enemy has nothing but a single bishop
  else if(our_pieces[BISHOP]==0 && our_pieces[KNIGHT]==0 && enemy_pieces[BISHOP]==1 && enemy_pieces[KNIGHT]==0)
  {
    return true;
  }
  //same as above but for enemy and us reversed
  else if(our_pieces[BISHOP]==1 && our_pieces[KNIGHT]==0 && enemy_pieces[BISHOP]==0 && enemy_pieces[KNIGHT]==0)
  {
    return true;
  }
  //a single knight, enemy-owned
  else if(our_pieces[BISHOP]==0 && our_pieces[KNIGHT]==0 && enemy_pieces[BISHOP]==0 && enemy_pieces[KNIGHT]==1)
  {
    return true;
  }
  //a single knight, player-owned
  else if(our_pieces[BISHOP]==0 && our_pieces[KNIGHT]==1 && enemy_pieces[BISHOP]==0 && enemy_pieces[KNIGHT]==0)
  {
    return true;
  }
  //one bishop per player
  else if(our_pieces[BISHOP]==1 && our_pieces[KNIGHT]==0 && enemy_pieces[BISHOP]==1 && enemy_pieces[KNIGHT]==0)
  {
    //if they're on the same color, it's a stalemate
    if(owned_bishop_color==enemy_bishop_color)
    {
      return true;
    }
  }
  
  //if we got through that and didn't return then there is sufficient material; return as such
  return false;
}

//free the memory referenced by a move accumulator vector
void TreeSearch::free_move_acc(vector <_Move*> move_accumulator)
{
  for(size_t i=0; i<move_accumulator.size(); i++)
  {
    free(move_accumulator[i]);
  }
}

//the heuristics we'll be using for minimax

double TreeSearch::informed_danger_heuristic(Board *node, int player_id, bool max)
{
  //player id of max player
  int pid=max? player_id : !player_id;
  
  return (node->points(pid,true,true)*0.75)-(node->points(!pid,true,true)); //keep ourselves in at least as good a point position as the enemy and a better piece position
}

double TreeSearch::informed_attack_heuristic(Board *node, int player_id, bool max)
{
  //a local player id
  //the heuristic is always calculated with respect to the max player
  //if the max player is not at move, calculate with respect to it anyway
  int pid=max? player_id : !player_id;
  
//  return node->points(pid,true,true); //keep ourselves alive above all else
//  return ((node->points(pid,true,true))-(node->points(!pid,true,true))); //make us have a higher score than the enemy above all else
//  return -(node->points(!pid,true,true)); //kill the enemy above all else
  return (node->points(pid,true,false)*0.7)-(node->points(!pid,true,false)); //kill the enemy but don't sacrifice everything to accomplish that
}

double TreeSearch::informed_defend_heuristic(Board *node, int player_id, bool max)
{
  //player id of max player
  int pid=max? player_id : !player_id;
  
  return (node->points(pid,true,false))-(node->points(!pid,true,false)*0.7); //defend ourselves first but kill the enemy where it's convienent
}

double TreeSearch::naive_attack_heuristic(Board *node, int player_id, bool max)
{
  //player id of max player
  int pid=max? player_id : !player_id;
  
  return (node->points(pid,false,false)*0.7)-(node->points(!pid,false,false)); //kill the enemy but don't sacrifice everything to accomplis that
}

double TreeSearch::naive_defend_heuristic(Board *node, int player_id, bool max)
{
  //player id of max player
  int pid=max? player_id : !player_id;
  
  return (node->points(pid,false,false))-(node->points(!pid,false,false)*0.7); //defend ourselves first but kill the enemy where it's convienent
}

//how much time to allocate to this move given the board and how much time we have left
double TreeSearch::time_for_this_move(Board *board, int player_id, double time_remaining, double enemy_time_remaining, int moves_made)
{
  //give us a 5% margin of error right away, just in case (defensively)
  //this means we will under-estimate the time we think we have so we don't accidentally time out
  time_remaining*=0.95;
  
  //if we have fewer than 15 seconds left, just allocate a quarter second per move no matter what, that should guarantee completion of at least 1 iteration, probably 2
  if(time_remaining<15)
  {
    return 0.25;
  }
  
  double time_for_move;
  
  //how many moves we think will be after this one; we'll revise it in a second
  double moves_remaining=50;
  
  //if we're losing and can afford some time, allocate more time to this move, in the hopes of catching up
  //this naive_points heuristic is too simple for actual game use, but serves well enough here
  if((time_remaining>enemy_time_remaining) && (board->points(player_id,false,false) > board->points(!player_id,false,false)))
  {
    //really look forward this move
    //if this uses too much time
    //then on the next move we won't be ahead in time and the above condition won't trigger
    moves_remaining/=1.5;
  }
  
  //distribute time evenly according to how many moves we think we have left to make
  time_for_move=time_remaining/moves_remaining;
  
  return time_for_move;
}

//make a random [legal] move
_Move *TreeSearch::random_move(Board *board, int player_id)
{
  //pick a piece and a valid move set
  vector<_Move*> valid_moves=generate_moves(board, player_id);
  
  //if we checked all pieces and none had valid moves
  if(valid_moves.empty())
  {
    printf("random_move() debug 0, checkmate!?\n");
    return NULL;
  }
  //make a random move of the legal moves generated earlier
  int rand_move_index=rand()%(valid_moves.size());
  
  _Move* move=NULL;
  
  move=board->copy_move(valid_moves[rand_move_index]);
  
  board->clear_children();
  return move;
}

//helper functions for depth-limited minimax

//this serves the functions of dl_maxV, dl_minV, abdl_maxV, and abdl_minV
//those functions themselves just carefully choose the arguments to give to this
//max should be true to max, false to min
//prune should be true for pruning, false for not; alpha and beta are ignored when prune is false
double TreeSearch::min_or_max(Board *node, int depth_limit, int qs_depth_limit, int player_id, bool max, heuristic heur, bool prune, double alpha, double beta, vector<_Move*> move_accumulator, bool time_limit, HistTable *hist, double time_for_move, double time_used)
{
  //NOTE: we can't do the terminal node checks before the generate_moves call
  //because whether it's a terminal node or not depends on move generation
  
  //generate children for the given node
  generate_moves(node,player_id);
  
  if(hist==NULL)
  {
    node->shuffle_children();
  }
  //when a history table is being used, order children by history table values
  else
  {
    node->shuffle_children();
    node->history_order_children(hist);
  }
  
  //NOTE: only the player at move can be in checkmate
  //if we are in checkmate, return heuristic minimum (for max player)
  if(node->get_check(player_id) && node->get_children().empty())
  {
    free_move_acc(move_accumulator);
    //worst case for this player
    return max? HEURISTIC_MINIMUM : HEURISTIC_MAXIMUM;
  }
  //if it's a stalemate
  else if((node->get_children().empty()) || (node->get_moves_since_capture()>=8 && node->get_moves_since_advancement()>=8 && stalemate_by_repeat(move_accumulator)) || (node->get_moves_since_capture()>=100 && node->get_moves_since_advancement()>=100) || insufficient_material(node,player_id))
  {
    free_move_acc(move_accumulator);
    //this isn't great, but it's not that bad either
//    return AVG(HEURISTIC_MINIMUM,HEURISTIC_MAXIMUM);
    return -15.0;
  }
  //if we hit the depth limit, use the heuristic
  //if this is a quiescent state or we've hit the quiscent search depth limit also
  else if(depth_limit<=0 && (qs_depth_limit<=0 || node->quiescent()))
  {
    free_move_acc(move_accumulator);
    //NOTE: no breaks are needed because every case returns
    switch(heur)
    {
      case INFORMED_DANGER:
        return informed_danger_heuristic(node,player_id,max);
      case INFORMED_ATTACK:
        return informed_attack_heuristic(node,player_id,max);
      case INFORMED_DEFEND:
        return informed_defend_heuristic(node,player_id,max);
      case NAIVE_ATTACK:
        return naive_attack_heuristic(node,player_id,max);
      case NAIVE_DEFEND:
        return naive_defend_heuristic(node,player_id,max);
      default:
        return informed_attack_heuristic(node,player_id,max);
    }
  }
  //if we hit the first depth limit but not the quiescent one
  else if(depth_limit<=0)
  {
    //decrement the quiescent limit
    qs_depth_limit--;
  }
  
  //if we got through that and didn't return it's determined by the other player's actions, so make some more calls
  //go through all children and find the best assuming the opponent makes good choices
  double best=(max)? HEURISTIC_MINIMUM : HEURISTIC_MAXIMUM;
  //the child that gives us the best heuristic value (so we can increment its history value)
  size_t best_child=0;
  
  for(size_t i=0; i<(node->get_children().size()); i++)
  {
    //make a new move accumulator to pass to the recursive call
    vector <_Move*> new_move_acc;
    for(size_t n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(node->copy_move(move_accumulator[n]));
    }
    //add on the move made to get to this child
    new_move_acc.push_back(node->copy_move(node->get_children()[i]->get_last_move_made()));
    
    //time the recursive calls so we can stop early if we run out of time
    struct timeval start_time;
    gettimeofday(&start_time,NULL);
    
    //NOTE: on the recursive calls we generate the moves for the /other/ player
    double opponent_move=min_or_max(node->get_children()[i], depth_limit-1, qs_depth_limit, !player_id, !max, heur, prune, alpha, beta, new_move_acc, time_limit, hist, time_for_move, time_used);
    
    struct timeval end_time;
    gettimeofday(&end_time,NULL);
    
    //compute the difference between start and end times and add that to the time used
    double before=start_time.tv_sec+(start_time.tv_usec/1000000.0);
    double after=end_time.tv_sec+(end_time.tv_usec/1000000.0);
    time_used+=(after-before);
    
    //if we're out of time, return NULL (as an error code) and clean up memory
    if((opponent_move==OUT_OF_TIME) || (time_limit && (time_used>=time_for_move)))
    {
//      printf("min_or_max debug 2, OUT OF TIME, returning early\n");
      best=OUT_OF_TIME;
      break;
    }
    
    //when prune is true, if we got a fail low on min or fail high on max, prune
    if(prune)
    {
      //fail low for min player or fail high for max player
      if(((opponent_move<=alpha) && (!max)) || ((opponent_move>=beta) && (max)))
      {
        best_child=i;
        //return the fail up so that the other recursion levels can handle it accordingly
        best=opponent_move;
//        printf("min_or_max debug 1, pruning a %lf with bounds (%lf,%lf)\n", opponent_move, alpha, beta);
        //breaking this loop early means we won't check any more children at this level, thus "pruning" the remaining iterations/children
        break;
      }
    }
    
    //if this move is better than the current best, it's the new best
    if((max && opponent_move>best) || (!max && opponent_move<best))
    {
      best_child=i;
      best=opponent_move;
      
      //NOTE: if prune is not true, these values will still be set, but they won't be used for anything
      //if I'm the max player and I have some best value, I will no longer accept anything lower than that
      if(max)
      {
        alpha=best;
      }
      //if I'm the min player and I have some best value, I will no longer accept anything higher than that
      else
      {
        beta=best;
      }
    }
  }
  
  //update the relevant history table entry before we return
  if(hist!=NULL && best!=OUT_OF_TIME)
  {
    hist->increment_or_make(node->get_children()[best_child]);
  }
  
  //manage memory; we won't need this any more
  node->clear_children();
  
  free_move_acc(move_accumulator);
  
  //return the best of all the worst from recursion (simulating the other player implicitly)
  return best;
}

//depth-limited minimax
_Move *TreeSearch::dl_minimax(Board *root, int depth_limit, int qs_depth_limit, int player_id, vector<_Move*> move_accumulator, heuristic heur, bool prune, bool time_limit, HistTable *hist, double time_for_move, double time_used)
{
//  printf("dl_minimax debug 0, got a board with %i children\n", root->get_children().size());
  
  double alpha=HEURISTIC_MINIMUM;
  double beta=HEURISTIC_MAXIMUM;
  
  //the tree-search frontier
  std::vector<Board*> frontier;
  
  //first generate the children of the given board state
  //this is done as a side-effect of move generation
  generate_moves(root, player_id);
  
  if(hist==NULL)
  {
    //board->shuffle_children randomizes children (created by applying moves)
    //so nodes with equal heuristic values don't always get taken in the same order
    root->shuffle_children();
  }
  //when a history table is being used, order children by history table values
  else
  {
    root->shuffle_children();
    root->history_order_children(hist);
  }
  
//  printf("dl_minimax debug 1, now have a board with %i children\n", root->get_children().size());
  
  //the first player is always max-ing
  //initially current_max is the lowest possible heuristic value
  //NOTE: change this as needed to reflect the lowest possible heuristic value
  double current_max=HEURISTIC_MINIMUM;
  //initially the move that got us that desired max value is null
  _Move *max_move=NULL;
  size_t best_child=0;
  
  //find the max value of all children (which will be determined recursively)
  for(size_t i=0; i<root->get_children().size(); i++)
  {
    //make a new move accumulator to pass to the recursive call
    vector <_Move*> new_move_acc;
    for(size_t n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(root->copy_move(move_accumulator[n]));
    }
    //add on the move made to get to this child
    new_move_acc.push_back(root->copy_move(root->get_children()[i]->get_last_move_made()));
    
    //time the recursive calls so we can stop early if we run out of time
    struct timeval start_time;
    gettimeofday(&start_time,NULL);
    
    //NOTE: this section depends on the heuristic used
    //get the heuristic value for this node (or better, if available; see dl_minV for more information)
    
    //this is a dl_minV call, using a more general function
    double heuristic=min_or_max(root->get_children()[i], depth_limit-1, qs_depth_limit, !player_id, false, heur, prune, alpha, beta, new_move_acc, time_limit, hist, time_for_move, time_used);
    
    struct timeval end_time;
    gettimeofday(&end_time,NULL);
    
    double before=start_time.tv_sec+(start_time.tv_usec/1000000.0);
    double after=end_time.tv_sec+(end_time.tv_usec/1000000.0);
    time_used+=(after-before);
    
    //if we're out of time, return NULL (as an error code) and clean up memory
    if((heuristic==OUT_OF_TIME) || (time_limit && (time_used>=time_for_move)))
    {
      current_max=OUT_OF_TIME;
//      printf("dl_minimax debug 1.5, OUT OF TIME, returning early\n");
      free(max_move);
      max_move=NULL;
      break;
    }
    
    //if we don't have a move yet take this one regardless of heuristic
    if(heuristic>current_max || max_move==NULL)
    {
      best_child=i;
      current_max=heuristic;
      
      //as the max player we will no longer accept anything worse than what we just got
      //in the no-pruning case this value gets ignored, but it doesn't hurt to set it anyway
      alpha=current_max;
      
      //we won't be using the old move so free it
      if(max_move!=NULL)
      {
        free(max_move);
      }
      
      max_move=root->copy_move(root->get_children()[i]->get_last_move_made());
    }
  }
  
  //update the relevant history table entry before we return
  if(hist!=NULL && current_max!=OUT_OF_TIME)
  {
    hist->increment_or_make(root->get_children()[best_child]);
  }
  
  //clean up memory from those recursive calls
  root->clear_children();
  
  if(max_move!=NULL)
  {
//    printf("dl_minimax debug 2, best move is from file, rank (%i,%i) to (%i,%i) (heuristic value %lf)\n", max_move->fromFile, max_move->fromRank, max_move->toFile, max_move->toRank, current_max);
  }
  
  free_move_acc(move_accumulator);
  
  //make (read: return) the move that got us to the best position
  //(stored as Board::last_move_made in the child we chose earlier)
  //note that for memory management purposes this is a copy (made when setting max_move)
  return max_move;
}

//iterative deepening depth-limited minimax with an option to time-limit instead of using a given max depth
_Move *TreeSearch::id_minimax(Board *root, int max_depth_limit, int qs_depth_limit, int player_id, vector<_Move*> move_accumulator, heuristic heur, bool prune, bool time_limit, HistTable *hist, double time_remaining, double enemy_time_remaining)
{
  _Move *end_move=NULL;
  
  //how much time we have used so far
  double time_used=0;
  //how much time to allocate for this move
  double time_for_move=time_for_this_move(root,player_id,time_remaining,enemy_time_remaining,move_accumulator.size());
//  printf("id_minimax debug 0, allocating %lf seconds to this move\n",time_for_move);
  
  //the <= here is so max_depth_limit is inclusive
  //in the case we're doing a time-limited version of this we don't want to stop on max depth limit
  for(int depth_limit=1; time_limit || (depth_limit<=max_depth_limit); depth_limit++)
  {
//    printf("id_minimax debug 1, getting a move from dl_minimax with depth limit %i, prune is %s\n", depth_limit, prune? "True" : "False");
    
    //make a new move accumulator to pass to the depth-limited call
    vector <_Move*> new_move_acc;
    for(size_t n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(root->copy_move(move_accumulator[n]));
    }
    
    //track time usage on this call by storing the start and end times
    struct timeval start_time;
    gettimeofday(&start_time,NULL);
    
    _Move *old_move=end_move;
    
    //NOTE: when not using a history table, hist will be NULL
    end_move=dl_minimax(root, depth_limit, qs_depth_limit, player_id, new_move_acc, heur, prune, time_limit, hist, time_for_move, time_used);
    
    //if a new move was successfully generated
    if(end_move!=NULL)
    {
      //free the memory from before
      free(old_move);
    }
    //otherwise make use of the previously generated move
    else
    {
      end_move=old_move;
    }
    
    struct timeval end_time;
    gettimeofday(&end_time,NULL);
    
    //compute the difference between start and end times and add that to the time used
    double before=start_time.tv_sec+(start_time.tv_usec/1000000.0);
    double after=end_time.tv_sec+(end_time.tv_usec/1000000.0);
    time_used+=(after-before);
    
//    printf("id_minimax debug 2, time used for this move so far is %lf seconds\n",time_used);
    
    //estimate the time that will be required for the next iteration
    //and take into account whether we will possibly be able to finish another iteration
    //if we don't think there's any real chance we can finish it don't bother starting it; this will save us some valuable play time for later
    
    //NOTE: this estimate is based on O(b^d) being the time complexity; thus b*(O(b^(d-1))) is used to compute it, figuring 2 as a VERY optimistic branch factor
    double time_for_next=2*(after-before);
    
/*
    //the next iteration will almost always take at least as long as this iteration did, since it'll have to check all those nodes, and probably many additional ones
    //this assumes no crazy amazing pruning benefits happen
    double time_for_next=after-before;
*/
    
    //time limit stop loop condition (since we're not using the loop condition in the for statement)
    if(time_limit && ((time_used+time_for_next)>=time_for_move))
    {
      break;
    }
    
    //if this move is a guaranteed checkmate, then don't bother trying any more
    if(end_move!=NULL)
    {
      Board *b=new Board(root);
      b->apply_move(b->copy_move(end_move),true);
      if(b->get_check(!player_id))
      {
        generate_moves(b,!player_id);
        if(b->get_children().empty())
        {
          delete b;
          break;
        }
      }
      delete b;
    }
  }
  
  free_move_acc(move_accumulator);
  
  return end_move;
}


