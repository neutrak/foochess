#include <stdio.h>
#include "AI.h"
#include "Board.h"
#include "SuperPiece.h"
#include "TreeSearch.h"

AI::AI()
{
  //make sane setting defaults
  
  //treesearch settings
  max_depth=1;
  qs_depth=30;
  ab_prune=true;
  
  //the history table this AI is using (NULL for none)
  //(history table is NULL for no history table, so doesn't need to be a seperate setting)
  hist=new HistTable();
  history_reset=15;
  
  //whether or not to use the entropy (branching-factor) heuristic
  entropy_heuristic=false;
  //whether to use alternate entropy heuristic (requires entropy_heuristic to be TRUE)
  distance_sum=false;
  
  //heurstic stuff
  heur_pawn_additions=true;
  heur_position_additions=true;
  
  //weight for opponent pieces and our pieces, respecitvely
  enemy_weight=1;
  owned_weight=0.9;
  
  //whether or not to time-limit
  time_limit=true;
  //how long the time should be if it is limited
  timeout=30.0;
  
  //how many nodes to keep after forward-pruning (0 for no forward pruning)
  beam_width=0;
}

AI::~AI()
{
}

//output current tree search settings
void AI::output_ts_settings()
{
  printf("max_depth=%i                       (disregarded if time_limit is true)\n",max_depth);
  printf("qs_depth=%i\n",qs_depth);
  printf("ab_prune=%s\n",ab_prune? "true" : "false");
  printf("\n");
  printf("history=%s\n",(hist==NULL)? "false" : "true");
  printf("history_reset=%i                   (disregarded if history is false)\n",history_reset);
  printf("\n");
  printf("entropy_heuristic=%s\n",entropy_heuristic? "true" : "false");
  printf("distance_sum=%s                    (disregarded if entropy_heuristic is false)\n",distance_sum? "true" : "false");
  printf("\n");
  printf("heur_pawn_additions=%s             (disregarded if entropy_heuristic is true)\n",heur_pawn_additions? "true" : "false");
  printf("heur_position_additions=%s         (disregarded if entropy_heuristic is true)\n",heur_position_additions? "true" : "false");
  printf("\n");
  printf("enemy_weight=%lf                   (disregarded if entropy_heuristic is true)\n",enemy_weight);
  printf("owned_weight=%lf                   (disregarded if entropy_heuristic is true)\n",owned_weight);
  printf("\n");
  printf("time_limit=%s\n",time_limit? "true" : "false");
  printf("timeout=%lf                        (disregarded if time_limit is false; units of seconds)\n",timeout);
  printf("\n");
  printf("beam_width=%i                      (disregarded if entropy_heuristic is true; 0 for no forward pruning, else how many children should left after pruning)\n",beam_width);
  printf("\n\n");
}

void AI::set_ts_option(char *variable, char *value, int buffer_size)
{
  //first lower case both the variable and value
  for(int n=0; n<buffer_size && variable[n]!='\0'; n++)
  {
    variable[n]=tolower(variable[n]);
  }
  for(int n=0; n<buffer_size && value[n]!='\0'; n++)
  {
    value[n]=tolower(value[n]);
  }
  
  if(!strncmp(variable,"max_depth",buffer_size))
  {
    max_depth=atoi(value);
  }
  else if(!strncmp(variable,"qs_depth",buffer_size))
  {
    qs_depth=atoi(value);
  }
  else if(!strncmp(variable,"ab_prune",buffer_size))
  {
    ab_prune=string_to_bool(value,buffer_size);
  }
  else if(!strncmp(variable,"history",buffer_size))
  {
    //remove any existing history
    if(hist!=NULL)
    {
      delete hist;
    }
    
    //if we're being asked to make some history, do that
    if(string_to_bool(value,buffer_size))
    {
      hist=new HistTable();
    }
    //if not, set a placeholder so we know we're not using history
    else
    {
      hist=NULL;
    }
  }
  else if(!strncmp(variable,"history_reset",buffer_size))
  {
    history_reset=atoi(value);
  }
  else if(!strncmp(variable,"entropy_heuristic",buffer_size))
  {
    entropy_heuristic=string_to_bool(value,buffer_size);
  }
  //NOTE: THIS ONLY APPLIES WHEN ENTROPY_HEURISTIC IS TRUE
  else if(!strncmp(variable,"distance_sum",buffer_size))
  {
    distance_sum=string_to_bool(value,buffer_size);
  }
  else if(!strncmp(variable,"heur_pawn_additions",buffer_size))
  {
    heur_pawn_additions=string_to_bool(value,buffer_size);
  }
  else if(!strncmp(variable,"heur_position_additions",buffer_size))
  {
    heur_position_additions=string_to_bool(value,buffer_size);
  }
  else if(!strncmp(variable,"enemy_weight",buffer_size))
  {
    enemy_weight=atof(value);
  }
  else if(!strncmp(variable,"owned_weight",buffer_size))
  {
    owned_weight=atof(value);
  }
  else if(!strncmp(variable,"time_limit",buffer_size))
  {
    time_limit=string_to_bool(value,buffer_size);
  }
  else if(!strncmp(variable,"timeout",buffer_size))
  {
    timeout=atof(value);
  }
  else if(!strncmp(variable,"beam_width",buffer_size))
  {
    beam_width=atoi(value);
  }
}

