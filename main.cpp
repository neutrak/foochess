//foobar-chess, a chess program based around an AI I made for class

#include <ctype.h>
#include "Board.h"
#include "AI.h"

#define BUFFER_SIZE 1024


//TODO: GENERALIZE THIS, MAKE IT LIKE, USABLE!!!!!!
//TODO: make this able to play a very configured AI against another very configured AI, either black or white for both user and AI, make user represented as an AI object
int main(int argc, char *argv[])
{
  //TODO: display options, allow user to set them
  
  //make a board to play on
  Board *board=new Board();
  int width=8;
  int height=8;
  
  //a buffer for the user to input something
  char input_buffer[BUFFER_SIZE];
  bzero(input_buffer,BUFFER_SIZE*sizeof(char));
  
  //create some AI player objects
//  AI *white_player=new AI();
//  white_player->init();
  
  AI *black_player=new AI();
  black_player->init();
  
  bool checkmate=false;
  //TODO: actually detect checkmates, and do it in a generalized manner
  while(!checkmate)
  {
    board->output_board();
    
    _Move *player_move=NULL;
    while(player_move==NULL)
    {
      printf("Move (expected format <from file><from rank><to file><to rank>; for example a2a3 would move from location a,2 to location a,3) (quit to quit): ");
      scanf("%s",input_buffer);
      
      printf("main debug -1, input_buffer=\"%s\"\n",input_buffer);
      if(!strncmp(input_buffer,"quit",BUFFER_SIZE))
      {
        checkmate=true;
        break;
      }
      
      //convert locations from characters to internal coordinate representation
      int from_file=tolower(input_buffer[0])-'a'+1;
      int from_rank=input_buffer[1]-'0';
      int to_file=tolower(input_buffer[2])-'a'+1;
      int to_rank=input_buffer[3]-'0';
      
      printf("main debug 0, got a move from %i,%i to %i,%i\n",from_file,from_rank,to_file,to_rank);
      if(from_file>=1 && from_file<=width && from_rank>=1 && from_rank<=height && to_file>=1 && to_file<=width && to_rank>=1 && to_rank<=height)
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
          board->apply_move(player_move,true);
          black_player->remember_move(board->copy_move(player_move));
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
    
    if(!checkmate)
    {
      printf("AI is thinking...\n");
      //now let the AI make a move
      black_player->run(board,BLACK);
    }
  }
  //TODO: detect winner
  
  delete board;
  
//  white_player->end();
//  delete white_player;
  
  black_player->end();
  delete black_player;
  
  return 0;
}

