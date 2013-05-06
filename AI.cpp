#include <stdio.h>
#include "AI.h"
#include "Board.h"
#include "SuperPiece.h"
#include "TreeSearch.h"

AI::AI()
{
}

AI::~AI()
{
}

//time-limited input (timeout is in seconds)
//returns true when input is recieved, false otherwise
bool AI::tl_input(char *buffer, int buffer_size, int timeout)
{
  //initially the input buffer should be empty (just in case it isn't)
  bzero(buffer,buffer_size);
  buffer[0]='\0';
  
/*
  struct pollfd fd={STDIN_FILENO, POLLRDNORM, 0};
  
  //poll takes timeout in milliseconds, so we multiply by 1000 here
  poll(&fd, 1, timeout*(1000));
  if(fd.revents & POLLRDNORM)
  {
    bool success=(read(STDIN_FILENO, buffer, buffer_size) > 0);
    
    if(success)
    {
      //consider any newlines as termination
      for(unsigned int i=0; i<strlen(buffer); i++)
      {
        if(buffer[i]=='\r' || buffer[i]=='\n')
        {
          buffer[i]='\0';
        }
      }
    }
    
    return success;
  }
*/
  
  return false;
}

//This function is run once, before your first turn.
void AI::init()
{
  //seed randomness
  srand(time(NULL));
  
  //clear out moves vector
  moves.clear();
  
  //no history table until otherwise specified
  hist=NULL;
  
  //TODO: generalize, remove this code
  //set algorithm and heuristic (temporary code)
  algo=BEAM_HT_QS_TL_AB_ID_DLMM;
  heur=INFORMED_DANGER;
  
//  algo=RANDOM;
//  heur=INFORMED_DANGER;
}

_Move *AI::user_move(Board *board, int player_id)
{
  //a buffer for the user to input something
  char input_buffer[BUFFER_SIZE];
  bzero(input_buffer,BUFFER_SIZE*sizeof(char));
  
  _Move *player_move=NULL;
  while(player_move==NULL)
  {
    printf("Move (expected format <from file><from rank><to file><to rank>; for example a2a3 would move from location a,2 to location a,3) (quit to quit): ");
    scanf("%s",input_buffer);
    
    printf("AI::user_move debug 0, input_buffer=\"%s\"\n",input_buffer);
    if(!strncmp(input_buffer,"quit",BUFFER_SIZE))
    {
      //NOTE: any memory we are using should be cleaned up by the kernel here...
      exit(0);
      break;
    }
    
    //convert locations from characters to internal coordinate representation
    int from_file=tolower(input_buffer[0])-'a'+1;
    int from_rank=input_buffer[1]-'0';
    int to_file=tolower(input_buffer[2])-'a'+1;
    int to_rank=input_buffer[3]-'0';
    
    printf("main debug 0, got a move from %i,%i to %i,%i\n",from_file,from_rank,to_file,to_rank);
    //NOTE: width and height are always 8, so this range is defined with constants
    if(from_file>=1 && from_file<=8 && from_rank>=1 && from_rank<=8 && to_file>=1 && to_file<=8 && to_rank>=1 && to_rank<=8)
    {
      if(board->get_element(from_file,from_rank)!=NULL && board->get_element(from_file,from_rank)->owner==WHITE)
      {
        //TODO: verify this move is one of the ones that can be generated for that board for the player
        char promote_type='Q';
        if(board->get_element(from_file,from_rank)->type=='P' && to_rank==8)
        {
          bool valid_promotion=false;
          while(!valid_promotion)
          {
            printf("Pawn detected, what would you like to promote to? (Queen, Rook, kNight, Bishop): ");
            scanf("%s",input_buffer);
            if(input_buffer[0]=='Q' || input_buffer[0]=='R' || input_buffer[0]=='N' || input_buffer[0]=='B')
            {
              valid_promotion=true;
              promote_type=input_buffer[0];
            }
            else
            {
              printf("Err: Invalid promotion type, try again\n");
            }
          }
        }
        
        player_move=board->make_move(board->get_element(from_file,from_rank),to_file,to_rank,promote_type);
      }
      else
      {
        printf("Err: Start location doesn't have a piece you own, try again\n");
      }
    }
    else
    {
      printf("Err: Move was off of the board, try again\n");
    }
  }
  
  return player_move;
}

void AI::remember_move(_Move *m)
{
  moves.push_back(m);
}

