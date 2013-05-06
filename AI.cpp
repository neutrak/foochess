#include <stdio.h>
#include "AI.h"
#include "Board.h"
#include "SuperPiece.h"
#include "TreeSearch.h"

AI::AI()
{
  //make sane setting defaults
  
  //the history table this AI is using (NULL for none)
  //(history table is NULL for no history table, so doesn't need to be a seperate setting)
//  hist=new HistTable();
  hist=NULL;
  
  //treesearch settings
  max_depth=1;
  qs_depth=3;
  ab_prune=true;
  
  //heurstic stuff
  heur_pawn_additions=true;
  heur_position_additions=true;
  
  //weight for opponent pieces and our pieces, respecitvely
  enemy_weight=1;
  owned_weight=0.75;
  
  //whether or not to time-limit
  time_limit=true;
  //how long the time should be if it is limited
  timeout=30.0;
  
  //how many nodes to keep after forward-pruning (0 for no forward pruning)
  beam_width=12;
}

AI::~AI()
{
}

//allow the user to change all the settings of this AI object
void AI::configure(int player_id)
{
  char input_buffer[BUFFER_SIZE];
  
  printf("Configuring for %s player...\n",(player_id==WHITE)? "White" : "Black");
  
  bool selection_done=false;
  while(!selection_done)
  {
    printf("Current algorithm is %s; change (Yes, No)? ",(algo==USER)? "USER" : (algo==RANDOM)? "RANDOM" : "TREE_SEARCH");
    scanf("%s",input_buffer);
    //always null-terminate
    input_buffer[BUFFER_SIZE-1]='\0';
    
    if(tolower(input_buffer[0])=='y')
    {
      printf("New selection (USER, RANDOM, TREE_SEARCH): ");
      scanf("%s",input_buffer);
      input_buffer[BUFFER_SIZE-1]='\0';
      
      //make this input case-insensitive
      for(unsigned int n=0; n<strlen(input_buffer); n++)
      {
        input_buffer[n]=tolower(input_buffer[n]);
      }
      
      if(!strncmp(input_buffer,"user",BUFFER_SIZE))
      {
        algo=USER;
      }
      else if(!strncmp(input_buffer,"random",BUFFER_SIZE))
      {
        algo=RANDOM;
      }
      else if(!strncmp(input_buffer,"tree_search",BUFFER_SIZE))
      {
        algo=TREE_SEARCH;
      }
    }
    else
    {
      selection_done=true;
    }
  }
  
  if(algo==TREE_SEARCH)
  {
    bool options_done=false;
    while(!options_done)
    {
      //TODO: let the user set all tree search options here
      options_done=true;
    }
  }
  
  //pretty formatting
  printf("\n\n");
}

//time-limited input (timeout is in seconds)
//returns true when input is recieved, false otherwise
bool AI::tl_input(char *buffer, int buffer_size, int timeout)
{
  //initially the input buffer should be empty (just in case it isn't)
  bzero(buffer,buffer_size);
  buffer[0]='\0';
  
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
  algo=TREE_SEARCH;
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
    printf("Move Selection (expected format <from file><from rank><to file><to rank>; for example a2a3 would move from location a,2 to location a,3) (quit to quit)\n");
    printf("Enter move: ");
    scanf("%s",input_buffer);
    //always null-terminate
    input_buffer[BUFFER_SIZE-1]='\0';
    
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
        //verify this move is one of the ones that can be generated for that board for the player, if not make them try again
        bool found_move=false;
        vector<_Move*> possible_moves=TreeSearch::generate_moves(board,player_id);
        for(size_t n=0; n<possible_moves.size(); n++)
        {
          //if the moves started in the same spot and ended in the same spot
          if((from_file == possible_moves[n]->fromFile) && (from_rank == possible_moves[n]->fromRank) && (to_file == possible_moves[n]->toFile) && (to_rank == possible_moves[n]->toRank))
          {
            found_move=true;
          }
        }
        board->clear_children();
        
        if(found_move)
        {
          char promote_type='Q';
          if(board->get_element(from_file,from_rank)->type=='P' && to_rank==8)
          {
            bool valid_promotion=false;
            while(!valid_promotion)
            {
              printf("Pawn detected, what would you like to promote to? (Queen, Rook, kNight, Bishop): ");
              scanf("%s",input_buffer);
              input_buffer[BUFFER_SIZE-1]='\0';
              //case-insensitive
              input_buffer[0]=tolower(input_buffer[0]);
              
              if(input_buffer[0]=='q' || input_buffer[0]=='q' || input_buffer[0]=='n' || input_buffer[0]=='b')
              {
                valid_promotion=true;
                promote_type=toupper(input_buffer[0]);
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
          printf("Err: Suggested move was illegal for your current state, try again\n");
        }
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
  else if(algo==TREE_SEARCH)
  {
    //make a move accumulator to start it out based on the moves their API gives us
    //note this builds the array in reverse order to what's given
    //because I need to push_back as I go in tree generation
    vector<_Move*> move_accumulator;
//    for(int i=moves.size()-1; i>=0; i--) //reverse-ordered moves
    for(size_t i=0; i<moves.size(); i++) //correctly ordered moves
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
    printf("AI::ai_move() debug 0.5, making tree search (id minimax) move\n");
    
    //declaration: static _Move *id_minimax(Board *root, int max_depth_limit, int qs_depth_limit, int player_id, vector<_Move*> move_accumulator, heuristic heur, bool prune, bool time_limit, HistTable *hist, unsigned int beam_width, double time_remaining, double enemy_time_remaining);
    //default AI player
//    move=ts.id_minimax(board,1,3,player_id,move_accumulator,heur,true,true,hist,12,time_remaining,enemy_time_remaining);
    
    //configured AI player
    //TODO: replace heur with weight settings and heuristic options
    //TODO: replace time_remaining with timeout values, ignored enemy_time_remaining
    move=ts.id_minimax(board,max_depth,qs_depth,player_id,move_accumulator,heur,ab_prune,time_limit,hist,beam_width,900,900);
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
    hist=NULL;
  }
  
  //clear out the moves vector
  for(size_t i=0; i<moves.size(); i++)
  {
    free(moves[i]);
  }
  moves.resize(0);
}

