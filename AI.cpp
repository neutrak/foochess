#include "AI.h"
#include "util.h"
#include "Board.h"

AI::AI(Connection* conn) : BaseAI(conn) {}

const char* AI::username()
{
  return "Shell AI";
}

const char* AI::password()
{
  return "password";
}

//This function is run once, before your first turn.
void AI::init()
{
  srand(time(NULL));
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
  //put the relevant data in a slightly more sane structure
  //to get constant time lookups for some things that may otherwise be O(n^2)
  Board *board=new Board(pieces,NULL);
  
  // Print out the current board state
  board->output_board();
  
  //first, get a list of the pieces we're allowed to move
  //can't make a move if we don't know possible pieces
  vector<Piece> owned_pieces;
  vector<Piece> enemy_pieces;
  
  vector<Piece>::iterator i;
  for(i=pieces.begin(); i!=pieces.end(); i++)
  {
    if((*i).owner() == playerID())
    {
      owned_pieces.push_back(*i);
    }
    else
    {
      enemy_pieces.push_back(*i);
    }
  }
  
  // Looks through information about the players
  for(size_t p=0; p<players.size(); p++)
  {
    cout<<players[p].playerName();
    // if playerID is 0, you're white, if its 1, you're black
    if(players[p].id() == playerID())
    {
      cout<<" (ME)";
    }
    cout<<" time remaining: "<<players[p].time()<<endl;
  }

  // if there has been a move, print the most recent move
  if(moves.size() > 0)
  {
    cout<<"Last Move Was: "<<endl<<moves[0]<<endl;
  }
  
  vector<_Move*> valid_moves;
  int rand_index;
  
  //a running count of the number of pieces we've tried to check
  //when it gets through all of them, give up
  int checked_pieces=0;
  
  //while there are no legal moves for the selected piece
  while(valid_moves.empty())
  {
    if(checked_pieces==owned_pieces.size())
    {
      break;
    }
    
    //of the owned pieces, select one at random
    rand_index=rand()%(owned_pieces.size());
    
    //if we haven't already checked this piece
    if(!(board->checked(owned_pieces[rand_index].file(), owned_pieces[rand_index].rank())))
    {
      //first, map onto the internal data structure
      _Piece *piece=board->get_element(owned_pieces[rand_index].file(),owned_pieces[rand_index].rank());
      
      //then generate some moves
      valid_moves=board->legal_moves(piece);
      
      cout<<"AI::run() debug -2, current valid_moves size is "<<valid_moves.size()<<endl;
      
      //go through all the moves, try making them (as nodes in a tree structure)
      for(vector<_Move*>::iterator i=valid_moves.begin(); i!=valid_moves.end(); i++)
      {
        //copy the current board
        //NOTE: this copy does NOT need to be free'd; it will be free'd as a result of the destructor call to board
        Board *post_move=new Board(board);
        
        //apply the given move
        post_move->apply_move(*i);
        
        //if the result of this move is our owner being in check
        if(post_move->in_check(piece->owner))
        {
          //then it's not really a valid move
          cout<<"Warn: We'll be in check after a move to ("<<(*i)->toFile<<","<<(*i)->toRank<<")"<<endl;
          
          //free the associated memory
          free(*i);
          
          //remove this move from the vector to return
          i=valid_moves.erase(i);
          //the -- is so the ++ in the for loop kicks us back to the proper next element
          i--;
        }
      }
      cout<<"AI::run() debug -1, current valid_moves size is "<<valid_moves.size()<<endl;
      
      //remember we've already checked this
      checked_pieces++;
      board->set_checked(piece->file, piece->rank);
      cout<<"AI::run() debug 0, checked_pieces="<<checked_pieces<<endl;
    }
  }
  
  //if we checked all pieces and none had valid moves
  if(valid_moves.empty())
  {
    cout<<"AI::run() debug 1, checkmate!?"<<endl;
  }
  else
  {
    //make a random move of the legal moves generated earlier
    int rand_move_index=rand()%(valid_moves.size());
    _Move* move=valid_moves[rand_move_index];
    cout<<"AI::run() debug 2, ACTUALLY MOVING FROM ("<<move->fromFile<<","<<move->fromRank<<") to ("<<move->toFile<<","<<move->toRank<<")"<<endl;
    owned_pieces[rand_index].move(move->toFile, move->toRank, move->promoteType);
  }
  
  //handle memory properly
  for(size_t i=0; i<valid_moves.size(); i++)
  {
    free(valid_moves[i]);
  }
  delete board;
  
  return true;
}

//This function is run once, after your last turn.
void AI::end(){}
