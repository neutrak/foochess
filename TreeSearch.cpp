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
          post_move->apply_move(*i);
          
          //if the result of this move is our owner being in check
          if(post_move->in_check(p->owner))
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


//helper functions for depth-limited minimax
int dl_maxV(Board *node, int depth_limit, int player_id)
{
  
}

int dl_minV(Board *node, int depth_limit, int player_id)
{
  //TODO: generate children for the given node
  
  //if it's an end state return the known value
  if(node->get_children().empty())
  {
    //no legal moves for us, failure (return heuristic minimum)
    return HEURISTIC_MINIMUM;
  }
  //else if we hit the depth limit return the heuristic at this level
  //the < is defensive, == should work but just in case
  else if(depth_limit<=0)
  {
    return node->naive_points(player_id);
  }
  //else it's determined by the min player's actions, so make some more calls
  else
  {
    //TODO: go through all children and find the minimum of the maxV calls for those nodes
  }
  
  //defensive; for the moment this can't fall through but we never want to return uninitialized data (just in case)
  return HEURISTIC_MINIMUM;
}

//depth-limited minimax
_Move *dl_minimax(Board *root, int depth_limit, int player_id)
{
  printf("dl_minimax debug 0, got a board with %i children\n", root->get_children().size());
  
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
  int current_max=0;
  //initially the move that got us that desired max value is null
  _Move *max_move=NULL;
  
  //go through the frontier, find the max value of all children (which will be determined recursively)
  std::vector<Board*>::iterator i;
  for(i=frontier.begin(); i!=frontier.end(); i++)
  {
    //NOTE: this section depends on the heuristic used
    //get the heuristic value for this node (or better, if available; see dl_minV for more information)
//    int heuristic=dl_minV((*i), depth_limit-1, player_id);
    int heuristic=(*i)->naive_points(player_id);
    if(heuristic>current_max)
    {
      current_max=heuristic;
      max_move=(*i)->get_last_move_made();
    }
  }
  
  printf("dl_minimax debug 2, best move is from file, rank (%i,%i) to (%i,%i)\n", max_move->fromFile, max_move->fromRank, max_move->toFile, max_move->toRank);
  
  //make (read: return) the move that got us to the best position
  //(stored as Board::last_move_made in the child we chose earlier)
  //note that for memory management purposes this is a copy
  return root->copy_move(max_move);
}


