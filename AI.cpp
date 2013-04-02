#include <stdio.h>
#include "AI.h"
#include "util.h"
#include "Board.h"
#include "SuperPiece.h"
#include "TreeSearch.h"

AI::AI(Connection* conn) : BaseAI(conn) {}

const char* AI::username()
{
  return "foobar bot";
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
    
    master->apply_move(current_move, true);
    //make this board a new copy of that master board
    board=new Board(master);
  }
  
  return board;
}

_Move *AI::user_move(Board *board)
{
  _Move *move=(_Move*)(malloc(sizeof(_Move)));
  if(move==NULL)
  {
    fprintf(stderr,"Err: Out of RAM!? (malloc failed)");
    exit(1);
  }
  
  move->_c=NULL;
  move->id=0;
  
  do
  {
    printf("Waiting for a move from the user... (expected format fromFile,fromRank toFile,toRank\\n)\nmove: ");
    fscanf(stdin,"%i,%i %i,%i", &(move->fromFile), &(move->fromRank), &(move->toFile), &(move->toRank));
    
    if(board->get_element(move->fromFile,move->fromRank)!=NULL && board->get_element(move->fromFile,move->fromRank)->type=='P')
    {
      printf("pawn detected; promoteType: ");
//      fscanf(stdin,"%i", &(move->promoteType)); //TODO: figure out why this causes subsequent reads to be non-blocking
      move->promoteType='Q';
      printf("\n");
    }
    else
    {
      move->promoteType='Q';
    }
  }
  //repeat getting a move until all points are within the bounds of the board
  while(!(move->fromFile>=1 && move->fromFile<=8 && move->fromRank>=1 && move->fromRank<=8 && move->toFile>=1 && move->toFile<=8 && move->toRank>=1 && move->toRank<=8));
  
  printf("AI::user_move() debug 0, move is (%i,%i) to (%i,%i) with promotion type %c\n",move->fromFile,move->fromRank,move->toFile,move->toRank,move->promoteType);
  return move;
}

//make a move depending on the algorithm in use and the time left
_Move *AI::ai_move(Board *board, algorithm algo, double time_remaining)
{
  _Move *move=NULL;
  
  //if the user is playing, get the start and end positions from stdin
  if(algo==USER)
  {
    printf("AI::run() debug 0.5, making user-specified move\n");
    move=user_move(board);
  }
  else if(algo==RANDOM)
  {
    printf("AI::run() debug 0.5, making random move\n");
    move=TreeSearch::random_move(board,playerID());
  }
  else if(algo==ID_DLMM || algo==TL_AB_ID_DLMM)
  {
    //make a move accumulator to start it out based on the moves their API gives us
    //note this builds the array in reverse order to what's given
    //because I need to push_back as I go in tree generation
    vector<_Move*> move_accumulator;
    for(int i=moves.size()-1; i>=0; i--)
    {
      _Move *new_move=(_Move*)(malloc(sizeof(_Move)));
      if(new_move==NULL)
      {
        fprintf(stderr,"Err: Out of RAM!? (malloc failed)");
        exit(1);
      }
      
      //connection is not something we're dealing with here
      new_move->_c=NULL;
      new_move->id=moves[i].id();
      new_move->fromFile=moves[i].fromFile();
      new_move->fromRank=moves[i].fromRank();
      new_move->toFile=moves[i].toFile();
      new_move->toRank=moves[i].toRank();
      new_move->promoteType=moves[i].promoteType();
      
      move_accumulator.push_back(new_move);
    }
    //NOTE: the move_accumulator entries are free'd during recursive calls, and so don't need to be here
    
    if(algo==ID_DLMM)
    {
      printf("AI::run() debug 0.5, making id_minimax move\n");
      move=TreeSearch::id_minimax(board,3,playerID(),move_accumulator,false,false,time_remaining);
    }
    else if(algo==TL_AB_ID_DLMM)
    {
      printf("AI::run() debug 0.5, making time-limited alpha-beta pruned id minimax move\n");
      move=TreeSearch::id_minimax(board,4,playerID(),move_accumulator,true,true,time_remaining);
    }
  }
  return move;
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run()
{
  //an algorithm variable so we can re-use generalized code instead of losing old functionality
//  algorithm algo=USER;
//  algorithm algo=RANDOM;
//  algorithm algo=ID_DLMM;
  algorithm algo=TL_AB_ID_DLMM;
  
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
  double time_remaining=0;
  
  // Looks through information about the players
  for(size_t p=0; p<players.size(); p++)
  {
    cout<<players[p].playerName();
    // if playerID is 0, you're white, if its 1, you're black
    if(players[p].id() == playerID())
    {
      cout<<" (ME)";
      //store the time left so that we can manage it well
      time_remaining=players[p].time();
      
      //if we're running out of time use a random move rather than timing out
      if(players[p].time() <= RANDOM_FALLBACK_TIME)
      {
        algo=RANDOM;
      }
    }
    cout<<" time remaining: "<<players[p].time()<<endl;
  }

  // if there has been a move, print the most recent move
  if(moves.size() > 0)
  {
    cout<<"Last Move Was: "<<endl<<moves[0]<<endl;
  }
  
  //make a move depending on the algorithm in use and the time left
  _Move* move=ai_move(board, algo, time_remaining);
  
  if(move!=NULL)
  {
    //figure out what piece that movement entails
    int piece_index=0;
    _SuperPiece *moving_piece=board->get_element(move->fromFile, move->fromRank);
    for(int i=0; i<owned_pieces.size(); i++)
    {
      if(owned_pieces[i].file()==moving_piece->file && owned_pieces[i].rank()==moving_piece->rank)
      {
        piece_index=i;
        i=owned_pieces.size();
      }
    }
    
    cout<<"AI::run() debug 1, ACTUALLY MOVING FROM ("<<move->fromFile<<","<<move->fromRank<<") to ("<<move->toFile<<","<<move->toRank<<")"<<endl;
    owned_pieces[piece_index].move(move->toFile, move->toRank, move->promoteType);
    
    //if we're moving a pawn diagonally to a place with no enemy
    if(owned_pieces[piece_index].type()=='P' && board->get_element(move->toFile, move->toRank)==NULL && (move->toFile!=move->fromFile))
    {
      //it must be an en passant
      printf("AI::run(), ACTUALLY TAKING EN PASSANT\n");
    }
    //if we're moving a king more 2 spaces
    else if(owned_pieces[piece_index].type()=='K' && abs((move->toFile)-(move->fromFile)==2))
    {
      //it's a castle
      printf("AI::run(), ACTUALLY TAKING CASTLE\n");
    }
    
    //apply the move we just made to the master board copy also; but use different memory for management ease
    master->apply_move(move, true);
  }
  else
  {
    fprintf(stderr, "Err: Could not make a move; no legal moves!?\n");
  }
  
  //handle memory properly
  //(the NULL check here is defensive)
  if(board!=NULL && board!=master)
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

