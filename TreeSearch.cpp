#include <stdio.h>
#include <stdlib.h>
#include "TreeSearch.h"

//returns a list of valid moves and creates associated children in board->children as a side-effect
vector <_Move*> generate_moves(Board *board, int player_id)
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
  
  //board->shuffle_children randomizes children (created by applying moves)
  //so nodes with equal heuristic values don't always get taken in the same order
  board->shuffle_children();
  
  return valid_moves;
}

//returns true if there is a stalemate caused by repeated moves
//else false
bool stalemate_by_repeat(vector <_Move*> move_accumulator)
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

//free the memory referenced by a move accumulator vector
void free_move_acc(vector <_Move*> move_accumulator)
{
  for(int i=0; i<move_accumulator.size(); i++)
  {
    free(move_accumulator[i]);
  }
}

//make a random [legal] move
_Move *random_move(Board *board, int player_id)
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
//TODO: add pruning handling to this when prune is true
double general_min_or_max_pruning(Board *node, int depth_limit, int player_id, bool max, bool prune, double alpha, double beta, vector<_Move*> move_accumulator)
{
//  printf("general_min_or_max_pruning debug 0, generating for player %i, depth limit %i\n", player_id, depth_limit);
  
  //note: we can't do the terminal node checks before the generate_moves call
  //because whether it's a terminal node or not dependson move generation
  
  //generate children for the given node
  generate_moves(node,player_id);
  
  //if it's an end state because we have no legal moves (draw) return the known value
  if(node->get_children().empty())
  {
    //NOTE: there's no need to do a check here since we return the same in either case
/*
    if(node->get_check(player_id))
    {
      //checkmate where we lose
    }
    else
    {
      //stalemate due to no valid moves
    }
*/
    
    free_move_acc(move_accumulator);
    //no legal moves for us, failure (return worst case for this player)
//    return (max)? HEURISTIC_MINIMUM : HEURISTIC_MAXIMUM;
    return HEURISTIC_MINIMUM;
  }
  //a checkmate occurs when a player is in check and has no legal moves, so check for that
  //in that case it's VERY VERY GOOD for this player (otherwise fall through and recurse)
  else if(node->get_check(!player_id))
  {
    Board *b=new Board(node);
    generate_moves(b,!player_id);
    
    if(b->get_children().empty())
    {
      delete b;
      free_move_acc(move_accumulator);
//      return (max)? HEURISTIC_MAXIMUM : HEURISTIC_MINIMUM;
      return HEURISTIC_MAXIMUM;
    }
    delete b;
  }
  //else if we've repeated moves in a bad way, it's a stalemate by repeat
  //return with a worst-case value
  else if(stalemate_by_repeat(move_accumulator))
  {
    free_move_acc(move_accumulator);
    return HEURISTIC_MINIMUM;
  }
  //else if we hit the depth limit return the heuristic at this level
  //the < is defensive, == should work but just in case
  else if(depth_limit<=0)
  {
    //a local player id, since for min player we want to flip this back for the purposes of heuristic value calcualtion
    int pid=player_id;
    if(!max)
    {
      pid!=pid;
    }
    
    free_move_acc(move_accumulator);
    
//    return node->naive_points(pid); //keep ourselves alive above all else
//    return ((node->naive_points(pid))-(node->naive_points(!pid))); //make us have a higher score than the enemy above all else
//    return -(node->naive_points(!pid)); //kill the enemy above all else
    return (node->naive_points(pid)*0.6)-(node->naive_points(!pid)); //kill the enemy but don't sacrifice everyting to accomplish that
  }
  
  //if we got through that and didn't return it's determined by the other player's actions, so make some more calls
  //go through all children and find the best assuming the opponent makes good choices
  double best=(max)? HEURISTIC_MINIMUM : HEURISTIC_MAXIMUM;
  
  for(int i=0; i<(node->get_children().size()); i++)
  {
    //make a new move accumulator to pass to the recursive call
    vector <_Move*> new_move_acc;
    for(int n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(node->copy_move(move_accumulator[n]));
    }
    //add on the move made to get to this child
    new_move_acc.push_back(node->copy_move(node->get_children()[i]->get_last_move_made()));
    
    //NOTE: on the recursive calls we generate the moves for the /other/ player
    double opponent_move=general_min_or_max_pruning(node->get_children()[i], depth_limit-1, !player_id, !max, prune, alpha, beta, new_move_acc);
    
    //if this move is better than the current best, it's the new best
    if((max && opponent_move>best) || (!max && opponent_move<best))
    {
      best=opponent_move;
    }
  }
  //manage memory; we won't need this any more
  node->clear_children();
  
  free_move_acc(move_accumulator);
  
  //return the best of all the worst from recursion (simulating the other player implicitly)
  return best;
}

