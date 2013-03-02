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
  Board *b=new Board(pieces,NULL);
  
  // Print out the current board state
  b->output_board();
  
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
  //while there are no legal moves for the selected piece
  while(valid_moves.empty())
  {
    //of the owned pieces, select one at random
    rand_index=rand()%(owned_pieces.size());
    valid_moves=b->legal_moves(owned_pieces[rand_index]);
  }
  
  //make a random move of the legal moves generated earlier
  int rand_move_index=rand()%(valid_moves.size());
  owned_pieces[rand_index].move(valid_moves[rand_move_index]->toFile, valid_moves[rand_move_index]->toRank, valid_moves[rand_move_index]->promoteType);
  
  //handle memory properly
  for(size_t i=0; i<valid_moves.size(); i++)
  {
    free(valid_moves[i]);
  }
  delete b;
  
  return true;
}

//This function is run once, after your last turn.
void AI::end(){}
