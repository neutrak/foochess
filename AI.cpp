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
  Board *b=new Board(pieces);
  
  // Print out the current board state
  b->output_board();

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
  
  //handle memory properly
  delete b;
  
  // select a random piece and move it to a random position on the board.  Attempts to promote to queen if a promotion happens
  pieces[rand()%pieces.size()].move(rand()%8+1, rand()%8+1, int('Q'));
  return true;
}

//This function is run once, after your last turn.
void AI::end(){}
