#include <stdio.h>
#include "AI.h"
#include "util.h"
#include "Board.h"
#include "SuperPiece.h"

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
  
  //initially the master copy of the board isn't made
  master=NULL;
}

//NOTE: the value pointed to by piece_index gets changed in this function
vector <_Move*> AI::generate_moves(Board *board, vector<Piece> owned_pieces, int *piece_index)
{
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
    if(!(board->get_element(owned_pieces[rand_index].file(), owned_pieces[rand_index].rank())->haveChecked))
    {
      //first, map onto the internal data structure
      _SuperPiece *piece=board->get_element(owned_pieces[rand_index].file(),owned_pieces[rand_index].rank());
      
      //then generate some moves
      valid_moves=board->legal_moves(piece);
      
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
          printf("AI::generate_moves() Warn: We'll be in check after a move from (%i,%i) to (%i,%i)\n",(*i)->fromFile, (*i)->fromRank, (*i)->toFile, (*i)->toRank);
          
          //free the associated memory
          free(*i);
          
          //remove this move from the vector to return
          i=valid_moves.erase(i);
          //the -- is so the ++ in the for loop kicks us back to the proper next element
          i--;
        }
        else
        {
          printf("AI::generate_moves(); random search generated a move from File, Rank (%i,%i) to (%i,%i)\n",(*i)->fromFile, (*i)->fromRank, (*i)->toFile, (*i)->toRank);
        }
        
        delete post_move;
      }
      
      //remember we've already checked this
      checked_pieces++;
      board->get_element(piece->file, piece->rank)->haveChecked=true;
    }
  }
  
  //let the calling code know what piece we ended up using
  *piece_index=rand_index;
  
  return valid_moves;
}

//creates a board based on the master board
//(creates the master board itself if there is none)
Board *AI::board_from_master()
{
  Board *board=NULL;
  
  //if this is the first move make the persistent data structure
  if(master==NULL)
  {
    //put the relevant data in a slightly more sane structure
    //to get constant time lookups for some things that may otherwise be O(n^2)
    master=new Board(pieces,NULL);
    
    //initialize all movements to 0, etc.
    for(size_t file=1; file<=8; file++)
    {
      for(size_t rank=1; rank<=8; rank++)
      {
        if(master->get_element(file,rank)!=NULL)
        {
          master->get_element(file,rank)->movements=0;
        }
      }
    }
    
    //(note here that there may be no previous move to apply)
    
    //if a move already happened (we're black) update the relevant movements variable
    if(moves.size()>0)
    {
      master->get_element(moves[0].toFile(), moves[0].toRank())->movements++;
      master->set_last_moved(moves[0].toFile(), moves[0].toRank());
    }
    board=master;
  }
  //make a turn-specific data structure
  else
  {
    //apply this move to the master board
    _Move *current_move=(_Move*)(malloc(sizeof(_Move)));
    current_move->_c=NULL;
    current_move->id=moves[0].id();
    current_move->fromFile=moves[0].fromFile();
    current_move->fromRank=moves[0].fromRank();
    current_move->toFile=moves[0].toFile();
    current_move->toRank=moves[0].toRank();
    current_move->promoteType=moves[0].promoteType();
    
    master->apply_move(current_move);
    //make this board a new copy of that master board
    board=new Board(master);
    
    //free the move
    free(current_move);
  }
  
  return board;
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
  Board *board=board_from_master();
  
  //this is just an integrity check to make sure data didn't get messed up at some point
  Board *tmp_board=new Board(pieces, NULL);
  if(!board->equals(tmp_board))
  {
    printf("AI::run() ERROR! we've lost the board state somewhere!\n");
    printf("\nOur board (internal) looks like this: \n");
    board->output_board();
    printf("\nThe real board (from their positions) looks like this: \n");
    tmp_board->output_board();
    exit(1);
  }
  delete tmp_board;
  
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
  
  //pick a piece and a valid move set
  int rand_index;
  vector<_Move*> valid_moves=generate_moves(board, owned_pieces, &rand_index);
  
  //if we checked all pieces and none had valid moves
  if(valid_moves.empty())
  {
    cout<<"AI::run() debug 0, checkmate!?"<<endl;
  }
  else
  {
    //make a random move of the legal moves generated earlier
    int rand_move_index=rand()%(valid_moves.size());
    _Move* move=valid_moves[rand_move_index];
    cout<<"AI::run() debug 1, ACTUALLY MOVING FROM ("<<move->fromFile<<","<<move->fromRank<<") to ("<<move->toFile<<","<<move->toRank<<")"<<endl;
    owned_pieces[rand_index].move(move->toFile, move->toRank, move->promoteType);
    
    //if we're moving a pawn diagonally to a place with no enemy
    if(owned_pieces[rand_index].type()=='P' && board->get_element(move->toFile, move->toRank)==NULL && (move->toFile!=move->fromFile))
    {
      //it must be an en passant
      printf("AI::run(), ACTUALLY TAKING EN PASSANT\n");
    }
    //if we're moving a king more 2 spaces
    else if(owned_pieces[rand_index].type()=='K' && abs((move->toFile)-(move->fromFile)==2))
    {
      //it's a castle
      printf("AI::run(), ACTUALLY TAKING CASTLE\n");
    }
    
    //apply the move we just made to the master board copy also
    master->apply_move(move);
  }
  
  //handle memory properly
  for(size_t i=0; i<valid_moves.size(); i++)
  {
    free(valid_moves[i]);
  }
  if(board!=master)
  {
    delete board;
  }
  
  return true;
}

//This function is run once, after your last turn.
void AI::end()
{
  if(master!=NULL)
  {
    delete master;
  }
}

