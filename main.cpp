//foobar-chess, a chess program based around an AI I made for class

#include <ctype.h>
#include "Board.h"
#include "AI.h"
#include "TreeSearch.h"
#define VERSION "1.0"

//this is able to play a very configured AI against another very configured AI, either black or white for both user and AI
int main(int argc, char *argv[])
{
  char save_file[BUFFER_SIZE];
  strncpy(save_file,"",BUFFER_SIZE);
  
  //command line arguments, only for help text and loading save files
  int arg_idx;
  for(arg_idx=1;arg_idx<argc;arg_idx++)
  {
    if(!strcmp(argv[arg_idx],"--help"))
    {
      printf("For help see the manual page\n");
      exit(0);
    }
    else if(!strcmp(argv[arg_idx],"--version"))
    {
      printf("foochess version %s\n",VERSION);
      exit(0);
    }
    
    //TODO: add an argument to allow a user to import a board state into the starting point of a game
    //via a board state text file (.bst)
    //thus providing a sort of crude save/load functionality
    
    if(!strcmp(argv[arg_idx],"--load"))
    {
      arg_idx++;
      if(argc<=arg_idx)
      {
        fprintf(stderr,"Err: Missing .bst save file argument for --load\n");
        exit(1);
      }
      strncpy(save_file,argv[arg_idx],BUFFER_SIZE);
    }
  }
  
  //make a board to play on
  Board *board=new Board();
  int start_player_id=WHITE;
  
  //if a save file is given then load the board state from that
  //using Board::load_from_file(char *fname)
  if(strlen(save_file)>0)
  {
    board->load_from_file(save_file,&start_player_id);
  }
  
  //create some AI player objects
  AI *white_player=new AI();
  white_player->init();
  
  AI *black_player=new AI();
  black_player->init();
  
  //by default, white player is a human
  white_player->set_algo(USER);
  
  //display options, allow user to set them
  white_player->configure(WHITE);
  black_player->configure(BLACK);
  
  bool game_over=false;
  int winner=WHITE;
  int at_play_player=start_player_id;
  while(!game_over)
  {
    if(at_play_player==WHITE)
    {
      board->output_board();
      
      if(board->get_last_move_made()!=NULL)
      {
        white_player->remember_move(board->copy_move(board->get_last_move_made()));
      }
      
      printf("White player's turn...\n");
      white_player->run(board,WHITE);
      
      TreeSearch::generate_moves(board,BLACK);
      int black_legal_moves=board->get_children().size();
      board->clear_children();
      
      //checkmate is when someone is in check and has no legal moves
      if((black_legal_moves==0) && (board->get_check(BLACK)))
      {
        printf("CHECKMATE\n");
        game_over=true;
        winner=WHITE;
      }
      //stalemate is when there are no legal moves or there has been repetition, insufficient material, etc.
      else if(black_legal_moves==0 || TreeSearch::stalemate(board,white_player->get_moves()))
      {
        printf("STALEMATE\n");
        game_over=true;
        winner=-1;
      }
      
      if(!game_over)
      {
        if(board->get_check(BLACK))
        {
          printf("CHECK\n");
        }
      }
    }
    else
    {
      
      //output in reverse for the black player, since that's how they will see the board
//      board->output_board();
      board->output_reverse_board();
      
      if(board->get_last_move_made()!=NULL)
      {
        black_player->remember_move(board->copy_move(board->get_last_move_made()));
      }
      
      printf("Black player's turn...\n");
      black_player->run(board,BLACK);
      
      TreeSearch::generate_moves(board,WHITE);
      int white_legal_moves=board->get_children().size();
      board->clear_children();
      
      //checkmate is when someone is in check and has no legal moves
      if((white_legal_moves==0) && (board->get_check(WHITE)))
      {
        printf("CHECKMATE\n");
        game_over=true;
        winner=BLACK;
      }
      //stalemate is when there are no legal moves or there has been repetition, insufficient material, etc.
      else if(white_legal_moves==0 || TreeSearch::stalemate(board,black_player->get_moves()))
      {
        printf("STALEMATE\n");
        game_over=true;
        winner=-1;
      }
      
      if((!game_over) && (board->get_check(WHITE)))
      {
        printf("CHECK\n");
      }
    }
    
    //switch which player is at play
    at_play_player=(at_play_player==BLACK?WHITE:BLACK);
  }
  board->output_board();
  printf("Game over, winner was %s\n",(winner==WHITE)? "White" : (winner==BLACK)? "Black" : "<stalemate>");
  
  delete board;
  
  white_player->end();
  delete white_player;
  
  black_player->end();
  delete black_player;
  
  return 0;
}