bool AI::string_to_bool(char *string, int buffer_size)
{
  if(tolower(string[0])=='t')
  {
    return true;
  }
  
  //if we didn't get a true value above, assume the value to be false
  return false;
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
    user_input(input_buffer);
    
    if(tolower(input_buffer[0])=='y')
    {
      printf("New selection (USER, RANDOM, TREE_SEARCH): ");
      user_input(input_buffer);
      
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
      output_ts_settings();
      printf("To change a value, state variable=new_value\n");
      printf("for example timeout=40.0 to make the timeout 40 seconds\n");
      printf("Setting (done or d to finish with settings and start playing): ");
      
      //get some input!
      char input_buffer[BUFFER_SIZE];
      bzero(input_buffer,BUFFER_SIZE);
      user_input(input_buffer);
      
      if((!strncmp(input_buffer,"done",BUFFER_SIZE)) || (!strncmp(input_buffer,"d",BUFFER_SIZE)))
      {
        options_done=true;
      }
      else
      {
        //store what is before and after the equal sign, respectively
        char variable[BUFFER_SIZE];
        bzero(variable,BUFFER_SIZE);
        char value[BUFFER_SIZE];
        bzero(value,BUFFER_SIZE);
        
        //look for an equal sign in the user input
        //if none is found the index should remain at -1 as an error code
        int equal_index=-1;
        for(int n=0; n<BUFFER_SIZE; n++)
        {
          if(input_buffer[n]!='\0')
          {
            //if we still haven't found an equal sign add this char to variable
            if(equal_index<0)
            {
              variable[n]=input_buffer[n];
              if(input_buffer[n]=='=')
              {
                equal_index=n;
                variable[n]='\0';
              }
            }
            //now that we've found a variable, read into the value string
            else
            {
              value[n-equal_index-1]=input_buffer[n];
            }
          }
          else
          {
            n=BUFFER_SIZE;
          }
        }
        
        //if an equal sign was found somewhere in the string, scan it in!
        if(equal_index>=0)
        {
//          printf("AI::configure() debug 0, got variable \"%s\" value \"%s\"\n",variable,value);
          //set the internal variable!
          set_ts_option(variable,value,BUFFER_SIZE);
        }
      }
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
  
  //this is just initialization
  algo=TREE_SEARCH;
}

//add "load" and "save" commands to load from and save to a board state file
bool AI::handle_load_save(const char *input_buffer, Board *board)
{
  //if there is no space
  const char *space_content=strstr(input_buffer," ");
  if(space_content==NULL)
  {
    //then skip it; it's not our problem
    return false;
  }
  
  int space_idx=space_content-input_buffer;
  
  char cmd_buf[BUFFER_SIZE];
  strncpy(cmd_buf,input_buffer,space_idx);
  cmd_buf[space_idx]='\0';
  
  char arg_buf[BUFFER_SIZE];
  strncpy(arg_buf,"",BUFFER_SIZE);
  strncpy(arg_buf,input_buffer+space_idx+1,strlen(input_buffer)-space_idx-1);
  
  //load a save file
  if((!strcmp(cmd_buf,"load")) || (!strcmp(cmd_buf,"l")))
  {
    //NOTE: start_player_id is IGNORED because there is already an at-play player in this case
    int start_player_id=WHITE;
    
    board->load_from_file(arg_buf, &start_player_id);
    
    if(start_player_id==WHITE)
    {
      board->output_board();
    }
    else
    {
      board->output_reverse_board();
    }
    return true;
  }
  //save to a file
  else if((!strcmp(cmd_buf,"save")) || (!strcmp(cmd_buf,"s")))
  {
    //TODO: write save functionality using board save method (yet to be written)
    board->output_board(); //debug
    printf("\n\n");
    board->output_reverse_board(); //debug
    printf("\n\n");
    
    return true;
  }
  
  return false;
}

void AI::user_input(char *input_buffer)
{
  //NOTE: fgets is documented to always null-terminate
  fgets(input_buffer,BUFFER_SIZE,stdin);
  
  //trim trailing newline
  int last_ch_idx=strlen(input_buffer)-1;
  if(last_ch_idx>=0 && input_buffer[last_ch_idx]=='\n')
  {
    input_buffer[last_ch_idx]='\0';
  }
}

_Move *AI::user_move(Board *board, int player_id)
{
  //a buffer for the user to input something
  char input_buffer[BUFFER_SIZE];
  bzero(input_buffer,BUFFER_SIZE*sizeof(char));
  
  bool skip_prompt=false;
  _Move *player_move=NULL;
  while(player_move==NULL)
  {
    if(!skip_prompt)
    {
      printf("Move Selection (expected format <from file><from rank><to file><to rank>)\n");
      printf("for example a2a3 would move from location a,2 to location a,3\n");
      printf("quit or q to quit\n");
      printf("save or s to save; save <filename.bst>\n");
      printf("load or l to load; load <filename.bst>\n");
      printf("Enter move: ");
    }
    skip_prompt=false;
    
    user_input(input_buffer);
    
    //ignore blank lines
    if(strlen(input_buffer)==0){
      skip_prompt=true;
      continue;
    }
    
//    printf("AI::user_move debug 0, input_buffer=\"%s\"\n",input_buffer);
    if((!strncmp(input_buffer,"quit",BUFFER_SIZE)) || (!strncmp(input_buffer,"q",BUFFER_SIZE)))
    {
      //NOTE: any memory we are using should be cleaned up by the kernel here...
      exit(0);
      break;
    }
    
    //add "load" and "save" commands to load from and save to a board state file
    if(handle_load_save(input_buffer,board))
    {
      continue;
    }
    
    //convert locations from characters to internal coordinate representation
    int from_file=tolower(input_buffer[0])-'a'+1;
    int from_rank=input_buffer[1]-'0';
    int to_file=tolower(input_buffer[2])-'a'+1;
    int to_rank=input_buffer[3]-'0';
    
//    printf("main debug 0, got a move from %c,%i to %c,%i\n",(char)(from_file+'a'-1),from_rank,(char)(to_file+'a'-1),to_rank);
    //NOTE: width and height are always 8, so this range is defined with constants
    if(from_file>=1 && from_file<=8 && from_rank>=1 && from_rank<=8 && to_file>=1 && to_file<=8 && to_rank>=1 && to_rank<=8)
    {
      if(board->get_element(from_file,from_rank)!=NULL && board->get_element(from_file,from_rank)->owner==player_id)
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
              
              user_input(input_buffer);
              
              //case-insensitive
              input_buffer[0]=tolower(input_buffer[0]);
              
              if(input_buffer[0]=='q' || input_buffer[0]=='r' || input_buffer[0]=='n' || input_buffer[0]=='b')
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
//    printf("AI::ai_move() debug 0.5, making user-specified move\n");
    move=user_move(board,player_id);
  }
  else if(algo==RANDOM)
  {
//    printf("AI::ai_move() debug 0.5, making random move\n");
    printf("Random move...\n");
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
//    printf("AI::ai_move() debug 0.5, making tree search (id minimax) move\n");
    printf("AI is thinking (will stop when time or depth limit is reached)...\n");
    if(time_limit)
    {
      printf("Using a time limit of %lf seconds...\n",timeout);
    }
    else
    {
      printf("Using a depth limit of %i moves ahead...\n",max_depth);
    }
    
    //default AI player (what was entered in the AI tournament)
//    move=ts.id_minimax(board,1,3,player_id,move_accumulator,false,false,true,true,1,0.75,false,true,true,hist,12,time_remaining,enemy_time_remaining,false);
    
    //configured AI player
    //NOTE: weight settings and heuristic options are used in place of a heur from an enum
    //NOTE: when fixed_time (last boolean argument) is true, time_remaining is time allocated to this move; in this case time heuristic is not used
    move=ts.id_minimax(board,max_depth,qs_depth,player_id,move_accumulator,entropy_heuristic,distance_sum,heur_pawn_additions,heur_position_additions,enemy_weight,owned_weight,ab_prune,time_limit,hist,beam_width,timeout,900,true);
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
  //NOTE: if history_reset<=0, no reset ever occurs
  if((history_reset>0) && (moves.size()%history_reset==0) && hist!=NULL)
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
    
    if(algo!=USER)
    {
      printf("Move made was %c%i%c%i\n",(char)(move->fromFile+'a'-1),move->fromRank,(char)(move->toFile+'a'-1),move->toRank);
    }
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