double dl_maxV(Board *node, int depth_limit, int player_id, vector<_Move*> move_accumulator)
{
  return general_min_or_max_pruning(node, depth_limit, player_id, true, false, 0, 0, move_accumulator);
}

double dl_minV(Board *node, int depth_limit, int player_id, vector<_Move*> move_accumulator)
{
  return general_min_or_max_pruning(node, depth_limit, player_id, false, false, 0, 0, move_accumulator);
}

//depth-limited minimax
_Move *dl_minimax(Board *root, int depth_limit, int player_id, vector<_Move*> move_accumulator)
{
//  printf("dl_minimax debug 0, got a board with %i children\n", root->get_children().size());
  
  //the tree-search frontier
  std::vector<Board*> frontier;
  
  //first generate the children of the given board state
  //this is done as a side-effect of move generation
  generate_moves(root, player_id);
  
  printf("dl_minimax debug 1, now have a board with %i children\n", root->get_children().size());
  
  //initially, this is all the children of the root board node
  for(int i=0; i<(root->get_children()).size(); i++)
  {
    frontier.push_back(root->get_children()[i]);
  }
  
  //the first player is always max-ing
  //initially current_max is the lowest possible heuristic value
  //NOTE: change this as needed to reflect the lowest possible heuristic value
  double current_max=HEURISTIC_MINIMUM;
  //initially the move that got us that desired max value is null
  _Move *max_move=NULL;
  
  //go through the frontier, find the max value of all children (which will be determined recursively)
  for(int i=0; i<frontier.size(); i++)
  {
    //make a new move accumulator to pass to the recursive call
    vector <_Move*> new_move_acc;
    for(int n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(root->copy_move(move_accumulator[n]));
    }
    //add on the move made to get to this child
    new_move_acc.push_back(root->copy_move(root->get_children()[i]->get_last_move_made()));
    
    //NOTE: this section depends on the heuristic used
    //get the heuristic value for this node (or better, if available; see dl_minV for more information)
    double heuristic=dl_minV(root->get_children()[i], depth_limit-1, player_id, new_move_acc);
    
    //if we don't have a move yet take this one regardless of heuristic
    if(heuristic>current_max || max_move==NULL)
    {
      current_max=heuristic;
      
      //we won't be using the old move so free it
      if(max_move!=NULL)
      {
        free(max_move);
      }
      
      max_move=root->copy_move(root->get_children()[i]->get_last_move_made());
    }
  }
  
  //clean up memory from those recursive calls
  root->clear_children();
  
  if(max_move!=NULL)
  {
    printf("dl_minimax debug 2, best move is from file, rank (%i,%i) to (%i,%i) (heuristic value %lf)\n", max_move->fromFile, max_move->fromRank, max_move->toFile, max_move->toRank, current_max);
  }
  
  free_move_acc(move_accumulator);
  
  //make (read: return) the move that got us to the best position
  //(stored as Board::last_move_made in the child we chose earlier)
  //note that for memory management purposes this is a copy (made when setting max_move)
  return max_move;
}

//iterative deepening depth-limited minimax
_Move *id_minimax(Board *root, int max_depth_limit, int player_id, vector<_Move*> move_accumulator)
{
  _Move *end_move=NULL;
  
  //the <= here is so max_depth_limit is inclusive
  for(int depth_limit=0; depth_limit<=max_depth_limit; depth_limit++)
  {
    //if this move will supercede an older iteration
    //free the memory from before
    if(end_move!=NULL)
    {
      free(end_move);
    }
    
    printf("id_minimax debug 0, getting a move from dl_minimax with depth limit %i\n",depth_limit);
    
    //make a new move accumulator to pass to the depth-limited call
    vector <_Move*> new_move_acc;
    for(int n=0; n<move_accumulator.size(); n++)
    {
      new_move_acc.push_back(root->copy_move(move_accumulator[n]));
    }
    
    end_move=dl_minimax(root, depth_limit, player_id, new_move_acc);
  }
  
  free_move_acc(move_accumulator);
  
  return end_move;
}