//make a move depending on the algorithm (within the AI class) in use and the time left
_Move *AI::ai_move(Board *board, int player_id, double time_remaining, double enemy_time_remaining)
{
  _Move *move=NULL;
  
  //if the user is playing, get the start and end positions from stdin
  if(algo==USER)
  {
    printf("AI::ai_move() debug 0.5, making user-specified move\n");
    move=user_move(board,player_id);
  }
  else if(algo==RANDOM)
  {
    printf("AI::ai_move() debug 0.5, making random move\n");
    move=TreeSearch::random_move(board,player_id);
  }
  else if(algo==ID_DLMM || algo==TL_AB_ID_DLMM || algo==QS_TL_AB_ID_DLMM || algo==HT_QS_TL_AB_ID_DLMM || algo==BEAM_QS_TL_AB_ID_DLMM || algo==BEAM_HT_QS_TL_AB_ID_DLMM)
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
      
      new_move->id=moves[i]->id;
      new_move->fromFile=moves[i]->fromFile;
      new_move->fromRank=moves[i]->fromRank;
      new_move->toFile=moves[i]->toFile;
      new_move->toRank=moves[i]->toRank;
      new_move->promoteType=moves[i]->promoteType;
      
      move_accumulator.push_back(new_move);
    }
    //NOTE: the move_accumulator entries are free'd during recursive calls, and so don't need to be here
    
    TreeSearch ts;
    
    //NOTE: the way a non-quiescent search is done is to set the quiescent depth limit as 0
    if(algo==ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making id_minimax move\n");
      move=ts.id_minimax(board,3,0,player_id,move_accumulator,heur,false,false,NULL,0,time_remaining,enemy_time_remaining);
    }
    else if(algo==TL_AB_ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making time-limited alpha-beta pruned id minimax move\n");
      move=ts.id_minimax(board,1,0,player_id,move_accumulator,heur,true,true,NULL,0,time_remaining,enemy_time_remaining);
    }
    else if(algo==QS_TL_AB_ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making quiescent-search time-limited alpha-beta pruned id minimax move\n");
      move=ts.id_minimax(board,1,3,player_id,move_accumulator,heur,true,true,NULL,0,time_remaining,enemy_time_remaining);
    }
    else if(algo==HT_QS_TL_AB_ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making history-table quiescent-search time-limited alpha-beta pruned id minimax move\n");
      move=ts.id_minimax(board,1,3,player_id,move_accumulator,heur,true,true,hist,0,time_remaining,enemy_time_remaining);
    }
    else if(algo==BEAM_QS_TL_AB_ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making beam-search quiescent-search time-limited alpha-beta pruned id minimax move\n");
      move=ts.id_minimax(board,1,3,player_id,move_accumulator,heur,true,true,NULL,12,time_remaining,enemy_time_remaining);
    }
    else if(algo==BEAM_HT_QS_TL_AB_ID_DLMM)
    {
      printf("AI::ai_move() debug 0.5, making beam-search history-table quiescent-search time-limited alpha-beta pruned id minimax move\n");
      move=ts.id_minimax(board,1,3,player_id,move_accumulator,heur,true,true,hist,12,time_remaining,enemy_time_remaining);
    }
  }
  return move;
}

//This function is called each time it is your turn.
//Return true to end your turn, return false to ask the server for updated information.
bool AI::run(Board *board, int player_id)
{
  // Print out the current board state
//  board->output_board();
  
  //this is a "sliding window" for history table algorithms
  //it doesn't really slide so much as step, but it's still better behavior than never adjusting for early to late game
  if((moves.size()%15==0) && hist!=NULL)
  {
    delete hist;
    hist=new HistTable();
  }
  
  double time_remaining=900;
  double enemy_time_remaining=900;
  
  //make a move depending on the algorithm in use and the time left
  _Move* move=ai_move(board, player_id, time_remaining, enemy_time_remaining);
  
  if(move!=NULL)
  {
    //apply the move we just made to the master board copy also; but use different memory for management ease
    board->apply_move(move, true);
    moves.push_back(board->copy_move(move));
  }
  else
  {
    fprintf(stderr, "Err: Could not make a move; no legal moves!?\n");
  }
  
  return true;
}

//This function is run once, after your last turn.
void AI::end()
{
  if(hist!=NULL)
  {
    delete hist;
  }
  
  //clear out the moves vector
  for(size_t i=0; i<moves.size(); i++)
  {
    free(moves[i]);
  }
  moves.resize(0);
}

